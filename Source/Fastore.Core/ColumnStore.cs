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
			_values = new BTree<T, KeyBTree<long>>(comparer);
		}

		private IComparer<T> _comparer;
		private IComparer<long> _rowComparer;

		private BTree<T, KeyBTree<long>> _values;

		private Dictionary<long, IKeyLeaf<long>> _rows = new Dictionary<long, IKeyLeaf<long>>();

		public T? GetValue(long rowId)
		{
			IKeyLeaf<long> leaf;
			if (!_rows.TryGetValue(rowId, out leaf))
				return null;

			var tree = leaf.Tree;
			var owner = (IBTreeLeaf<T, KeyBTree<long>>)tree.Owner;
			return owner.GetKey(tree, Comparer<KeyBTree<long>>.Default);
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
			// TODO: avoid constructing row btree until needed
			// Create a new row bucket in case there isn't only for the given value  
			var newRows = new KeyBTree<long>(_rowComparer);

			// Attempt to insert the row bucket
			IBTreeLeaf<T, KeyBTree<long>> valueLeaf;

			// If already existing, use it
			var existingRows = _values.Insert(value, newRows, out valueLeaf);
			if (existingRows != null)
				newRows = existingRows.Value;
			else
				newRows.Owner = valueLeaf;

			// Add the row into the row bucket
			IKeyLeaf<long> idLeaf;
			if (newRows.Insert(rowId, out idLeaf))
			{
				// If a row was added, add the row leaf to the hash table
				_rows.Add(rowId, idLeaf);
			}
		}
	}
}
