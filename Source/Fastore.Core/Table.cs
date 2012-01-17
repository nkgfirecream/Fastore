using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Linq.Expressions;

namespace Fastore.Core
{
    public class Table
    {
		public Table(params ColumnDef[] defs)
		{
			for (var i = 0; i < defs.Length; i++)
				InternalAddColumn(i, defs[i]);
			RecompileLogic();
		}

		private List<ColumnDef> _defs = new List<ColumnDef>();
		private List<object> _stores = new List<object>();
		private long _nextID;

		public void AddColumn(int index, ColumnDef def)
		{
			InternalAddColumn(index, def);
			RecompileLogic();
		}

		private void InternalAddColumn(int index, ColumnDef def)
		{
			// TODO: allow for a custom value sorter
			// TODO: wire up row sorter to next column
			object store;
			if (def.IsUnique)
			{
				var storeType = typeof(ColumnSet<>).MakeGenericType(new Type[] { def.Type });
				store = Activator.CreateInstance(storeType, new object[] { null });
			}
			else
			{
				var storeType = typeof(ColumnBag<>).MakeGenericType(new Type[] { def.Type });
				store = Activator.CreateInstance(storeType, new object[] { null, null });
			}

			_stores.Insert(index, store);

			_defs.Insert(index, def);
		}

		private void RecompileLogic()
		{
			_insertHandlers = new InsertHandler[_defs.Count];
			for (int i = 0; i < _defs.Count; i++)
			{
				_insertHandlers[i] = BuildInsertLambda(i).Compile();
			}
		}

		private Expression<InsertHandler> BuildInsertLambda(int column)
		{
			var table = Expression.Parameter(typeof(Table), "table");
			var value = Expression.Parameter(typeof(object), "value");
			var id = Expression.Parameter(typeof(long), "id");
			return
				Expression.Lambda<InsertHandler>
				(
					BuildInsertBody(column, table, value, id),
					table,
					value,
					id
				);
		}

		private Expression BuildInsertBody(int column, ParameterExpression table, ParameterExpression value, ParameterExpression id)
		{
			var storeType = _stores[column].GetType();
			return
				Expression.Call
				(
					Expression.Convert
					(
						Expression.Property
						(
							Expression.Field(table, typeof(Table).GetField("_stores", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance)), 
							"Item", 
							Expression.Constant(column)
						),
						storeType
					), 
					storeType.GetMethod("Insert"), 
					Expression.Convert(value, _defs[column].Type), 
					id
				);
		}

		private delegate void InsertHandler(Table table, object value, long id);
		private InsertHandler[] _insertHandlers;

        public void Insert(long id, int[] projection, params object[] values)
        {
			projection = EnsureProjection(projection);
			
			for (int i = 0; i < projection.Length; i++)
			{
				var colIndex = projection[i];
				var value = values[i];
				if (value != null)
				{
					_insertHandlers[colIndex](this, value, id);

					// TODO: roll-back other columns if insert fails for any column
				}
			}            
        }

		public long Insert(int[] projection, object[] values)
		{
			var id = Interlocked.Increment(ref _nextID);
			Insert(id, projection, values);
			return id;
		}

		public long Insert(params object[] values)
		{
			return Insert((int[])null, values);
		}

        public object[] Select(long id, int[] projection)
        {
			var values = new object[projection.Length];
            for (int i = 0; i < projection.Length; i++)
            {
				dynamic store = _stores[projection[i]];
				values[i] = store.GetValue(id);
			}
			return values;
        }

		public IEnumerable<KeyValuePair<long, object[]>> Select(int column, bool isForward, int[] projection)
		{
			dynamic store = _stores[column];
			
			projection = EnsureProjection(projection);

			foreach (var entry in store.GetRows(isForward))
			{
				long id = entry.Key;
				var values = new object[projection.Length];
				for (int i = 0; i < projection.Length; i++)
				{
					var colIndex = projection[i];
					if (colIndex == column)
						values[i] = entry.Key;
					else
					{
						dynamic other = _stores[colIndex];
						values[i] = other.GetValue(id);
					}
				}
				yield return new KeyValuePair<long, object[]>(id, values);
			}
		}

		private int[] EnsureProjection(int[] projection)
		{
			// TODO: validate that all indices are in-bounds

			// Create a default projection of all columns if not specified
			if (projection == null)
			{
				projection = new int[_defs.Count];
				for (var i = 0; i < _defs.Count; i++)
					projection[i] = i;
			}
			return projection;
		}
    }
}
