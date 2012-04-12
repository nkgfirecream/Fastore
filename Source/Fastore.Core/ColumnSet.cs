using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
	public class ColumnSet<T>
	{
		public ColumnSet(IComparer<T> comparer = null)
		{
			_comparer = comparer ?? Comparer<T>.Default;
			_values = new BTree<T, long>(_comparer);
		}

		private IComparer<T> _comparer;

		private BTree<T, long> _values;

		private Dictionary<long, IKeyValueLeaf<T, long>> _rows = new Dictionary<long, IKeyValueLeaf<T, long>>();

		public Optional<T> GetValue(long rowId)
		{
			IKeyValueLeaf<T, long> leaf;
			if (!_rows.TryGetValue(rowId, out leaf))
				return Optional<T>.Null;

			return leaf.GetKey(v => v == rowId);
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(bool isForward)
		{
			foreach (var valueEntry in _values.Get(isForward))
				yield return new KeyValuePair<long, T>(valueEntry.Value, valueEntry.Key);
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(bool isForward, Optional<T> start)
		{
			foreach (var valueEntry in _values.Get(isForward, start))
				yield return new KeyValuePair<long, T>(valueEntry.Value, valueEntry.Key);
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(bool isForward, Optional<T> start, Optional<T> end, int? limit)
		{
			var items = _values.Get(isForward, start, end);
			foreach (var valueEntry in limit.HasValue ? items.Take(limit.Value) : items)
				yield return new KeyValuePair<long, T>(valueEntry.Value, valueEntry.Key);
		}

		public bool Insert(T value, long rowId)
		{
			IKeyValueLeaf<T, long> valueLeaf;

			var existingRows = _values.Insert(value, rowId, out valueLeaf);
			if (existingRows != Optional<long>.Null)
				return false;
			else
			{
				// If a row was added, add the row leaf to the hash table
				_rows.Add(rowId, valueLeaf);

				return true;
			}
		}
	}
}
