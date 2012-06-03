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

		// Latest known state of the hive
		private HiveState _hiveState;

		// Currently known schema
		private Schema _schema;

		/// <summary> The default apply timeout. </summary>
		public const int DefaultApplyTimeout = 1000;

		private int _applyTimeout = DefaultApplyTimeout;
		/// <summary> ApplyTimeout specifies the maximum time in milliseconds to wait for workers to respond to an apply request. </summary>
		/// <remarks> The default is 1000 (1 second). </remarks>
		public int ApplyTimeout 
		{ 
			get { return _applyTimeout; } 
			set
			{
				if (value < -1)
					throw new ArgumentOutOfRangeException("value", "ApplyTimeout must be -1 or greater.");
				_applyTimeout = value;
			}
		}

		public Database(string address, int port)
        {
			var service = ConnectToService(address, port);
			try
			{
				// Discover the state of the entire hive from the given service
				var hiveState = service.getHiveState();
				UpdateHiveState(hiveState);

				// Add the service to our collection to avoid re-connection if needed
				_services.Add(hiveState.HostID, service);

				BootStrapSchema();
			}
			catch
			{
				// If anything goes wrong, be sure to release the service client
				ReleaseService(service);
				throw;
			}
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

					result = ConnectToService(serviceState.Address, serviceState.Port);

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
		private Service.Client ConnectToService(string address, int port)
		{
			var transport = new Thrift.Transport.TSocket(address, port);
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
							_workerStates.Add(worker.Key, new Tuple<ServiceState, WorkerState>(service.Value, worker.Value));
							foreach (var repo in worker.Value.RepositoryStatus.Where(r => r.Value == RepositoryStatus.Online || r.Value == RepositoryStatus.Checkpointing))
							{
								PodMap map;
								if (!_columnWorkers.TryGetValue(repo.Key, out map))
								{
									map = new PodMap();
									_columnWorkers.Add(repo.Key, map);
								}
								map.Pods.Add(worker.Key);
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

					result = ConnectToWorker(workerState.Item1.Address, workerState.Item2.Port, podID);

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
		private Worker.Client ConnectToWorker(string address, int port, int podID)
		{
			var transport = new Thrift.Transport.TSocket(address, port);
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
		private Worker.Client GetWorker(int columnID)
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
				var worker = map.Pods[map.Next];

				// Release lock during ensure
				Monitor.Exit(_mapLock);
				taken = false;

				return EnsureWorker(worker);
			}
			finally
			{
				if (taken)
					Monitor.Exit(_mapLock);
			}
		}

		/// <summary> Get the workers to use for the given column ID. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private Worker.Client[] GetWorkers(int columnID)
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

				var pods = map.Pods.ToArray();

				// Release lock during ensures
				Monitor.Exit(_mapLock);
				taken = false;

				var result = new Worker.Client[pods.Length];
				for (int i = 0; i < result.Length; i++)
					result[i] = EnsureWorker(pods[i]);
				
				return result;
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
			Dictionary<Worker.Client, Exception> errors = null;
			var elapsed = new Stopwatch();
			while (true)
			{
				// Determine the worker to use
				var worker = GetWorker(columnId);

				// If we've already failed with this worker, give up
				if (errors != null && errors.ContainsKey(worker))
					throw new AggregateException(from e in errors select e.Value);

				try
				{
					elapsed.Restart();
					work(worker);
					elapsed.Stop();
				}
				catch (Exception e)
				{
					// If the exception is an entity (exception coming from the remote), rethrow
					if (e is Thrift.Protocol.TBase)
						throw;

					if (errors == null)
						errors = new Dictionary<Worker.Client, Exception>();
					errors.Add(worker, e);
				}

				// Succeeded, track any errors we received
				if (errors != null)
					TrackErrors(errors);

				// Succeeded, track the elapsed time
				TrackTime(worker, elapsed.ElapsedTicks);
			}
		}

		///// <summary> Performs a write operation against all applicable workers and manages errors and retries. </summary>
		///// <remarks> This method is thread-safe. </remarks>
		//private void AttemptWrite(int columnIds[], Action<Worker.Client> work)
		//{
		//    Dictionary<Worker.Client, Exception> errors = null;
		//    var elapsed = new Stopwatch();
		//    while (true)
		//    {
		//        // Determine the worker to use
		//        var workers = GetWorkers(columnId);

		//        try
		//        {
		//            elapsed.Restart();
		//            work(worker);
		//            elapsed.Stop();
		//        }
		//        catch (Exception e)
		//        {
		//            // If the exception is an entity (exception coming from the remote), rethrow
		//            if (e is Thrift.Protocol.TBase)
		//                throw;

		//            if (errors == null)
		//                errors = new Dictionary<Worker.Client, Exception>();
		//            errors.Add(worker, e);
		//        }

		//        // Succeeded, track any errors we received
		//        if (errors != null)
		//            TrackErrors(errors);

		//        // Succeeded, track the elapsed time
		//        TrackTime(worker, elapsed.ElapsedTicks);
		//    }
		//}

		/// <summary> Tracks the time taken by the given worker. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private void TrackTime(Worker.Client worker, long p)
		{
			// TODO: track the time taken by the worker for better routing
		}

		/// <summary> Tracks errors reported by workers. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		private void TrackErrors(Dictionary<Worker.Client, Exception> errors)
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

				result[y] = new DataSetRow(rowData, rowId);
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
			rangeQuery.Limit = limit;

			return rangeQuery;
		}

        public void Include(int[] columnIds, object rowId, object[] row)
		{
			throw new NotImplementedException();
			//var writes = EncodeIncludes(columnIds, rowId, row);

			//var transactionID = new TransactionID();

			//// Apply the modification to each worker and wait for all results to arrive
			//var tasks = new List<Task<TransactionID>>(writes.Count);
			//foreach (var write in writes)
			//{
			//    tasks.Add
			//    (
			//        Task.Factory.StartNew
			//        (
			//            () => 
			//            {
			//                TransactionID result = new TransactionID();
			//                AttemptRead
			//                (
			//                    write.Key, 
			//                    (worker) => { result = worker.apply(transactionID, new Dictionary<int, ColumnWrites> { { write.Key, write.Value } }); }
			//                );
			//                return result;
			//            }
			//        )
			//    );
			//}
			//Task.WaitAll(tasks.ToArray(), ApplyTimeout);

			//if (columnIds[0] == 0)
			//    RefreshSchema();
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
			throw new NotImplementedException();
			//Dictionary<int, ColumnWrites> writes = new Dictionary<int, ColumnWrites>();
			//byte[] rowIdb = Fastore.Client.Encoder.Encode(rowId);
            
			//Exclude ex = new Fastore.Exclude();
			//ex.RowID = rowIdb;

			//ColumnWrites wt = new ColumnWrites();
			//wt.Excludes = new List<Fastore.Exclude>();
			//wt.Excludes.Add(ex);

			//for (int i = 0; i < columnIds.Length; i++)
			//{
			//    writes.Add(columnIds[i], wt);
			//}

			//Service.apply(_defaultId, writes);

			//if (columnIds[0] == 0)
			//    RefreshSchema();
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
