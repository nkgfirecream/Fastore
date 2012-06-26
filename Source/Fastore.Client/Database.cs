using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Threading;

namespace Alphora.Fastore.Client
{
	// TODO: concurrency
    public class Database : IDataAccess, IDisposable
    {
		public const int MaxSystemColumnID = 9999;

		private class PodMap
		{
			public PodMap()
			{
				Pods = new List<int>();
			}
			public List<int> Pods;
			public int Next;
		}

		private Object _mapLock = new Object();

		// Connected services by host ID
		private Dictionary<int, Service.Client> _services = new Dictionary<int, Service.Client>();

		// Connected workers by pod ID
		private Dictionary<int, Worker.Client> _workers = new Dictionary<int, Worker.Client>();

		// Worker states by pod ID
		private Dictionary<int, Tuple<ServiceState, WorkerState>> _workerStates = new Dictionary<int, Tuple<ServiceState, WorkerState>>();

		// Pod map (round robin pod IDs) per column
		private Dictionary<int, PodMap> _columnWorkers = new Dictionary<int, PodMap>();

		// The next worker to use to retrieve system columns from
		private int _nextSystemWorker = 0;

		// Latest known state of the hive
		private HiveState _hiveState;

		// Currently known schema
		private Schema _schema;

		/// <summary> The default timeout for write operations. </summary>
		public const int DefaultWriteTimeout = 1000;

		private int _writeTimeout = DefaultWriteTimeout;
		/// <summary> ApplyTimeout specifies the maximum time in milliseconds to wait for workers to respond to an apply request. </summary>
		/// <remarks> The default is 1000 (1 second). </remarks>
		public int WriteTimeout 
		{ 
			get { return _writeTimeout; } 
			set
			{
				if (value < -1)
					throw new ArgumentOutOfRangeException("value", "WriteTimeout must be -1 or greater.");
				_writeTimeout = value;
			}
		}

		public Database(ServiceAddress[] addresses)
        {
			// Convert from service addresses to network addresses
			var networkAddresses = (from a in addresses select new NetworkAddress { Name = a.Name, Port = a.Port }).ToArray();

			// Number of potential workers for each service (in case we nee to create the hive)
			var serviceWorkers = new int[networkAddresses.Length];

			for (int i = 0; i < networkAddresses.Length; i++)
			{
				var service = ConnectToService(networkAddresses[i]);
				try
				{
					HiveState hiveState;
					try
					{
						// Discover the state of the entire hive from the given service
						hiveState = service.getHiveState(false);

						// Ensure that all services are not joined (if the first one wasn't)
						if (i > 0)
							throw new Exception(String.Format("Service '{0}' is joined to topology {1}, while at least one other specified service is not part of any topology.", networkAddresses[i].Name, hiveState.TopologyID));
					}
					catch (NotJoined nj)
					{
						serviceWorkers[i] = nj.PotentialWorkers;
						ReleaseService(service);
						continue;
					}

					UpdateHiveState(hiveState);

					// Add the service to our collection to avoid re-connection if needed
					_services.Add(hiveState.ReportingHostID, service);

					BootStrapSchema();
				}
				catch
				{
					// If anything goes wrong, be sure to release the service client
					_services.Clear();
					ReleaseService(service);
					throw;
				}
			}

			var newTopology = CreateTopology(serviceWorkers);

			var addressesByHost = new Dictionary<int, NetworkAddress>();
			for (var hostID = 0; hostID < networkAddresses.Length; hostID++)
				addressesByHost.Add(hostID, networkAddresses[hostID]);

			var newHive = new HiveState { TopologyID = newTopology.TopologyID, Services = new Dictionary<int, ServiceState>() };
			for (var hostID = 0; hostID < networkAddresses.Length; hostID++)
			{
				var service = ConnectToService(networkAddresses[hostID]);
				try
				{
					var serviceState = service.init(newTopology, addressesByHost, hostID);
					newHive.Services.Add(hostID, serviceState);
				}
				finally
				{
					ReleaseService(service);
				}
			}

			UpdateHiveState(newHive);
			BootStrapSchema();
		}

