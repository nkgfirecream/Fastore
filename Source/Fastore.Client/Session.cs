using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    public class Session : IDataAccess
    {
        public Session() {}
        public void Dispose() {}

        public Transaction Begin(bool readIsolation, bool writeIsolation)
        {
            throw new NotImplementedException();
        }

        //Hit the thrift API to build range.
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

        public Statistics GetStatistics(int columnId)
        {
            throw new NotImplementedException();
        }
    }
}
