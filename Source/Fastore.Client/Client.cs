using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Alphora.Fastore;
using Thrift;

namespace Alphora.Fastore.Client
{
    public static class Client
    {
		public static Database Connect(string address, int port)
		{
			return new Database(address, port);
		}
    }
}