		private static Topology CreateTopology(int[] serviceWorkers)
		{
			var newTopology = new Topology { TopologyID = Guid.NewGuid().GetHashCode() };
			newTopology.Hosts = new Dictionary<int, Dictionary<int, List<int>>>();
			var podID = 0;
			for (var hostID = 0; hostID < serviceWorkers.Length; hostID++)
			{
				var pods = new Dictionary<int, List<int>>();
				for (int i = 0; i < serviceWorkers[hostID]; i++)
				{
					pods.Add(podID, new List<int>());	// No no columns initially 
					podID++;
				}
				newTopology.Hosts.Add(hostID, pods);
			}
			return newTopology;
		}

		public void Dispose()
		{
			var errors = new List<Exception>();
			foreach (var service in _services.ToArray())
			{
				try
				{
					ReleaseService(service.Value);
				}
				catch (Exception e)
				{
					errors.Add(e);
				}
				finally
				{
					_services.Remove(service.Key);
				}
			}
			foreach (var worker in _workers.ToArray())
			{
				try
				{
					ReleaseWorker(worker.Value);
				}
				catch (Exception e)
				{
					errors.Add(e);
				}
				finally
				{
					_workers.Remove(worker.Key);
				}
			}
			UpdateHiveState(null);
			if (errors.Count > 0)
				throw new AggregateException(errors);
		}

		/// <summary> Ensures that there is a connection to the service for the given host ID and returns it. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private Service.Client EnsureService(int hostID)
		{
			Monitor.Enter(_mapLock);
			bool taken = true;
			try
			{
				Service.Client result;
				if (!_services.TryGetValue(hostID, out result))
				{
					ServiceState serviceState;
					if (!_hiveState.Services.TryGetValue(hostID, out serviceState))
						throw new Exception(String.Format("No service is currently associated with host ID ({0}).", hostID));

					// Release the lock during connection
					Monitor.Exit(_mapLock);
					taken = false;

					result = ConnectToService(serviceState.Address);

					// Take the lock again
					Monitor.Enter(_mapLock);
					taken = true;

					// Verify that the service hasn't been found in the mean-time
					if (_services.ContainsKey(hostID))
					{
						ReleaseService(result);
						return _services[hostID];
					}
					else
						_services.Add(hostID, result);
				}
				return result;
			}
			finally
			{
				if (taken)
					Monitor.Exit(_mapLock);
			}
		}

		private void ReleaseService(Service.Client client)
		{
			client.InputProtocol.Transport.Close();
		}

		/// <summary> Makes a connection to a service given connection information. </summary>
		/// <param name="hostID"> The host ID of the service if it is known; if not given the service is asked. </param>
		/// <returns></returns>
		private Service.Client ConnectToService(NetworkAddress address)
		{
			var transport = new Thrift.Transport.TSocket(address.Name, address.Port);
			transport.Open();
			try
			{
				var protocol = new Thrift.Protocol.TBinaryProtocol(transport);

				return new Service.Client(protocol);
			}
			catch
			{
				transport.Close();
				throw;
			}
		}

		private void UpdateHiveState(HiveState newState)
		{
			lock(_mapLock)
			{
				_hiveState = newState;

				// Maintain some indexes for quick access
				_workerStates.Clear();
				_columnWorkers.Clear();

				if (newState != null)
					foreach (var service in newState.Services)
						foreach (var worker in service.Value.Workers)
						{
							_workerStates.Add(worker.PodID, new Tuple<ServiceState, WorkerState>(service.Value, worker));
							foreach (var repo in worker.RepositoryStatus.Where(r => r.Value == RepositoryStatus.Online || r.Value == RepositoryStatus.Checkpointing))
							{
								PodMap map;
								if (!_columnWorkers.TryGetValue(repo.Key, out map))
								{
									map = new PodMap();
									_columnWorkers.Add(repo.Key, map);
								}
								map.Pods.Add(worker.PodID);
							}
						}
			}
		}

