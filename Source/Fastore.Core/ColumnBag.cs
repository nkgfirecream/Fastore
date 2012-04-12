﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace Fastore.Core
{
	public class ColumnBag<T>
	{
		public ColumnBag(IComparer<T> comparer = null, IComparer<long> rowComparer = null)
		{
			_comparer = comparer ?? Comparer<T>.Default;
			_rowComparer = rowComparer ?? Comparer<long>.Default;
			_values = new BTree<T, KeyBTree<long>>(_comparer);
		}

		private IComparer<T> _comparer;
		private IComparer<long> _rowComparer;

		private BTree<T, KeyBTree<long>> _values;

		private Dictionary<long, IKeyLeaf<long>> _rows = new Dictionary<long, IKeyLeaf<long>>();

		public Optional<T> GetValue(long rowId)
		{
			IKeyLeaf<long> leaf;
			if (!_rows.TryGetValue(rowId, out leaf))
				return Optional<T>.Null;

			var tree = leaf.Tree;
			var owner = (IKeyValueLeaf<T, KeyBTree<long>>)tree.Owner;
			return owner.GetKey(v => Object.ReferenceEquals(v, tree));
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(bool isForward)
		{
			foreach (var valueEntry in _values.Get(isForward))
				foreach (var rowEntry in valueEntry.Value.Get(isForward))
				    yield return new KeyValuePair<long, T>(rowEntry, valueEntry.Key);
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(bool isForward, Optional<T> start)
		{
			foreach (var valueEntry in _values.Get(isForward, start))
				foreach (var rowEntry in valueEntry.Value.Get(isForward))
					yield return new KeyValuePair<long, T>(rowEntry, valueEntry.Key);
		}

		public IEnumerable<KeyValuePair<long, T>> GetRows(bool isForward, Optional<T> start, Optional<T> end, int? limit)
		{
			var items = _values.Get(isForward, start, end);
			foreach (var valueEntry in limit.HasValue ? items.Take(limit.Value) : items)
				foreach (var rowEntry in valueEntry.Value.Get(isForward))
					yield return new KeyValuePair<long, T>(rowEntry, valueEntry.Key);
		}

		public bool Insert(T value, long rowId)
		{
			// TODO: avoid constructing row btree until needed
			// Create a new row bucket in case there isn't only for the given value  
			var newRows = new KeyBTree<long>(_rowComparer);

			// Attempt to insert the row bucket
			IKeyValueLeaf<T, KeyBTree<long>> valueLeaf;

			// If already existing, use it
			var existingRows = _values.Insert(value, newRows, out valueLeaf);
			if (existingRows != Optional<KeyBTree<long>>.Null)
				newRows = existingRows.Value;
			else
				newRows.Owner = valueLeaf;

			// Add the row into the row bucket
			IKeyLeaf<long> idLeaf;
			if (newRows.Insert(rowId, out idLeaf))
			{
				// If a row was added, add the row leaf to the hash table
				_rows.Add(rowId, idLeaf);

				return true;
			}
			else
				return false;
		}
	}
}
