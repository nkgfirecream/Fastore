using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace Fastore.Core
{
	public class ColumnStore<T>
	{
		public ColumnStore(IComparer<T> comparer = null, IComparer<long> rowComparer = null)
		{
			_comparer = comparer ?? Comparer<T>.Default;
			_rowComparer = rowComparer ?? Comparer<long>.Default;
			_values = new BTree<T, KeyOnlyBTree<long>>(comparer);
		}

		private IComparer<T> _comparer;
		private IComparer<long> _rowComparer;

		private BTree<T, KeyOnlyBTree<long>> _values;

		private Dictionary<long, ILeaf<long>> _rows = new Dictionary<long, ILeaf<long>>();

		public T? GetValue(long rowId)
		{
			ILeaf<T, long> leaf;
			if (!_rows.TryGetValue(rowId, out leaf))
				return null;

			return leaf.GetKey(rowId, Comparer<long>.Default);
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(bool isForward)
		{
			foreach (var valueEntry in _values.Get(isForward))
				foreach (var rowEntry in valueEntry.Value.Get(isForward))
					yield return rowEntry;
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(T start, bool isForward)
		{
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(T start, T end, bool isForward)
		{
		}

		public void Insert(T value, long rowId)
		{
			var newRows = new KeyOnlyBTree<long>(_rowComparer);
			ILeaf<T, KeyOnlyBTree<long>> leaf;
			var existingRows = _values.Insert(value, newRows, out leaf);
			if (existingRows != null)
				newRows = existingRows.Value;
			ILeaf<long> up;
			newRows.Insert(rowId, out node);
		}
	}
}
