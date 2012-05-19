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

		internal Transaction(Database database, bool readIsolation, bool writeIsolation)
		{
			Database = database;
			ReadIsolation = readIsolation;
			WriteIsolation = writeIsolation;
		}

        public void Dispose()
        {
            if (!_completed)
				Rollback();
        }

        public void Commit()
        {
			_completed = true;
			throw new NotImplementedException();
        }

		public void Rollback()
		{
			_completed = true;
			throw new NotImplementedException();
		}

        public DataSet GetRange(int[] columnIds, Order[] orders, Range[] ranges, object startId = null)
        {
            return Database.GetRange(columnIds, orders, ranges, startId);
        }

        public void Include(int[] columnIds, object rowId, object[] row)
        {
            Database.Include(columnIds, rowId, row);
        }

        public void Exclude(int[] columnIds, object rowId)
        {
            Database.Exclude(columnIds, rowId);
        }

		public Statistic[] GetStatistics(int[] columnIds)
        {
            return Database.GetStatistics(columnIds);
        }
    }
}
