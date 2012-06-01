using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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

		private Dictionary<int, Service.Client> _services = new Dictionary<int, Service.Client>();
		private Dictionary<int, Worker.Client> _workers = new Dictionary<int, Worker.Client>();
		private Dictionary<int, Tuple<ServiceState, WorkerState>> _workerStates = new Dictionary<int, Tuple<ServiceState, WorkerState>>();
		private Dictionary<int, PodMap> _columnWorkers = new Dictionary<int, PodMap>();
		private HiveState _hiveState;
		private Schema _schema;
        private TransactionID _defaultId = new TransactionID() { Key = 0, Revision = 0 };

		public Database(string address, int port)
        {
			var service = ConnectToService(address, port);
            
            //GetRange to determine the schema requires a few assumptions
            //about the schema in order to be able to decode it properly.
            //(Like, the types of the schema columns)
            BootStrapSchema();           
        }

		private Service.Client EnsureService(int hostID)
		{
			Service.Client result;
			if (!_services.TryGetValue(hostID, out result))
			{
				ServiceState serviceState;
				if (!_hiveState.Services.TryGetValue(hostID, out serviceState))
					throw new Exception(String.Format("No service is currently associated with host ID ({0}).", hostID));
				result = ConnectToService(serviceState.Address, serviceState.Port);
			}
			return result;
		}

		private Service.Client ConnectToService(string address, int port, int hostID = -1)
		{
			var transport = new Thrift.Transport.TSocket(address, port);
			transport.Open();
			try
			{
				var protocol = new Thrift.Protocol.TBinaryProtocol(transport);

				var service = new Service.Client(protocol);

				// If hostID isn't provided, ask the service for its host ID
				if (hostID < 0)
				{
					UpdateHiveState(service.getHiveState());
					hostID = _hiveState.HostID;
				}

				_services.Add(hostID, service);

				return service;
			}
			catch
			{
				transport.Close();
				throw;
			}
		}

		private void UpdateHiveState(HiveState newState)
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

		private Worker.Client EnsureWorker(int podID)
		{
			Worker.Client result;
			if (!_workers.TryGetValue(podID, out result))
			{
				Tuple<ServiceState, WorkerState> workerState;
				if (!_workerStates.TryGetValue(podID, out workerState))
					throw new Exception(String.Format("No Worker is currently associated with pod ID ({0}).", podID));
				result = ConnectToWorker(workerState.Item1.Address, workerState.Item2.Port, podID);
			}
			return result;
		}

		private Worker.Client ConnectToWorker(string address, int port, int podID)
		{
			var transport = new Thrift.Transport.TSocket(address, port);
			transport.Open();
			try
			{
				var protocol = new Thrift.Protocol.TBinaryProtocol(transport);

				var worker = new Worker.Client(protocol);

				_workers.Add(podID, worker);

				return worker;
			}
			catch
			{
				transport.Close();
				throw;
			}
		}

		private Worker.Client GetWorker(int columnID)
		{
			PodMap map;
			if (!_columnWorkers.TryGetValue(columnID, out map))
			{
				// TODO: Update the hive state before erroring.
				throw new Exception(String.Format("No worker is currently available for column ID ({0}).", columnID));
			}
			map.Next = (map.Next + 1) % map.Pods.Count;
			return EnsureWorker(map.Pods[map.Next]);
		}

		public void Dispose() 
		{ 
			var errors = new List<Exception>();
			while (_services.Count > 0)
			{
				var service = _services.First();
				try
				{
					service.Value.OutputProtocol.Transport.Close();
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
			while (_workers.Count > 0)
			{
				var worker = _workers.First();
				try
				{
					worker.Value.OutputProtocol.Transport.Close();
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

		public Transaction Begin(bool readIsolation, bool writeIsolation)
		{
			return new Transaction(this, readIsolation, writeIsolation);
		}

		//Hit the thrift API to build range.
		public DataSet GetRange(int[] columnIds, Range range, int limit, object startId = null)
		{
			// Create the range query
			var query = CreateQuery(range, limit, startId);

			// Determine the worker to use
			var worker = GetWorker(range.ColumnID);

			// Make the range request
            var rangeResults = worker.query(new Dictionary<int, Query> { { range.ColumnID, query }});
			var rangeResult = rangeResults[range.ColumnID].Answer.RangeValues[0];

			// Create the row ID query
			Query rowIdQuery = GetRowsQuery(rangeResult);

			// Construct the column row ID queries grouped by worker
			Dictionary<Worker.Client, Dictionary<int, Query>> queriesByWorker = new Dictionary<Worker.Client, Dictionary<int, Query>>();
			for (int i = 0; i < columnIds.Length; i++)
			{
				if (columnIds[i] != range.ColumnID)
				{
					worker = GetWorker(columnIds[i]);
					Dictionary<int, Query> queries;
					if (!queriesByWorker.TryGetValue(worker, out queries))
					{
						queries = new Dictionary<int, Query>();
						queriesByWorker.Add(worker, queries);
					}
					queries.Add(columnIds[i], rowIdQuery);
				}
			}

			// Make the query request against the workers and wait for all results to arrive
			var tasks = new List<Task<Dictionary<int, ReadResult>>>(queriesByWorker.Count);
			foreach (var workerQueries in queriesByWorker)
				tasks.Add(Task.Factory.StartNew(()=> { return workerQueries.Key.query(workerQueries.Value); }));
			
			// Combine all results into a single dictionary by column
			var resultsByColumn = new Dictionary<int, ReadResult>(columnIds.Length);
			foreach (var task in tasks)
				foreach (var result in task.Result)
					resultsByColumn.Add(result.Key, result.Value);
			
			return ResultToDataSet(columnIds, rowIdQuery.RowIDs, range.ColumnID, rangeResult, resultsByColumn);
		}

		private DataSet ResultToDataSet(int[] columnIds, List<byte[]> rowIDs, int rangeColumnID, RangeResult rangeResult, Dictionary<int, ReadResult> rowResults)
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
//            Dictionary<int, ColumnWrites> writes = new Dictionary<int, ColumnWrites>();
//            byte[] rowIdb = Fastore.Client.Encoder.Encode(rowId);

//            for (int i = 0; i < columnIds.Length; i++)
//            {               
//                Include inc = new Fastore.Include();
//                inc.RowID = rowIdb;
//                inc.Value = Fastore.Client.Encoder.Encode(row[i]);

//                ColumnWrites wt = new ColumnWrites();
//                wt.Includes = new List<Fastore.Include>();
//                wt.Includes.Add(inc);
//                writes.Add(columnIds[i], wt);
//            }

//            Service.apply(_defaultId, writes);

//            if (columnIds[0] == 0)
//                RefreshSchema();
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
            //Actually, we only need the ID and Type to bootstrap properly.
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
