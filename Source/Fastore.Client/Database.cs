﻿using System;
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
        private TransactionID _defaultId = new TransactionID() { Key = 0, Revision = 0 };

		public Database(Alphora.Fastore.Service.Client host, Thrift.Transport.TTransport transport)
        {
            Host = host;
			_transport = transport;
            
            //GetRange to determine the schema requires a few assumptions
            //about the schema in order to be able to decode it properly.
            //(Like, the types of the schema columns)
            BootStrapSchema();           
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
            if (orders.Length > 1)
                throw new NotSupportedException("Multiple orders not supported");

            if (ranges.Length > 1)
                throw new NotSupportedException("Multiple ranges not supported");

            if (orders.Length == 1 && ranges.Length == 1 && orders[0].ColumnID != ranges[0].ColumnID)
                throw new InvalidOperationException("Base order and range must have same column id");


            //First, pull in the correct rowIds based on the range and order.
            RangeRequest rangeRequest = new RangeRequest();
            rangeRequest.Ascending = orders.Length > 0 ? orders[0].Ascending : true;
            int rangeId = ranges.Length > 0 ? ranges[0].ColumnID : orders.Length > 0 ? orders[0].ColumnID : columnIds[0];

            if (ranges.Length > 0)
            {
                var clientrange = ranges[0];

                //Some things to think about...
                //Start always represents the Lowest value in the range requested (in whatever order the column is stored in),
                //not the value you want actually want to start with (counterintuitive in the reverse order case).
                //Since the start/end swap happens on the server, the startId needs to be associated with whatever bound
                //actually is the start value. 
                if (clientrange.Start.HasValue)
                {
                    Fastore.RangeBound bound = new Fastore.RangeBound();
                    bound.Inclusive = clientrange.Start.Value.Inclusive;
                    bound.Value = Fastore.Client.Encoder.Encode(clientrange.Start.Value.Bound);

                    if (rangeRequest.Ascending && startId != null)
                        bound.RowID = Fastore.Client.Encoder.Encode(startId);

                    rangeRequest.Start = bound;
                }

                if (clientrange.End.HasValue)
                {
                    Fastore.RangeBound bound = new Fastore.RangeBound();
                    bound.Inclusive = clientrange.End.Value.Inclusive;
                    bound.Value = Fastore.Client.Encoder.Encode(clientrange.End.Value.Bound);

                    if (!rangeRequest.Ascending && startId != null)
                        bound.RowID = Fastore.Client.Encoder.Encode(startId);

                    rangeRequest.End = bound;
                }

                rangeRequest.Limit = ranges[0].Limit;
            }

			var rangeRequests = new List<RangeRequest>();
			rangeRequests.Add(rangeRequest);
			
			Query rangeQuery = new Query();
            rangeQuery.Ranges = rangeRequests;

            Dictionary<int, Query> rangeQueries = new Dictionary<int, Query>();
            rangeQueries.Add(rangeId, rangeQuery);

            var rangeQueriesResult = Host.Query(rangeQueries);
            
            //We only sent one query, so we only care about one result...
            var rangeResult = rangeQueriesResult.Answers[rangeId].RangeValues[0];         
           
            //Put all the rowIds in a list, so we can either send them off again to fill the dataset.
            //(I am filling the order column twice in the case that it is part of the selection,
            //but I can add code to be smart enough to handle that in the future)
            List<byte[]> rowIds = new List<byte[]>();
            foreach (var valuerow in rangeResult.ValueRowsList)
            {
                foreach (var rowid in valuerow.RowIDs)
                {
                    rowIds.Add(rowid);              
                }
            }           

            //Create dataset to store result in....
            DataSet ds = new DataSet(rowIds.Count, columnIds.Length);
            ds.EndOfRange = rangeResult.EndOfRange;

            Query rowIdQuery = new Query() { RowIDs = rowIds };

            for (int i = 0; i < columnIds.Length; i++)
            {
                //I'm doing this one at a time because it will eventually be parallelized. Some other logic will need to group columns by hosts and send them off
                //in batches.
                Dictionary<int, Query> queries = new Dictionary<int,Query>();
                queries.Add(columnIds[i], rowIdQuery);
                var idResult = Host.Query(queries);

                var values = idResult.Answers[columnIds[i]].RowIDValues;

                for (int j = 0; j < values.Count; j++)
                {
                    ds[j][i] = Fastore.Client.Encoder.Decode(values[j], _schema[columnIds[i]].Type);
                }
            }

            return ds;
		}

        public void Include(int[] columnIds, object rowId, object[] row)
		{
            Dictionary<int, ColumnWrites> writes = new Dictionary<int, ColumnWrites>();
            byte[] rowIdb = Fastore.Client.Encoder.Encode(rowId);

            for (int i = 0; i < columnIds.Length; i++)
            {               
                Include inc = new Fastore.Include();
                inc.RowID = rowIdb;
                inc.Value = Fastore.Client.Encoder.Encode(row[i]);

                ColumnWrites wt = new ColumnWrites();
                wt.Includes.Add(inc);
                writes.Add(columnIds[i], wt);
            }

            Host.Apply(_defaultId, writes);

            if (columnIds[0] == 0)
                RefreshSchema();
		}

		public void Exclude(int[] columnIds, object rowId)
		{
            Dictionary<int, ColumnWrites> writes = new Dictionary<int, ColumnWrites>();
            byte[] rowIdb = Fastore.Client.Encoder.Encode(rowId);
            
            Exclude ex = new Fastore.Exclude();
            ex.RowID = rowIdb;

            ColumnWrites wt = new ColumnWrites();
            wt.Excludes.Add(ex);

            for (int i = 0; i < columnIds.Length; i++)
            {
                writes.Add(columnIds[i], wt);
            }

            Host.Apply(_defaultId, writes);

            if (columnIds[0] == 0)
                RefreshSchema();
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
					new[] { new Range { ColumnID = 0, Limit = int.MaxValue } }
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
            _schema = LoadSchema();
		}

        private void BootStrapSchema()
        {
            //Actually, we only need the ID and Type to bootstrap properly.
            ColumnDef id = new ColumnDef();
            ColumnDef name = new ColumnDef();
            ColumnDef vt = new ColumnDef();
            ColumnDef idt = new ColumnDef();
            ColumnDef unique = new ColumnDef();

            id.ColumnID = 0;
            id.Name = "ID";
            id.Type = "Int";
            id.IDType = "Int";
            id.IsUnique = true;

            name.ColumnID = 1;
            name.Name = "Name";
            name.Type = "String";
            name.IDType = "Int";
            name.IsUnique = false;

            vt.ColumnID = 2;
            vt.Name = "ValueType";
            vt.Type = "String";
            vt.IDType = "Int";
            vt.IsUnique = false;

            idt.ColumnID = 3;
            idt.Name = "RowIDType";
            idt.Type = "String";
            idt.IDType = "Int";
            idt.IsUnique = false;

            unique.ColumnID = 4;
            unique.Name = "IsUnique";
            unique.Type = "Bool";
            unique.IDType = "Int";
            unique.IsUnique = false;

            _schema = new Schema();

            _schema.Add(0, id);
            _schema.Add(1, name);
            _schema.Add(2, vt);
            _schema.Add(3, idt);
            _schema.Add(4, unique);

            //Boot strapping is done, pull in real schema
            RefreshSchema();
        }
	}
}