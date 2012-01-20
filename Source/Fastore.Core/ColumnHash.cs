using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace Fastore.Core
{
	public class ColumnHash<T>
	{
		public ColumnHash(IComparer<T> comparer = null)
		{
			_comparer = comparer ?? Comparer<T>.Default;
			_values = new BTree<T, ISet<long>>(_comparer);
		}

		private IComparer<T> _comparer;

		private BTree<T, ISet<long>> _values;

		private Dictionary<long, IBTreeLeaf<T, ISet<long>>> _rows = new Dictionary<long, IBTreeLeaf<T, ISet<long>>>();

		public Optional<T> GetValue(long rowId)
		{
			IBTreeLeaf<T, ISet<long>> leaf;
			if (!_rows.TryGetValue(rowId, out leaf))
				return Optional<T>.Null;

			return leaf.GetKey(v => v.Contains(rowId));
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(bool isForward)
		{
			foreach (var valueEntry in _values.Get(isForward))
				if (isForward)
				{
					foreach (var rowEntry in valueEntry.Value)
						yield return new KeyValuePair<long, T>(rowEntry, valueEntry.Key);
				}
				else
				{
					foreach (var rowEntry in valueEntry.Value.Reverse())
						yield return new KeyValuePair<long, T>(rowEntry, valueEntry.Key);
				}
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(bool isForward, Optional<T> start)
		{
			foreach (var valueEntry in _values.Get(isForward, start))
				if (isForward)
				{
					foreach (var rowEntry in valueEntry.Value)
						yield return new KeyValuePair<long, T>(rowEntry, valueEntry.Key);
				}
				else
				{
					foreach (var rowEntry in valueEntry.Value.Reverse())
						yield return new KeyValuePair<long, T>(rowEntry, valueEntry.Key);
				}
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(bool isForward, Optional<T> start, Optional<T> end)
		{
			foreach (var valueEntry in _values.Get(isForward, start, end))
				if (isForward)
				{
					foreach (var rowEntry in valueEntry.Value)
						yield return new KeyValuePair<long, T>(rowEntry, valueEntry.Key);
				}
				else
				{
					foreach (var rowEntry in valueEntry.Value.Reverse())
						yield return new KeyValuePair<long, T>(rowEntry, valueEntry.Key);
				}
		}

		public bool Insert(T value, long rowId)
		{
			// TODO: avoid constructing row hashset until needed

			// Create a new row bucket in case there isn't only for the given value  
			ISet<long> newRows = new HashSet<long>();

			// Attempt to insert the row bucket
			IBTreeLeaf<T, ISet<long>> valueLeaf;
			var existingRows = _values.Insert(value, newRows, out valueLeaf);
			// If already existing, use it
			if (existingRows != Optional<ISet<long>>.Null)
				newRows = existingRows.Value;

			// Add the row into the row bucket
			if (newRows.Add(rowId))
			{
				// If a row was added, add the row leaf to the hash table
				_rows.Add(rowId, valueLeaf);

				return true;
			}
			else
				return false;
		}
	}
}
