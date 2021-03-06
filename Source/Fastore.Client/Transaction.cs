﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Alphora.Fastore.Client
{
    /// <Remarks>
    ///   If a transaction is disposed and hasn't been committed or rolled back then it is automatically rolled back.
    ///   Once a transaction has been committed or rolled back then it is in a new transaction state again and can be used as if it were new again.
    /// </Remarks>
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
			// TODO: gen ID  - perhaps defer until needed; first read-write would obtain revision
			_transactionId = new TransactionID { Key = 0, Revision = 0 };	
			_log = new Dictionary<int, LogColumn>();
		}

        public void Dispose()
        {
            if (!_completed)
				Rollback();
        }

        public void Commit(bool flush = false)
        {
			var writes = GatherWrites();

            Database.Apply(writes, flush);

            _log.Clear();
            _completed = true;
        }

		private Dictionary<int, ColumnWrites> GatherWrites()
		{
			Dictionary<int, ColumnWrites> writesPerColumn = new Dictionary<int, ColumnWrites>();

			// Gather changes for each column
			foreach (var entry in _log)
			{
				ColumnWrites writes = null;

				// Process Includes
				foreach (var include in entry.Value.Includes)
				{
					if (writes == null)
					{
						writes = new ColumnWrites();
						writes.Includes = new List<Fastore.Include>();
					}
					Include inc = new Fastore.Include();
					inc.RowID = Fastore.Client.Encoder.Encode(include.Key);
					inc.Value = Fastore.Client.Encoder.Encode(include.Value);
					writes.Includes.Add(inc);
				}

				// Process Excludes
				foreach (var exclude in entry.Value.Excludes)
				{
					if (writes == null)
						writes = new ColumnWrites();
					if (writes.Excludes == null)
						writes.Excludes = new List<Fastore.Exclude>();
					Exclude ex = new Fastore.Exclude { RowID = Fastore.Client.Encoder.Encode(exclude) };
					writes.Excludes.Add(ex);
				}

				if (writes != null)
					writesPerColumn.Add(entry.Key, writes);
			}

			return writesPerColumn;
		}

		public void Rollback()
		{
			_log.Clear();
			_completed = true;
		}

        public RangeSet GetRange(int[] columnIds, Range range, int limit, object startId = null)
        {
            // Get the raw results
            var raw = Database.GetRange(columnIds, range, limit, startId);

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
            var resultRows = new List<DataSet.DataSetRow>();
            foreach (var row in raw.Data)
            {
				var newRow = new DataSet.DataSetRow { ID = row.ID, Values = row.Values };
                var allNull = true;
                for (int i = 0; i < row.Values.Length; i++)
                {
                    LogColumn col = changeMap[i];
                    if (col != null)
                    {
                        if (col.Excludes.Contains(row.Values[i]))
                            newRow.Values[i] = null;
                        else
                        {
                            allNull = false;
                            newRow.Values[i] = row.Values[i];
                        }
                    }
                    else
                        newRow.Values[i] = row.Values[i];
                }
                if (!allNull)
                    resultRows.Add(newRow);
            }

            // TODO: handle includes - probably need to keep a shadow of column buffers to do the merging with
            // Cases for a given range
            // 1 - Local exclusions within range
            // 2 - Local inclusions within range
            // 3 - Local updates to a row that:
            //      A - Move a row into the range
            //      B - Move a row out of the range

            //Update case: a change to a value in a row
            //This could cause the rows to get out of order if you've updated a row in the range.
            //foreach (var row in resultRows)
            //{
            //    for (int i = 0; i < row.Values.Length; i++)
            //    {
            //        LogColumn col = changeMap[i];
            //        if (col != null)
            //        {
            //            if (col.Includes.ContainsKey(row.ID))
            //            {
            //                row.Values[i] = col.Includes[row.ID];
            //            }
            //        }
            //    }
            //}

            //Insert case: Include new rows that are not present in our get range.

            // Turn the rows back into a dataset
            var result = new DataSet(resultRows.Count, columnIds.Length);
            for (var i = 0; i < result.Count; i++)
                result[i] = resultRows[i];
            raw.Data = result;

			return raw;
        }

		public DataSet GetValues(int[] columnIds, object[] rowIds)
		{
			// TODO: Filter/augment data for the transaction
			return Database.GetValues(columnIds, rowIds);
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

		public Dictionary<int, TimeSpan> Ping()
		{
			return Database.Ping();
		}
	}
}
