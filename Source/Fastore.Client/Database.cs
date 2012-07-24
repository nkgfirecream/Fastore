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
		/// <summary> The maximum number of rows to attempt to fetch at one time. </summary>
		public const int MaxFetchLimit = 500;

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

		// Connection pool of services by host ID
		private ConnectionPool<int, Service.Client> _services;

		// Connected workers by pod ID
		private ConnectionPool<int, Worker.Client> _workers;

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
			_services =
				new ConnectionPool<int, Service.Client>
				(
					(proto) => new Service.Client(proto),
					new Func<int, NetworkAddress>(GetServiceAddress),
					(client) => { client.InputProtocol.Transport.Close(); },
					(client) => client.InputProtocol.Transport.IsOpen
				);

			_workers =
				new ConnectionPool<int, Worker.Client>
				(
					(proto) => new Worker.Client(proto),
					new Func<int, NetworkAddress>(GetWorkerAddress),
					(client) => { client.InputProtocol.Transport.Close(); },
					(client) => client.InputProtocol.Transport.IsOpen
				);

			// Convert from service addresses to network addresses
			var networkAddresses = (from a in addresses select new NetworkAddress { Name = a.Name, Port = a.Port }).ToArray();

			// Number of potential workers for each service (in case we nee to create the hive)
			var serviceWorkers = new int[networkAddresses.Length];

			for (int i = 0; i < networkAddresses.Length; i++)
			{
				var service = _services.Connect(networkAddresses[i]);
				try
				{
					// Discover the state of the entire hive from the given service
					var hiveStateResult = service.getHiveState(false);
					if (!hiveStateResult.__isset.hiveState)
					{
						// If no hive state is given, the service is not joined, we should proceed to discover the number 
						//  of potential workers for the rest of the services ensuring that they are all not joined.
						serviceWorkers[i] = hiveStateResult.PotentialWorkers;
						_services.Destroy(service);
						continue;
					}

					// If we have passed the first host, we are in "discovery" mode for a new topology so we find any services that are joined.
					if (i > 0)
						throw new ClientException(String.Format("Service '{0}' is joined to topology {1}, while at least one other specified service is not part of any topology.", networkAddresses[i].Name, hiveStateResult.HiveState.TopologyID));

                    UpdateHiveState(hiveStateResult.HiveState);
                    BootStrapSchema();
                    RefreshHiveState();
                    
                    //Everything worked... exit function
                    return;
				}
				catch
				{
					// If anything goes wrong, be sure to release the service client
					_services.Destroy(service);
					_services.Dispose();
					throw;
				}
			}

            //Create a new topology instead.
			var newTopology = CreateTopology(serviceWorkers);

			var addressesByHost = new Dictionary<int, NetworkAddress>();
			for (var hostID = 0; hostID < networkAddresses.Length; hostID++)
				addressesByHost.Add(hostID, networkAddresses[hostID]);

			var newHive = new HiveState { TopologyID = newTopology.TopologyID, Services = new Dictionary<int, ServiceState>() };
			for (var hostID = 0; hostID < networkAddresses.Length; hostID++)
			{
				var service = _services.Connect(networkAddresses[hostID]);
				try
				{
					var serviceState = service.init(newTopology, addressesByHost, hostID);
					newHive.Services.Add(hostID, serviceState);
				}
				finally
				{
					_services.Destroy(service);
				}
			}

			UpdateHiveState(newHive);
			BootStrapSchema();
			UpdateTopologySchema(newTopology);
		}

		private void UpdateTopologySchema(Topology newTopology)
		{
			var writes = new Dictionary<int, ColumnWrites>();

			// Insert the topology
			writes.Add
			(
				Dictionary.TopologyID, 
				new ColumnWrites 
				{ 
					Includes = new List<Fastore.Include> 
					{ 
						new Fastore.Include { RowID = Encoder.Encode(newTopology.TopologyID), Value = Encoder.Encode(newTopology.TopologyID) } 
					} 
				}
			);

			// Insert the hosts and pods
			var hostWrites = new ColumnWrites { Includes = new List<Fastore.Include>() };
			var podWrites = new ColumnWrites { Includes = new List<Fastore.Include>() };
			var podHostWrites = new ColumnWrites { Includes = new List<Fastore.Include>() };
			foreach (var h in newTopology.Hosts)
			{
				hostWrites.Includes.Add(new Fastore.Include { RowID = Encoder.Encode(h.Key), Value = Encoder.Encode(h.Key) });
				foreach (var p in h.Value)
				{
					podWrites.Includes.Add(new Fastore.Include { RowID = Encoder.Encode(p.Key), Value = Encoder.Encode(p.Key) });
					podHostWrites.Includes.Add(new Fastore.Include { RowID = Encoder.Encode(p.Key), Value = Encoder.Encode(h.Key) });
				}
			}
			writes.Add(Dictionary.HostID, hostWrites);
			writes.Add(Dictionary.PodID, podWrites);
			writes.Add(Dictionary.PodHostID, podHostWrites);

			Apply(writes, false);
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
			ClientException.ForceCleanup
			(
				() => { _services.Dispose(); },
				() => { _workers.Dispose(); },
				() => { UpdateHiveState(null); }
			);
		}

		private NetworkAddress GetServiceAddress(int hostID)
		{
			lock (_mapLock)
			{
				ServiceState serviceState;
				if (!_hiveState.Services.TryGetValue(hostID, out serviceState))
					throw new ClientException(String.Format("No service is currently associated with host ID ({0}).", hostID));

				return serviceState.Address;
			}
		}

        private HiveState GetHiveState()
        {
            //TODO: Need to actually try to get new worker information, update topologyID, etc.
            //This just assumes that we won't add any workers once we are up and running
            var newHive = new HiveState() { TopologyID = 0, Services = new Dictionary<int, ServiceState>() };

            var tasks = new List<Task<KeyValuePair<int, ServiceState>>>();
            foreach (var service in _hiveState.Services)
            {
                tasks.Add
                (
                    Task.Factory.StartNew
                    (
                        () =>
                        {
                            var serviceClient = _services[service.Key];
                            var state = serviceClient.getState();
							if (!state.__isset.serviceState)
								throw new ClientException(String.Format("Host ({0}) is unexpectedly not part of the topology.", service.Key)); 
                            _services.Release(new KeyValuePair<int,Service.Client>(service.Key, serviceClient));
                            return new KeyValuePair<int, ServiceState>(service.Key, state.ServiceState);
                        }
                    )
                );
            }

            foreach (var task in tasks)
            {
                var statePair = task.Result;
                newHive.Services.Add(statePair.Key, statePair.Value);
            }

            //Don't care for now...
            newHive.ReportingHostID = newHive.Services.Keys.First();

            return newHive;
        }

        private void RefreshHiveState()
        {
            HiveState newState = GetHiveState();
            UpdateHiveState(newState);
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
                            //TODO: Don't assume all repos are online.
							foreach (var repo in worker.RepositoryStatus) //.Where(r => r.Value == RepositoryStatus.Online || r.Value == RepositoryStatus.Checkpointing)
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

		private NetworkAddress GetWorkerAddress(int podID)
		{
			lock (_mapLock)
			{
				// Attempt to find a worker for the given pod
				Tuple<ServiceState, WorkerState> workerState;
				if (!_workerStates.TryGetValue(podID, out workerState))
					throw new ClientException(String.Format("No Worker is currently associated with pod ID ({0}).", podID), ClientException.Codes.NoWorkerForColumn);

				return new NetworkAddress { Name = workerState.Item1.Address.Name, Port = workerState.Item2.Port };
			}
		}


		/// <summary> Get the next worker to use for the given column ID. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private KeyValuePair<int, Worker.Client> GetWorker(int columnID)
		{
			if (columnID <= Dictionary.MaxSystemColumnID)
				return GetWorkerForSystemColumn();
			else
				return GetWorkerForColumn(columnID);
		}

		private KeyValuePair<int, Worker.Client> GetWorkerForColumn(int columnID)
		{
			var podId = GetWorkerIDForColumn(columnID);
			return new KeyValuePair<int, Worker.Client>(podId, _workers[podId]);
		}

		private int GetWorkerIDForColumn(int columnID)
		{
			lock (_mapLock)
			{
				PodMap map;
				if (!_columnWorkers.TryGetValue(columnID, out map))
				{
					// TODO: Update the hive state before erroring.

					var error = new ClientException(String.Format("No worker is currently available for column ID ({0}).", columnID), ClientException.Codes.NoWorkerForColumn);
					error.Data.Add("ColumnID", columnID);
					throw error;
				}

				map.Next = (map.Next + 1) % map.Pods.Count;
				var podId = map.Pods[map.Next];

				return podId;
			}
		}

		private KeyValuePair<int, Worker.Client> GetWorkerForSystemColumn()
		{
			// For a system column, any worker will do, so just use an already connected worker
			int podID;
			lock (_mapLock)
			{
				podID = _workerStates.Keys.ElementAt(_nextSystemWorker);
				_nextSystemWorker = (_nextSystemWorker + 1) % _workerStates.Count;
			}
			return new KeyValuePair<int, Worker.Client>(podID, _workers[podID]);
		}

		struct WorkerInfo
		{
			public int PodID;
			public int[] Columns;
		}

		/// <summary> Determine the workers to write-to for the given column IDs. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private WorkerInfo[] DetermineWorkers(Dictionary<int, ColumnWrites> writes)
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
								).Union(from c in _schema.Keys where c <= Dictionary.MaxSystemColumnID select c).ToArray()
							}
					).ToArray();
				
				// Release lock during ensures
				Monitor.Exit(_mapLock);
				taken = false;

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
				try
				{
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
				finally
				{
					_workers.Release(worker);
				}
			}
		}

		/// <summary> Performs a write operation against a specific worker; manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private void AttemptWrite(int podId, Action<Worker.Client> work)
		{
		    var elapsed = new Stopwatch();
		    try
		    {
		        elapsed.Start();
		        WorkerInvoke(podId, work);
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

		/// <summary> Given a set of column IDs and range criteria, retrieve a set of values. </summary>
		public RangeSet GetRange(int[] columnIds, Range range, int limit = MaxFetchLimit, object startId = null)
		{
			// Create the range query
			var query = CreateQuery(range, limit, startId);

			// Make the range request
			Dictionary<int, ReadResult> rangeResults = null;
			AttemptRead(range.ColumnID, (worker) => { rangeResults = worker.query(new Dictionary<int, Query> { { range.ColumnID, query } }); });
			var rangeResult = rangeResults[range.ColumnID].Answer.RangeValues[0];

			// Create the row ID query
			Query rowIdQuery = GetRowsQuery(rangeResult);

			// Get the values for all but the range
			var result = InternalGetValues(columnIds, range.ColumnID, rowIdQuery);

			// Add the range values into the result
			return ResultsToRangeSet(result, range.ColumnID, Array.IndexOf(columnIds, range.ColumnID), rangeResult);
		}

		private DataSet InternalGetValues(int[] columnIds, int exclusionColumnId, Query rowIdQuery)
		{
			// Make the query request against all columns except for the range column
			var tasks = new List<Task<Dictionary<int, ReadResult>>>(columnIds.Length);
			for (int i = 0; i < columnIds.Length; i++)
			{
				var columnId = columnIds[i];
				if (columnId != exclusionColumnId)
				{
					tasks.Add
					(
						Task.Factory.StartNew
						(
							() =>
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

			return ResultsToDataSet(columnIds, rowIdQuery.RowIDs, resultsByColumn);
		}

		public DataSet GetValues(int[] columnIds, object[] rowIds)
		{
			// Create the row ID query
			Query rowIdQuery = new Query { RowIDs = EncodeRowIds(rowIds) };

			// Make the query
			return InternalGetValues(columnIds, -1, rowIdQuery); 
		}

		private static List<byte[]> EncodeRowIds(object[] rowIds)
		{
			var encodedRowIds = new List<byte[]>(rowIds.Length);
			for (int i = 0; i < rowIds.Length; i++)
				encodedRowIds.Add(Encoder.Encode(rowIds[i]));
			return encodedRowIds;
		}

		private DataSet ResultsToDataSet(int[] columnIds, List<byte[]> rowIDs, Dictionary<int, ReadResult> rowResults)
		{
			DataSet result = new DataSet(rowIDs.Count, columnIds.Length);

			for (int y = 0; y < rowIDs.Count; y++)
			{
				object[] rowData = new object[columnIds.Length];
				object rowId = Fastore.Client.Encoder.Decode(rowIDs[y], _schema[columnIds[0]].IDType);

				for (int x = 0; x < columnIds.Length; x++)
				{
					var columnId = columnIds[x];
					if (rowResults.ContainsKey(columnId))
						rowData[x] = Fastore.Client.Encoder.Decode(rowResults[columnId].Answer.RowIDValues[y], _schema[columnId].Type);
				}

				result[y] = new DataSet.DataSetRow { ID = rowId, Values = rowData };
			}

			return result;
		}

		private RangeSet ResultsToRangeSet(DataSet set, int rangeColumnId, int rangeColumnIndex, RangeResult rangeResult)
		{
			var result = 
				new RangeSet 
				{ 
					Eof = rangeResult.Eof, 
					Bof = rangeResult.Bof, 
					Limited = rangeResult.Limited,
					Data = set
				};

			int valueRowValue = 0;
			int valueRowRow = 0;
			for (int y = 0; y < set.Count; y++)
			{
				set[y].Values[rangeColumnIndex] = Fastore.Client.Encoder.Decode(rangeResult.ValueRowsList[valueRowValue].Value, _schema[rangeColumnId].Type);
				valueRowRow++;
				if (valueRowRow >= rangeResult.ValueRowsList[valueRowValue].RowIDs.Count)
				{
					valueRowValue++;
					valueRowRow = 0;
				}
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
            rangeRequest.Limit = limit;

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

        internal void Apply(Dictionary<int, ColumnWrites> writes, bool flush)
        {
			if (writes.Count > 0)
				while (true)
				{
					var transactionID = new TransactionID() { Key = 0, Revision = 0 };

					var workers = DetermineWorkers(writes);

					var tasks = StartWorkerWrites(writes, transactionID, workers);

					var failedWorkers = new Dictionary<int, Thrift.Protocol.TBase>();
					var workersByTransaction = ProcessWriteResults(workers, tasks, failedWorkers);

					if (FinalizeTransaction(workers, workersByTransaction, failedWorkers))
					{
						// If we've inserted/deleted system table(s), force a schema refresh
						if (writes.Keys.Min() <= Dictionary.MaxSystemColumnID)
						{
							RefreshSchema();
							RefreshHiveState();
						}

						if (flush)
							FlushWorkers(transactionID, workers);
						break;
					}
				}
        }

		private void FlushWorkers(TransactionID transactionID, WorkerInfo[] workers)
		{
			var flushTasks =
			(
				from w in workers
				select
					Task.Factory.StartNew
					(
					() =>
					{
						var worker = _workers[w.PodID];
						try
						{
							worker.flush(transactionID);
						}
						finally
						{
							_workers.Release(new KeyValuePair<int, Worker.Client>(w.PodID, worker));
						}
					}
					)
			).ToArray();

			// TODO: need algorithm that moves on as soon as needed quantity reached without waiting for slower ones

			// Wait for critical number of workers to flush
			var neededCount = Math.Max(1, workers.Length / 2);
			for (var i = 0; i < flushTasks.Length && i < neededCount; i++)
				flushTasks[i].Wait();
		}

        public void Include(int[] columnIds, object rowId, object[] row)
		{
			var writes = EncodeIncludes(columnIds, rowId, row);
            Apply(writes, false);
		}

		private SortedDictionary<TransactionID, List<WorkerInfo>> ProcessWriteResults(WorkerInfo[] workers, List<Task<TransactionID>> tasks, Dictionary<int, Thrift.Protocol.TBase> failedWorkers)
		{
			var stopWatch = new Stopwatch();
			stopWatch.Start();
			var workersByTransaction = new SortedDictionary<TransactionID, List<WorkerInfo>>(TransactionIDComparer.Default);
			for (int i = 0; i < tasks.Count; i++)
			{
				// Attempt to fetch the result for each task
				TransactionID resultId;
				try
				{
					//// if the task doesn't complete in time, assume failure; move on to the next one...
					//if (!tasks[i].Wait(Math.Max(0, WriteTimeout - (int)stopWatch.ElapsedMilliseconds)))
					//{
					//	failedWorkers.Add(i, null);
					//	continue;
					//}
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
							WorkerInvoke(worker.PodID, (client) => { client.commit(max); });
				
					// Also send out a commit to workers that timed-out
					foreach (var worker in failedWorkers.Where(w => w.Value == null))
						WorkerInvoke(worker.Key, (client) => { client.commit(max); });
					return true;
				}
				else
				{
					// Failure, roll-back successful prepares
					foreach (var group in workersByTransaction)
						foreach (var worker in group.Value)
							WorkerInvoke(worker.PodID, (client) => { client.rollback(group.Key); });
				}
			}
			return false;
		}

		/// <summary> Invokes a given command against a worker. </summary>
		private void WorkerInvoke(int podID, Action<Worker.Client> work)
		{
			var client = _workers[podID];
			try
			{
				work(client);
			}
			finally
			{
				_workers.Release(new KeyValuePair<int, Worker.Client>(podID, client));
			}
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
								(client) => { result = client.apply(transactionID, work); }
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
            Apply(EncodeExcludes(columnIds, rowId), false);
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
			// Make the request against each column
			var tasks = new List<Task<Fastore.Statistic>>(columnIds.Length);
			for (var i = 0; i < columnIds.Length; i++)
			{
				var columnId = columnIds[i];
				tasks.Add
				(
					Task.Factory.StartNew
					(
						() =>
						{
							Fastore.Statistic result = null;
							AttemptRead
							(
								columnId,
								(worker) => { result = worker.getStatistics(new List<int> { columnId })[0]; }
							);
							return result;
						}
					)
				);
			}

			return 
			(
				from t in tasks let r = t.Result 
					select new Statistic { Total = r.Total, Unique = r.Unique }
			).ToArray();
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
			var finished = false;
			while (!finished)
			{
				var columns =
					GetRange
					(
						Dictionary.ColumnColumns,
						new Range { ColumnID = Dictionary.ColumnID, Ascending = true },
						MaxFetchLimit
					);
				foreach (var column in columns.Data)
				{
					var def =
						new ColumnDef
						{
							ColumnID = (int)column.Values[0],
							Name = (string)column.Values[1],
							Type = (string)column.Values[2],
							IDType = (string)column.Values[3],
							BufferType = (BufferType)column.Values[4]
						};
					schema.Add(def.ColumnID, def);
				}
				finished = !columns.Limited;
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

            id.ColumnID = Dictionary.ColumnID;
            id.Name = "Column.ID";
            id.Type = "Int";
            id.IDType = "Int";
            id.BufferType = BufferType.Identity;

            name.ColumnID = Dictionary.ColumnName;
            name.Name = "Column.Name";
            name.Type = "String";
            name.IDType = "Int";
            name.BufferType = BufferType.Unique;

            vt.ColumnID = Dictionary.ColumnValueType;
            vt.Name = "Column.ValueType";
            vt.Type = "String";
            vt.IDType = "Int";
            vt.BufferType = BufferType.Multi;

            idt.ColumnID = Dictionary.ColumnRowIDType;
            idt.Name = "Column.RowIDType";
            idt.Type = "String";
            idt.IDType = "Int";
            idt.BufferType = BufferType.Multi;

            unique.ColumnID = Dictionary.ColumnBufferType;
            unique.Name = "Column.BufferType";
            unique.Type = "Int";
            unique.IDType = "Int";
            unique.BufferType = BufferType.Multi;

            _schema = new Schema();

            _schema.Add(Dictionary.ColumnID, id);
			_schema.Add(Dictionary.ColumnName, name);
			_schema.Add(Dictionary.ColumnValueType, vt);
			_schema.Add(Dictionary.ColumnRowIDType, idt);
			_schema.Add(Dictionary.ColumnBufferType, unique);

            //Boot strapping is done, pull in real schema
            RefreshSchema();
        }

		public Dictionary<int, TimeSpan> Ping()
		{
			// Setup the set of hosts
			int[] hostIDs;
			lock (_mapLock)
			{
				hostIDs = _hiveState.Services.Keys.ToArray();
			}

			// Start a ping request to each
			var tasks =
				from id in hostIDs
				select Task.Factory.StartNew<KeyValuePair<int, TimeSpan>>
				(
					() =>
					{
						var service = _services[id];
						try
						{
							var timer = new Stopwatch();
							timer.Start();
							service.ping();
							timer.Stop();
							return new KeyValuePair<int, TimeSpan>(id, timer.Elapsed);
						}
						finally
						{
							_services.Release(new KeyValuePair<int, Service.Client>(id, service));
						}
					}
				);
			
			// Gather ping results
			var result = new Dictionary<int, TimeSpan>(hostIDs.Length);
			foreach (var task in tasks)
			{
				try
				{
					var item = task.Result;
					result.Add(item.Key, item.Value);
				}
				catch
				{
					// TODO: report errors
				}
			}

			return result;
		}
	}
}
