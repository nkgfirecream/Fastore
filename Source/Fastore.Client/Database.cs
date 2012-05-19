using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
	// TODO: concurrency
    public class Database : IDataAccess, IDisposable
    {
		internal Alphora.Fastore.Service.Client Host { get; set; }
		private Thrift.Transport.TTransport _transport;
		private Schema _schema;

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

		public Statistic[] GetStatistics(int[] columnIds)
		{
			return 
			(
				from s in Host.GetStatistics(columnIds.ToList()) 
					select new Statistic { Total = s.Total, Unique = s.Unique }
			).ToArray();
		}

		public Schema GetSchema()
		{
			if (_schema == null)
				_schema = LoadSchema();
			return new Schema(_schema);
		}

		private Schema LoadSchema()
		{
			var schema = new Schema();
			var columns =
				GetRange
				(
					new[] { 0, 1, 2, 3, 4 },
					new[] { new Order { ColumnID = 0, Ascending = true } },
					new[] { new Range { ColumnID = 0 } }
				);
			foreach (var column in columns)
			{
				var def =
					new ColumnDef
					{
						ColumnID = (int)column[0],
						Name = (string)column[1],
						Type = (string)column[2],
						IDType = (string)column[3],
						IsUnique = (bool)column[4]
					};
				schema.Add(def.ColumnID, def);
			}
			return schema;
		}



		public void RefreshSchema()
		{
			_schema = null;
		}

		//These will change as the various other objects are fleshed out.
		//Right now, they just push everything down to the client, but soon they
		//will need to locally buffer reads and writes, etc.

		//Basically, this class will only serve as a wrapper for talking to hosts. All the other processing will already be done beforehand.
		//(We will probably in fact have several clients connected to various hosts)
		public void CreateColumn(ColumnDef def)
		{
			//var topo = Host.GetTopology();
			//topo.Topology.Repositories.Add(new Repository() { ColumnID = def.ColumnID, HostID = 0 });

			//Host.PrepareTopology(null, topo.Topology);
			Include(new[] { 0, 1, 2, 3, 4 }, def.ColumnID, new object[] { def.ColumnID, def.Name, def.Type, def.IDType, def.IsUnique });
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
