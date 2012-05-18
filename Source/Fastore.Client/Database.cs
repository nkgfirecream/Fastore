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
    }
}
