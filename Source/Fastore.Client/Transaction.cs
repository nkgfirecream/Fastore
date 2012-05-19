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
            throw new NotImplementedException();
        }

        public void Include(int[] columnIds, object rowId, object[] row)
        {
            throw new NotImplementedException();
        }

        public void Exclude(int[] columnIds, object rowId)
        {
            throw new NotImplementedException();
        }

		public Statistic[] GetStatistics(int[] columnIds)
        {
            throw new NotImplementedException();
        }
    }
}
