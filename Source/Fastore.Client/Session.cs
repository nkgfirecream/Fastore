using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    public class Session : IDataAccess
    {
        private Client _client;
        public Session(Client client)
        {
            _client = client;
        }
        public void Dispose() {}

        public Transaction Begin(bool readIsolation, bool writeIsolation)
        {
            throw new NotImplementedException();
        }

        //Hit the thrift API to build range.
        public DataSet GetRange(int[] columnIds, Order[] orders, Range[] ranges, object startId = null)
        {
            return _client.GetRange(columnIds, orders, ranges, startId);
        }

        public void Include(int[] columnIds, object rowId, object[] row)
        {
            _client.Include(columnIds, rowId, row);
        }

        public void Exclude(int[] columnIds, object rowId)
        {
            _client.Exclude(columnIds, rowId);
        }

        public Statistics GetStatistics(int columnId)
        {
            throw new NotImplementedException();
        }
    }
}
