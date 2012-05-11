using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    public class Database
    {
        private Client _client;
        public Database(Client host)
        {
            _client = host;
        }

        public Session Start()
        {
            return new Session(_client);
        }

        public void CreateColumn(ColumnDef def)
        {
            _client.CreateColumn(def);
        }

        public void DeleteColumn(int columnId)
        {
            _client.DeleteColumn(columnId);
        }

        public bool ExistsColumn(int columnId)
        {
            return _client.ExistsColumn(columnId);
        }
    }
}