		/// <summary> Returns the worker for the given pod ID. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private Worker.Client EnsureWorker(int podID)
		{
			Monitor.Enter(_mapLock);
			bool taken = true;
			try
			{
				Worker.Client result;
				// Check for existing known worker
				if (!_workers.TryGetValue(podID, out result))
				{
					// Attempt to find a worker for the given pod
					Tuple<ServiceState, WorkerState> workerState;
					if (!_workerStates.TryGetValue(podID, out workerState))
						throw new Exception(String.Format("No Worker is currently associated with pod ID ({0}).", podID));

					// Release the lock during connection
					Monitor.Exit(_mapLock);
					taken = false;

					result = ConnectToWorker(new NetworkAddress { Name = workerState.Item1.Address.Name, Port = workerState.Item2.Port }, podID);

					// Take the lock again
					Monitor.Enter(_mapLock);
					taken = true;

					// Verify that the worker hasn't been found in the mean-time
					if (_workers.ContainsKey(podID))
					{
						ReleaseWorker(result);
						return _workers[podID];
					}
					else
						_workers.Add(podID, result);
				}
				return result;
			}
			finally
			{
				if (taken)
					Monitor.Exit(_mapLock);
			}
		}

		private void ReleaseWorker(Worker.Client client)
		{
			client.InputProtocol.Transport.Close();
		}

		/// <summary> Connects to a given pod ID. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private Worker.Client ConnectToWorker(NetworkAddress address, int podID)
		{
			var transport = new Thrift.Transport.TSocket(address.Name, address.Port);
			transport.Open();
			try
			{
				var protocol = new Thrift.Protocol.TBinaryProtocol(transport);

				return new Worker.Client(protocol);
			}
			catch
			{
				transport.Close();
				throw;
			}
		}

		/// <summary> Get the next worker to use for the given column ID. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private KeyValuePair<int, Worker.Client> GetWorker(int columnID)
		{
			if (columnID <= MaxSystemColumnID)
				return GetWorkerForSystemColumn();
			else
				return GetWorkerForColumn(columnID);
		}

		private KeyValuePair<int, Worker.Client> GetWorkerForColumn(int columnID)
		{
			Monitor.Enter(_mapLock);
			var taken = true;
			try
			{
				PodMap map;
				if (!_columnWorkers.TryGetValue(columnID, out map))
				{
					// TODO: Update the hive state before erroring.
					throw new Exception(String.Format("No worker is currently available for column ID ({0}).", columnID));
				}

				map.Next = (map.Next + 1) % map.Pods.Count;
				var podId = map.Pods[map.Next];

				// Release lock during ensure
				Monitor.Exit(_mapLock);
				taken = false;

				return new KeyValuePair<int, Worker.Client>(podId, EnsureWorker(podId));
			}
			finally
			{
				if (taken)
					Monitor.Exit(_mapLock);
			}
		}

		private KeyValuePair<int, Worker.Client> GetWorkerForSystemColumn()
		{
			// For a system column, any worker will do, so just use an already connected worker
			lock (_mapLock)
			{
				if (_workers.Count == 0)
					EnsureWorker(_workerStates.Keys.First());
				_nextSystemWorker = (_nextSystemWorker + 1) % _workers.Count;
				return _workers.ElementAt(_nextSystemWorker);
			}
		}

		struct WorkerInfo
		{
			public int PodID;
			public Worker.Client Client;
			public int[] Columns;
		}

		/// <summary> Get the workers to write-to for the given column IDs. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private WorkerInfo[] GetWorkers(int[] columnIDs)
		{
			Monitor.Enter(_mapLock);
			var taken = true;
			try
			{
				// TODO: is there a better better scheme than writing to every worker?  This method takes column IDs in case one arises.

				// Snapshot all needed pods
				var results = 
					(
						from ws in _workerStates 
							select new WorkerInfo() 
							{ 
								PodID = ws.Key, 
								Columns = 
								(
									from r in ws.Value.Item2.RepositoryStatus 
										where r.Value == RepositoryStatus.Online || r.Value == RepositoryStatus.Checkpointing 
										select r.Key
								).ToArray() 
							}
					).ToArray();
				
				// Release lock during ensures
				Monitor.Exit(_mapLock);
				taken = false;

				// Ensure a connections to all pods
				for (int i = 0; i < results.Length; i++)
					results[i].Client = EnsureWorker(results[i].PodID);
				
				return results;
			}
			finally
			{
				if (taken)
					Monitor.Exit(_mapLock);
			}
		}

