using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    public class Database : IDataAccess, IDisposable
    {
		internal Alphora.Fastore.Service.Client Host { get; set; }
		private Thrift.Transport.TTransport _transport;

		public Database(Alphora.Fastore.Service.Client host, Thrift.Transport.TTransport transport)
        {
            Host = host;
			_transport = transport;
        }

		public void Dispose() 
		{ 
			if (_transport != null)
			{
				try
				{
					_transport.Close();
				}
				finally
				{
					_transport = null;
				}
			}
		}

		public Transaction Begin(bool readIsolation, bool writeIsolation)
		{
			return new Transaction(this, readIsolation, writeIsolation);
		}

		//Hit the thrift API to build range.
		public DataSet GetRange(int[] columnIds, Order[] orders, Range[] ranges, object startId = null)
		{
			//return Host.GetRange(columnIds, orders, ranges, startId);
			throw new NotImplementedException();
		}

		public void Include(int[] columnIds, object rowId, object[] row)
		{
			//Host.Include(columnIds, rowId, row);
			throw new NotImplementedException();
		}

		public void Exclude(int[] columnIds, object rowId)
		{
			//Host.Exclude(columnIds, rowId);
			throw new NotImplementedException();
		}

		public Statistics GetStatistics(int columnId)
		{
			throw new NotImplementedException();
		}

		//These will change as the various other objects are fleshed out.
		//Right now, they just push everything down to the client, but soon they
		//will need to locally buffer reads and writes, etc.

		//Basically, this class will only serve as a wrapper for talking to hosts. All the other processing will already be done beforehand.
		//(We will probably in fact have several clients connected to various hosts)
		internal void CreateColumn(ColumnDef def)
		{
			var topo = Host.GetTopology();
			topo.Topology.Repositories.Add(new Repository() { ColumnID = def.ColumnID, HostID = 0 });

			Host.PrepareTopology(null, topo.Topology);
		}

		internal void DeleteColumn(int columnId)
		{
			var topo = Host.GetTopology();
		}

		internal bool ExistsColumn(int columnId)
		{
			throw new NotImplementedException();
		}
	}
}
