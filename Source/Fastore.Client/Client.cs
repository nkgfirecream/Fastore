using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Alphora.Fastore;
using Thrift;

namespace Alphora.Fastore.Client
{
    public class Client
    {
        Alphora.Fastore.Service.Client _client;
        
        public Client(string url, int port)
        {
            var transport = new Thrift.Transport.TSocket(url, port);
            transport.Open();

            var protocol = new Thrift.Protocol.TBinaryProtocol(transport);

            _client = new Service.Client(protocol);
        }

        //These will change as the various other objects are fleshed out.
        //Right now, they just push everything down to the client, but soon they
        //will need to locally buffer reads and writes, etc.

        //Basically, this class will only serve as a wrapper for talking to hosts. All the other processing will already be done beforehand.
        //(We will probably in fact have several clients connected to various hosts)
        internal void CreateColumn(ColumnDef def) 
        {
            var topo = _client.GetTopology();
            topo.Topology.Repositories.Add(new Repository() { ColumnID = def.ColumnID, HostID = 0 });

            _client.PrepareTopology(null, topo.Topology);
        }

        internal void DeleteColumn(int columnId)
        {
            var topo = _client.GetTopology();
        }

        internal bool ExistsColumn(int columnId)
        {
            throw new NotImplementedException();        
        }

        internal void Include(int[] columnIds, object rowId, object[] row)
        {
            //For now we are only going to use apply and get...
            throw new NotImplementedException();
        }

        internal void Exclude(int[] columnIds, object rowId)
        {
            throw new NotImplementedException();
        }

        internal DataSet GetRange(int[] columnIds, Order[] orders, Range[] ranges, object startId = null)
        {
            throw new NotImplementedException();
        }
    }
}
