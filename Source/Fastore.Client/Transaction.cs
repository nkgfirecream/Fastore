using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    public class Transaction : IDataAccess, IDisposable
    {
		public Database Database { get; private set; }
		public bool ReadIsolation { get; private set; }
		public bool WriteIsolation { get; private set; }

		private bool _completed;
		private TransactionID _transactionId;

		// Log entries - by column ID then by row ID - null value means exclude
		private Dictionary<int, LogColumn> _log;

		internal Transaction(Database database, bool readIsolation, bool writeIsolation)
		{
			Database = database;
			ReadIsolation = readIsolation;
			WriteIsolation = writeIsolation;
			_transactionId = new TransactionID();	// TODO: gen ID  - perhaps defer until needed; first read-write would obtain revision
			_log = new Dictionary<int, LogColumn>();
		}

        public void Dispose()
        {
            if (!_completed)
				Rollback();
        }

        public void Commit()
        {
            Dictionary<int, ColumnWrites> writes = new Dictionary<int, ColumnWrites>();

			// Gather changes for each column
			foreach (var entry in _log)
			{
				ColumnWrites wt = null;
				
				// Process Includes
				foreach (var include in entry.Value.Includes)
				{
					if (wt == null)
					{
						wt = new ColumnWrites();
						wt.Includes = new List<Fastore.Include>();
					}
					Include inc = new Fastore.Include();
					inc.RowID = Fastore.Client.Encoder.Encode(include.Key);
					inc.Value = Fastore.Client.Encoder.Encode(include.Value);
					wt.Includes.Add(inc);
				}

				// Process Excludes
				foreach (var exclude in entry.Value.Excludes)
				{
					if (wt == null)
						wt = new ColumnWrites();
					if (wt.Excludes == null)
                        wt.Excludes = new List<Fastore.Exclude>();
					Exclude ex = new Fastore.Exclude { RowID = Fastore.Client.Encoder.Encode(exclude) };
					wt.Excludes.Add(ex);
				}

				if (wt != null)
					writes.Add(entry.Key, wt);
            }

            Database.Host.Apply(_transactionId, writes);

			_log.Clear();
			_completed = true;
        }

		public void Rollback()
		{
			_log.Clear();
			_completed = true;
		}

        public DataSet GetRange(int[] columnIds, Order[] orders, Range[] ranges, object startId = null)
        {
			// Get the raw results
			var raw = Database.GetRange(columnIds, orders, ranges, startId);
		
			// Find a per-column change map for each column in the selection
			var changeMap = new LogColumn[columnIds.Length];
			var anyMapped = false;
			for (int x = 0; x < columnIds.Length; x++)
			{
				LogColumn col;
				if (_log.TryGetValue(columnIds[x], out col))
				{
					anyMapped = true;
					changeMap[x] = col;
				}
				else
					changeMap[x] = null;
			}

			// Return raw if no changes to the requested columns
			if (!anyMapped)
				return raw;

			// Process excludes from results
			var resultRows = new List<object[]>();
			foreach (var row in raw)
			{
				var newRow = new object[row.Length];
				var allNull = true;
				for (int i = 0; i < row.Length - 1; i++)
				{
					LogColumn col = changeMap[i];
					if (col != null)
					{
						if (col.Excludes.Contains(row[i]))
							newRow[i] = null;
						else
						{
							allNull = false;
							newRow[i] = row[i];
						}
					}
					else
						newRow[i] = row[i];
				}
				if (!allNull)
					resultRows.Add(newRow);
			}

			// TODO: handle includes - probably need to keep a shadow of column buffers to do the merging with

			// Turn the rows back into a dataset
			var result = new DataSet(resultRows.Count, columnIds.Length + 1);
			for (var i = 0; i < result.Count; i++)
				result[i] = resultRows[i];
			return result;
        }

        public void Include(int[] columnIds, object rowId, object[] row)
        {
			for (var i = 0; i < columnIds.Length; i++)
				EnsureColumnLog(columnIds[i]).Includes[rowId] = row[i];
        }

        public void Exclude(int[] columnIds, object rowId)
        {
			for (var i = 0; i < columnIds.Length; i++)
				EnsureColumnLog(columnIds[i]).Excludes.Add(rowId);
		}

		public Statistic[] GetStatistics(int[] columnIds)
        {
            return Database.GetStatistics(columnIds);
        }

		private LogColumn EnsureColumnLog(int columnId)
		{
			LogColumn col;
			if (!_log.TryGetValue(columnId, out col))
			{
				col = new LogColumn();
				_log.Add(columnId, col);
			}
			return col;
		}

		private class LogColumn
		{
			public Dictionary<object, object> Includes = new Dictionary<object, object>();
			public HashSet<object> Excludes = new HashSet<object>();
		}
	}
}