		/// <summary> Performs a read operation against a worker and manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private void AttemptRead(int columnId, Action<Worker.Client> work)
		{
			Dictionary<int, Exception> errors = null;
			var elapsed = new Stopwatch();
			while (true)
			{
				// Determine the (next) worker to use
				var worker = GetWorker(columnId);

				// If we've already failed with this worker, give up
				if (errors != null && errors.ContainsKey(worker.Key))
				{
					TrackErrors(errors);
					throw new AggregateException(from e in errors select e.Value);
				}

				try
				{
					elapsed.Restart();
					work(worker.Value);
					elapsed.Stop();
				}
				catch (Exception e)
				{
					// If the exception is an entity (exception coming from the remote), rethrow
					if (e is Thrift.Protocol.TBase)
						throw;

					if (errors == null)
						errors = new Dictionary<int, Exception>();
					errors.Add(worker.Key, e);
					continue;
				}

				// Succeeded, track any errors we received
				if (errors != null)
					TrackErrors(errors);

				// Succeeded, track the elapsed time
				TrackTime(worker.Key, elapsed.ElapsedTicks);

				break;
			}
		}

		/// <summary> Performs a write operation against a specific worker; manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private void AttemptWrite(int podId, Action work)
		{
		    var elapsed = new Stopwatch();
		    try
		    {
		        elapsed.Start();
		        work();
		        elapsed.Stop();
		    }
		    catch (Exception e)
		    {
		        // If the exception is an entity (exception coming from the remote), rethrow
		        if (!(e is Thrift.Protocol.TBase))
					TrackErrors(new Dictionary<int, Exception> { { podId, e } });
				throw;
			}

		    // Succeeded, track the elapsed time
		    TrackTime(podId, elapsed.ElapsedTicks);
		}

		/// <summary> Tracks the time taken by the given worker. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private void TrackTime(int podId, long p)
		{
			// TODO: track the time taken by the worker for better routing
		}

		/// <summary> Tracks errors reported by workers. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private void TrackErrors(Dictionary<int, Exception> errors)
		{
			// TODO: stop trying to reach workers that keep giving errors, ask for a state update too
		}

		public Transaction Begin(bool readIsolation, bool writeIsolation)
		{
			return new Transaction(this, readIsolation, writeIsolation);
		}

		//Hit the thrift API to build range.
		public DataSet GetRange(int[] columnIds, Range range, int limit, object startId = null)
		{
			// Create the range query
			var query = CreateQuery(range, limit, startId);

			// Make the range request
			Dictionary<int, ReadResult> rangeResults = null;
			AttemptRead(range.ColumnID, (worker) => { rangeResults = worker.query(new Dictionary<int, Query> { { range.ColumnID, query } }); });
			var rangeResult = rangeResults[range.ColumnID].Answer.RangeValues[0];

			// Create the row ID query
			Query rowIdQuery = GetRowsQuery(rangeResult);

			// Make the query request against all columns except for the range column
			var tasks = new List<Task<Dictionary<int, ReadResult>>>(columnIds.Length);
			for (int i = 0; i < columnIds.Length; i++)
			{
				var columnId = columnIds[i];
				if (columnId != range.ColumnID)
				{
					tasks.Add
					(
						Task.Factory.StartNew
						(
							()=> 
							{
								Dictionary<int, ReadResult> result = null;
								AttemptRead
								(
									columnId,
									(worker) => { result = worker.query(new Dictionary<int, Query>() { { columnId, rowIdQuery } }); }
								);
								return result;
							}
						)
					);
				}
			}
			
			// Combine all results into a single dictionary by column
			var resultsByColumn = new Dictionary<int, ReadResult>(columnIds.Length);
			foreach (var task in tasks)
				foreach (var result in task.Result)
					resultsByColumn.Add(result.Key, result.Value);
			
			return RangeResultsToDataSet(columnIds, rowIdQuery.RowIDs, range.ColumnID, rangeResult, resultsByColumn);
		}

