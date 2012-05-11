using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Alphora.Fastore;

namespace ServiceTest
{
    class Program
    {
        static void Main(string[] args)
        {
            Alphora.Fastore.Service.Client client;

            Thrift.Transport.TSocket sock = new Thrift.Transport.TSocket("localhost", 9090);
            sock.Open();

            Thrift.Protocol.TBinaryProtocol prot = new Thrift.Protocol.TBinaryProtocol(sock, false, false);

            client = new Service.Client(prot);

            client.GetTopology();
        }
    }
}
