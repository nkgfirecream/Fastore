using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Linq.Expressions;
using System.Threading.Tasks;

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
		private List<WorkerQueue> _workers = new List<WorkerQueue>();
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
				var storeType = typeof(ColumnSet<>).MakeGenericType(def.Type);
				store = Activator.CreateInstance(storeType, new object[] { null });
			}
			else
			{
				var storeType = typeof(ColumnHash<>).MakeGenericType(def.Type);
				store = Activator.CreateInstance(storeType, new object[] { null });
			}

			_stores.Insert(index, store);

			_defs.Insert(index, def);

			_workers.Insert(index, new WorkerQueue());
		}

		private void RecompileLogic()
		{
			RecompileInsertHandlers();
			RecompileSelectHandlers();
		}

		private delegate void InsertHandler(Table table, object value, long id);
		private InsertHandler[] _insertHandlers;

		private void RecompileInsertHandlers()
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
			// Pseudo code:  { _stores[column].Insert(value, id); }
			var storeType = _stores[column].GetType();
			return
				Expression.Call
				(
					BuildStoreGetter(column, table, storeType), 
					storeType.GetMethod("Insert"), 
					Expression.Convert(value, _defs[column].Type), 
					id
				);
		}

		
        public void Insert(long id, int[] projection, params object[] values)
        {
			// TODO: Ensure that the projection doesn't contain duplicates - this doesn't makes sense for insertion

			projection = EnsureProjection(projection); 
			
			for (int i = 0; i < projection.Length; i++)
			{
				var colIndex = projection[i];
				var value = values[i];
				if (value != null)
				{
					_workers[i].Queue(() => _insertHandlers[colIndex](this, value, id));

					// TODO: roll-back other columns if insert fails for any column
				}
			}            
        }

		// Temporary routine, wouldn't need because selection would replace this.
		public void WaitForWorkers()
		{
			WaitHandle.WaitAll(_workers.Select<WorkerQueue, WaitHandle>(q => q.Handle).ToArray());
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

		private delegate object SelectHandler(Table table, long id);
		private SelectHandler[] _selectHandlers;

		private void RecompileSelectHandlers()
		{
			_selectHandlers = new SelectHandler[_defs.Count];
			for (int i = 0; i < _defs.Count; i++)
			{
				_selectHandlers[i] = BuildSelectLambda(i).Compile();
			}
		}

		private Expression<SelectHandler> BuildSelectLambda(int column)
		{
			var table = Expression.Parameter(typeof(Table), "table");
			var id = Expression.Parameter(typeof(long), "id");
			return
				Expression.Lambda<SelectHandler>
				(
					BuildSelectBody(column, table, id),
					table,
					id
				);
		}

		private Expression BuildSelectBody(int column, ParameterExpression table, ParameterExpression id)
		{
			// Pseudo code:  { dynamic ov = _stores[column].GetValue(id); return (object)(ov.HasValue ? ov.Value : null); }
			var storeType = _stores[column].GetType();
			var columnType = _defs[column].Type;
			var optionalValue = Expression.Variable(typeof(Optional<>).MakeGenericType(columnType), "optionalValue");
			return
				Expression.Block
				(
					new[] { optionalValue },
					Expression.Assign
					(
						optionalValue,
						Expression.Call
						(
							BuildStoreGetter(column, table, storeType),
							storeType.GetMethod("GetValue"),
							id
						)
					),
					Expression.Condition
					(
						Expression.Property(optionalValue, "HasValue"),
						Expression.Convert
						(
							Expression.Property(optionalValue, "Value"),
							typeof(object)
						),
						Expression.Constant(null, typeof(object))
					)
				);
		}

		private static UnaryExpression BuildStoreGetter(int column, ParameterExpression table, Type storeType)
		{
			return 
				Expression.Convert
				(
					Expression.Property
					(
						Expression.Field(table, typeof(Table).GetField("_stores", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance)),
						"Item",
						Expression.Constant(column)
					),
					storeType
				);
		}

		public object[] Select(long id, int[] projection)
        {
			var values = new object[projection.Length];
            for (int i = 0; i < projection.Length; i++)
            {
				values[i] = _selectHandlers[projection[i]](this, id);
			}
			return values;
        }

		public IEnumerable<KeyValuePair<long, object[]>> Select(int column, object start, object end, bool isForward, int[] projection)
		{
			projection = EnsureProjection(projection);

			foreach (var entry in GetRows(column, start, end, isForward))
			{
				long id = entry.Key;
				var values = new object[projection.Length];
				for (int i = 0; i < projection.Length; i++)
				{
					var colIndex = projection[i];
					if (colIndex == column)
						values[i] = entry.Value;
					else
						values[i] = _selectHandlers[colIndex](this, id);
				}
				yield return new KeyValuePair<long, object[]>(id, values);
			}
		}

		private dynamic GetRows(int column, object start, object end, bool isForward)
		{
			var store = _stores[column];
			var storeType = store.GetType();
			var dataType = storeType.GetGenericArguments()[0];
			var optionalType = typeof(Optional<>).MakeGenericType(dataType);
			var nullOptional = optionalType.GetField("Null", System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.Public).GetValue(null);
			var optionalStart = start == null ? nullOptional : Activator.CreateInstance(optionalType, start);
			var optionalEnd = end == null ? nullOptional : Activator.CreateInstance(optionalType, end);
			return storeType.GetMethod("GetRows", new Type[] { typeof(bool), optionalType, optionalType }).Invoke(store, new object[] { isForward, optionalStart, optionalEnd });
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