		private DataSet RangeResultsToDataSet(int[] columnIds, List<byte[]> rowIDs, int rangeColumnID, RangeResult rangeResult, Dictionary<int, ReadResult> rowResults)
		{
			DataSet result = new DataSet(rowIDs.Count, columnIds.Length);
			result.EndOfRange = rangeResult.EndOfRange;
			result.BeginOfRange = rangeResult.BeginOfRange;

			int valueRowValue = 0;
			int valueRowRow = 0;
			for (int y = 0; y < rowIDs.Count; y++)
			{
				object[] rowData = new object[columnIds.Length];
				object rowId = Fastore.Client.Encoder.Decode(rowIDs[y], _schema[columnIds[0]].IDType);
				
				for (int x = 0; x < columnIds.Length; x++)
				{
					var columnId = columnIds[x];
					if (columnId == rangeColumnID)
					{
						// Column is the range column
						rowData[x] = Fastore.Client.Encoder.Decode(rangeResult.ValueRowsList[valueRowValue].Value, _schema[columnId].Type);
						valueRowRow++;
						if (valueRowRow >= rangeResult.ValueRowsList[valueRowValue].RowIDs.Count)
						{
							valueRowValue++;
							valueRowRow = 0;
						}
					}
					else
						rowData[x] = Fastore.Client.Encoder.Decode(rowResults[columnId].Answer.RowIDValues[y], _schema[columnId].Type);
				}

				result[y] = new DataSetRow(rowId, rowData);
            }

			return result;
		}

		private static Query GetRowsQuery(RangeResult rangeResult)
		{
			List<byte[]> rowIds = new List<byte[]>();
			foreach (var valuerow in rangeResult.ValueRowsList)
			{
				foreach (var rowid in valuerow.RowIDs)
					rowIds.Add(rowid);
			}

			return new Query() { RowIDs = rowIds };
		}

		private static Query CreateQuery(Range range, int limit, object startId)
		{
			RangeRequest rangeRequest = new RangeRequest();
			rangeRequest.Ascending = range.Ascending;
			rangeRequest.__isset.limit = true;

			if (range.Start.HasValue)
			{
				Fastore.RangeBound bound = new Fastore.RangeBound();
				bound.Inclusive = range.Start.Value.Inclusive;
				bound.Value = Fastore.Client.Encoder.Encode(range.Start.Value.Bound);

				rangeRequest.First = bound;
			}

			if (range.End.HasValue)
			{
				Fastore.RangeBound bound = new Fastore.RangeBound();
				bound.Inclusive = range.End.Value.Inclusive;
				bound.Value = Fastore.Client.Encoder.Encode(range.End.Value.Bound);

				rangeRequest.Last = bound;
			}

			if (startId != null)
				rangeRequest.RowID = Fastore.Client.Encoder.Encode(startId);

			Query rangeQuery = new Query();
			rangeQuery.Ranges = new List<RangeRequest>();
			rangeQuery.Ranges.Add(rangeRequest);

			return rangeQuery;
		}

        public void Include(int[] columnIds, object rowId, object[] row)
		{
			var writes = EncodeIncludes(columnIds, rowId, row);

			while (true)
			{
				var transactionID = new TransactionID();

				var workers = GetWorkers(columnIds);

				var tasks = StartWorkerWrites(writes, transactionID, workers);

				var failedWorkers = new Dictionary<int, Thrift.Protocol.TBase>();
				var workersByTransaction = ProcessWriteResults(workers, tasks, failedWorkers);

				if (FinalizeTransaction(workers, workersByTransaction, failedWorkers))
				{
					// If we've inserted/deleted system table(s), force a schema refresh
					if (writes.ContainsKey(0))
						RefreshSchema();
					break;
				}
			}
		}

