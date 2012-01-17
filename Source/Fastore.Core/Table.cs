using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace Fastore.Core
{
    public class Table
    {
		private List<ColumnDef> _defs = new List<ColumnDef>();
		private List<object> _stores = new List<object>();
		private long _nextID;

		public void AddColumn(int index, ColumnDef def)
		{
			var storeType = typeof(ColumnStore<>).MakeGenericType(new Type[] { def.Type });
			
			// TODO: allow for a custom value sorter
			// TODO: wire up row sorter to next column
			var store = Activator.CreateInstance(storeType, new object[] { null, null });
			
			_stores.Insert(index, store);

			_defs.Insert(index, def);
		}

        public void Insert(long id, params object[] values)
        {
            if (values.Length != _defs.Count)
                throw new ArgumentException("Hey! We need the same number of values as columns!");

            for (int i = 0; i < values.Length; i++)
            {
				var value = values[i];
				if (value != null)
				{
					var def = _defs[i];
					dynamic store = _stores[i];
					store.Insert(values[i], id);
				}
            }            
        }

		public long Insert(params object[] values)
		{
			var id = Interlocked.Increment(ref _nextID);
			Insert(id, values);
			return id;
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

		public IEnumerable<object[]> Select(int column, bool isForward, int[] projection)
		{
			dynamic store = _stores[column];
			foreach (var entry in store.GetRows(isForward))
			{
				var id = entry.Value;
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
				yield return values;
			}
		}
    }
}
