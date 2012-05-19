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
		public static Database Connect(string url, int port)
		{
            var transport = new Thrift.Transport.TSocket(url, port);
            transport.Open();

            var protocol = new Thrift.Protocol.TBinaryProtocol(transport);

            return new Database(new Service.Client(protocol), transport);
		}
    }
}