		private SortedDictionary<TransactionID, List<WorkerInfo>> ProcessWriteResults(WorkerInfo[] workers, List<Task<TransactionID>> tasks, Dictionary<int, Thrift.Protocol.TBase> failedWorkers)
		{
			var stopWatch = new Stopwatch();
			stopWatch.Start();
			var workersByTransaction = new SortedDictionary<TransactionID, List<WorkerInfo>>();
			for (int i = 0; i < tasks.Count; i++)
			{
				if (tasks[i].IsCompleted)
				{
					// Attempt to fetch the result for each task
					TransactionID resultId;
					try
					{
						// if the task doesn't complete in time, assume failure; move on to the next one...
						if (!tasks[i].Wait(Math.Max(0, WriteTimeout - (int)stopWatch.ElapsedMilliseconds)))
						{
							failedWorkers.Add(i, null);
							continue;
						}
						resultId = tasks[i].Result;
					}
					catch (Exception e)
					{
						if (e is Thrift.Protocol.TBase)
							failedWorkers.Add(i, e as Thrift.Protocol.TBase);
						// else: Other errors were managed by AttemptWrite
						continue;
					}

					// If successful, group with other workers that returned the same revision
					List<WorkerInfo> transactionBucket;
					if (!workersByTransaction.TryGetValue(resultId, out transactionBucket))
					{
						transactionBucket = new List<WorkerInfo>();
						workersByTransaction.Add(resultId, transactionBucket);
					}
					transactionBucket.Add(workers[i]);
				}
			}
			return workersByTransaction;
		}

		private bool FinalizeTransaction(WorkerInfo[] workers, SortedDictionary<TransactionID, List<WorkerInfo>> workersByTransaction, Dictionary<int, Thrift.Protocol.TBase> failedWorkers)
		{
			if (workersByTransaction.Count > 0)
			{
				var max = workersByTransaction.Keys.Max();
				var successes = workersByTransaction[max];
				if (successes.Count > (workers.Length / 2))
				{
					// Transaction successful, commit all reached workers
					foreach (var group in workersByTransaction)
						foreach (var worker in group.Value)
							worker.Client.commit(max);
				
					// Also send out a commit to workers that timed-out
					foreach (var worker in failedWorkers.Where(w => w.Value == null))
						workers[worker.Key].Client.commit(max);
					return true;
				}
				else
				{
					// Failure, roll-back successful prepares
					foreach (var group in workersByTransaction)
						foreach (var worker in group.Value)
							worker.Client.rollback(group.Key);
				}
			}
			return false;
		}

		/// <summary> Apply the writes to each worker, even if there are no modifications for that worker. </summary>
		private List<Task<TransactionID>> StartWorkerWrites(Dictionary<int, ColumnWrites> writes, TransactionID transactionID, WorkerInfo[] workers)
		{
			var tasks = new List<Task<TransactionID>>(workers.Length);
			// Apply the modification to every worker, even if no work
			foreach (var worker in workers)
			{
				// Determine the set of writes that apply to the worker's repositories
				var work = new Dictionary<int, ColumnWrites>();
				foreach (var columnId in worker.Columns.Where(c => writes.ContainsKey(c)))
					work.Add(columnId, writes[columnId]);

				tasks.Add
				(
					Task.Factory.StartNew
					(
						() =>
						{
							TransactionID result = new TransactionID();
							AttemptWrite
							(
								worker.PodID,
								() => { result = worker.Client.apply(transactionID, work); }
							);
							return result;
						}
					)
				);
			}
			return tasks;
		}

		private Dictionary<int, ColumnWrites> EncodeIncludes(int[] columnIds, object rowId, object[] row)
		{
			var writes = new Dictionary<int, ColumnWrites>();
			byte[] rowIdb = Fastore.Client.Encoder.Encode(rowId);

			for (int i = 0; i < columnIds.Length; i++)
			{
				Include inc = new Fastore.Include();
				inc.RowID = rowIdb;
				inc.Value = Fastore.Client.Encoder.Encode(row[i]);

				ColumnWrites wt = new ColumnWrites();
				wt.Includes = new List<Fastore.Include>();
				wt.Includes.Add(inc);
				writes.Add(columnIds[i], wt);
			}
			return writes;
		}

		public void Exclude(int[] columnIds, object rowId)
		{
            var writes = EncodeExcludes(columnIds, rowId);

            while (true)
            {
                var transactionID = new TransactionID();

                var workers = GetWorkers(columnIds);

                var tasks = StartWorkerWrites(writes, transactionID, workers);

                var failedWorkers = new Dictionary<int, Thrift.Protocol.TBase>();
                var workersByTransaction = ProcessWriteResults(workers, tasks, failedWorkers);

                if (FinalizeTransaction(workers, workersByTransaction, failedWorkers))
                {
                    // If we've inserted/deleted system table(s), force a schema refresh
                    if (writes.ContainsKey(0))
                        RefreshSchema();
                    break;
                }
            }
		}

        private Dictionary<int, ColumnWrites> EncodeExcludes(int[] columnIds, object rowId)
        {
            var writes = new Dictionary<int, ColumnWrites>();
            byte[] rowIdb = Fastore.Client.Encoder.Encode(rowId);

            for (int i = 0; i < columnIds.Length; i++)
            {
                Exclude inc = new Fastore.Exclude();
                inc.RowID = rowIdb;

                ColumnWrites wt = new ColumnWrites();
                wt.Excludes = new List<Fastore.Exclude>();
                wt.Excludes.Add(inc);
                writes.Add(columnIds[i], wt);
            }
            return writes;
        }

		public Statistic[] GetStatistics(int[] columnIds)
		{
			throw new NotImplementedException();
			//return 
			//(
			//    from s in Service.getStatistics(columnIds.ToList()) 
			//        select new Statistic { Total = s.Total, Unique = s.Unique }
			//).ToArray();
		}

		public Schema GetSchema()
		{
			if (_schema == null)
				_schema = LoadSchema();
			return new Schema(_schema);
		}

		private Schema LoadSchema()
		{
			var schema = new Schema();
			var columns =
				GetRange
				(
					new[] { 0, 1, 2, 3, 4 },
					new Range { ColumnID = 0, Ascending = true },
					int.MaxValue
				);
			foreach (var column in columns)
			{
				var def =
					new ColumnDef
					{
						ColumnID = (int)column.Values[0],
						Name = (string)column.Values[1],
						Type = (string)column.Values[2],
						IDType = (string)column.Values[3],
						IsUnique = (bool)column.Values[4]
					};
				schema.Add(def.ColumnID, def);
			}
			return schema;
		}

		public void RefreshSchema()
		{
            _schema = LoadSchema();
		}

        private void BootStrapSchema()
        {
            // Actually, we only need the ID and Type to bootstrap properly.
            ColumnDef id = new ColumnDef();
            ColumnDef name = new ColumnDef();
            ColumnDef vt = new ColumnDef();
            ColumnDef idt = new ColumnDef();
            ColumnDef unique = new ColumnDef();

            id.ColumnID = 0;
            id.Name = "Column.ID";
            id.Type = "Int";
            id.IDType = "Int";
            id.IsUnique = true;

            name.ColumnID = 1;
            name.Name = "Column.Name";
            name.Type = "String";
            name.IDType = "Int";
            name.IsUnique = false;

            vt.ColumnID = 2;
            vt.Name = "Column.ValueType";
            vt.Type = "String";
            vt.IDType = "Int";
            vt.IsUnique = false;

            idt.ColumnID = 3;
            idt.Name = "Column.RowIDType";
            idt.Type = "String";
            idt.IDType = "Int";
            idt.IsUnique = false;

            unique.ColumnID = 4;
            unique.Name = "Column.IsUnique";
            unique.Type = "Bool";
            unique.IDType = "Int";
            unique.IsUnique = false;

            _schema = new Schema();

            _schema.Add(0, id);
            _schema.Add(1, name);
            _schema.Add(2, vt);
            _schema.Add(3, idt);
            _schema.Add(4, unique);

            //Boot strapping is done, pull in real schema
            RefreshSchema();
        }
	}
}
