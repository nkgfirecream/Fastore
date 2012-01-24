using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    //Need to implement Delete, Get, Contains, Binary searching, etc.
    public class KeyValueLeafBTree<Key, Value> : IKeyValueTree<Key, Value>
    {
		public KeyValueLeafBTree(IComparer<Key> comparer = null, int fanout = 128, int leafSize = 128)
		{
			if (fanout < 2 || leafSize < 2)
				throw new ArgumentException("Minimum fan-out and leaf size is 2.");
			_fanout = fanout;
			_leafSize = leafSize;

			Comparer = comparer ?? Comparer<Key>.Default;

			_root = new Leaf(this);
		}

		public IComparer<Key> Comparer { get; private set; }

		private int _fanout = 10;
        public int Fanout
		{
			get { return _fanout; }
		}

        private int _leafSize = 100;
		public int LeafSize
		{
			get { return _leafSize; }
		}

		public event ValueMovedHandler<Key, Value> ValueMoved;
		
		private INode _root;

        public override string ToString()
        {
            return _root.ToString();
        }

		public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward)
		{
			return _root.Get(isForward);
		}

		public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward, Optional<Key> start)
		{
			return _root.Get(isForward, start);
		}

		public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward, Optional<Key> start, Optional<Key> end)
		{
			return _root.Get(isForward, start, end);
		}

		/// <summary> Attempts to insert a given key/value</summary>
		/// <param name="leaf"> The leaf in which the entry was located. </param>
		/// <returns> If the key already exists, nothing is changed and the existing value is returned. </returns>
        public Optional<Value> Insert(Key key, Value value, out IKeyValueLeaf<Key,Value> leaf)
        {
            var result = _root.Insert(key, value, out leaf);
            if (result.Split != null)
                _root = new Branch(this, _root, result.Split.Right, result.Split.Key);
			return result.Found;
        }

        internal void DoValuesMoved(IKeyValueLeaf<Key, Value> leaf)
        {
            if (ValueMoved != null)
				foreach (var entry in leaf.Get(true))
					ValueMoved(entry.Value, leaf);
        }

		class Branch : INode
		{
			public Branch(KeyValueLeafBTree<Key, Value> tree)
			{
				_tree = tree;
				_keys = new Key[tree.Fanout - 1];
				_children = new INode[tree.Fanout];
			}

			public Branch(KeyValueLeafBTree<Key, Value> tree, INode left, INode right, Key key)
				: this(tree)
			{
				_children[0] = left;
				_children[1] = right;
				_keys[0] = key;
				Count = 1;
			}

			private Key[] _keys;
			private INode[] _children;
			private KeyValueLeafBTree<Key, Value> _tree;

			/// <summary> Count is numbers of keys. Number of children is keys + 1. </summary>
			public int Count { get; private set; }

			public InsertResult Insert(Key key, Value value, out IKeyValueLeaf<Key, Value> leaf)
			{
				var index = IndexOf(key);
				var result = _children[index].Insert(key, value, out leaf);

				// If child split, add the adjacent node
				if (result.Split != null)
					result.Split = InsertChild(index, result.Split.Key, result.Split.Right);
				return result;
			}

			private Split InsertChild(int index, Key key, INode child)
			{
				// If full, split
				if (Count == _tree.Fanout - 1)
				{
					int mid = (Count + 1) / 2;

					// Create new sibling node
					Branch node = new Branch(_tree);
					node.Count = Count - mid;
					Array.Copy(_keys, mid, node._keys, 0, node.Count);
					Array.Copy(_children, mid, node._children, 0, node.Count + 1);

					Count = mid - 1;

					Split result = new Split() { Key = _keys[mid - 1], Right = node };

					if (index <= Count)
						InternalInsertChild(index, key, child);
					else
						node.InternalInsertChild(index - (Count + 1), key, child);

					return result;
				}
				else
				{
					InternalInsertChild(index, key, child);
					return null;
				}
			}

			private void InternalInsertChild(int index, Key key, INode child)
			{
                if (index != Count)
                {
                    int size = Count - index;
                    Array.Copy(_keys, index, _keys, index + 1, size);
                    Array.Copy(_children, index + 1, _children, index + 2, size);
                }

				_keys[index] = key;
				_children[index + 1] = child;
				Count++;
			}

			private int IndexOf(Key key)
			{
				var result = Array.BinarySearch(_keys, 0, Count, key, _tree.Comparer);
				if (result < 0)
				{
					var index = ~result;
					if (index > Count)
						return Count;
					else
						return index;
				}
				else
					return result;
			}

			public override string ToString()
			{
				var sb = new StringBuilder("[");
				for (int i = 0; i <= Count; i++)
					sb.Append(i.ToString() + ": " + _children[i].ToString());
				sb.Append("]");
				return sb.ToString();
			}

			public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward)
			{
				return Get(isForward, Optional<Key>.Null);
			}

			public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward, Optional<Key> start)
			{
				return Get(isForward, start, Optional<Key>.Null);
			}

			public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward, Optional<Key> start, Optional<Key> end)
			{
				var startIndex = start.HasValue ? IndexOf(start.Value) : 0;
				var endIndex = end.HasValue ? IndexOf(end.Value) : Count + 1;
				if (isForward)
				{
					for (int i = startIndex; i < endIndex; i++)
						foreach (var entry in _children[i].Get(isForward))
							yield return entry;
				}
				else
				{
					for (int i = endIndex - 1; i >= startIndex; i--)
						foreach (var entry in _children[i].Get(isForward))
							yield return entry;
				}
			}
		}

		class Leaf : INode, IKeyValueLeaf<Key, Value>
		{
            private KeyValuePair<Key, Value>[] _items { get; set; }
			private int Count { get; set; }

			private KeyValueLeafBTree<Key, Value> _tree;

			public Leaf(KeyValueLeafBTree<Key, Value> parent)
			{
				_tree = parent;
                _items = new KeyValuePair<Key, Value>[parent._leafSize];
			}

			public InsertResult Insert(Key key, Value value, out IKeyValueLeaf<Key, Value> leaf)
			{
                int pos = IndexOf(key);
				var result = new InsertResult();
				if (Count == _tree._leafSize)
				{                 
                   
					var node = new Leaf(_tree);
					// Determine the new node size - if the insert is to the end, leave this node full, assume contiguous insertions
					node.Count = 
						pos == Count
							? 0
							: (_tree._leafSize + 1) / 2;
					Count = Count - node.Count;

					Array.Copy(_items, node.Count, node._items, 0, node.Count);

					if (pos < Count)
						result.Found = InternalInsert(key, value, pos, out leaf);
					else
						result.Found = node.InternalInsert(key, value, pos - Count, out leaf);

					_tree.DoValuesMoved(node);

					result.Split = new Split() { Key = node._items[0].Key, Right = node };
				}
				else
					result.Found = InternalInsert(key, value, pos, out leaf);

				return result;
			}

			public Optional<Value> InternalInsert(Key key, Value value, int index, out IKeyValueLeaf<Key, Value> leaf)
			{
				leaf = this;
				if (index < Count && _tree.Comparer.Compare(_items[index].Key, key) == 0)
					return _items[index].Value;
                else
                {
                    if(index != Count)
                        Array.Copy(_items, index, _items, index + 1, Count - index);

                    _items[index] = new KeyValuePair<Key, Value>(key, value);
                    Count++;
                    return Optional<Value>.Null;
                }
			}

			private int IndexOf(Key key)
			{
                int lo = 0;
                int hi = Count - 1;
                int localIndex = 0;
                int result = -1;

                while (lo <= hi)
                {
                    localIndex = (lo + hi) / 2;
                    result = result = _tree.Comparer.Compare(key, _items[localIndex].Key);

                    if (result == 0)
                        break;
                    else if (result < 0)
                        hi = localIndex - 1;
                    else
                        lo = localIndex + 1;
                }

                if (result == 0)
                    return localIndex;
                else
                    return lo;
			}

			public override string ToString()
			{
				var sb = new StringBuilder("{");
				var first = true;
				foreach (var entry in Get(true))
				{
					if (!first)
						sb.Append(",");
					else
						first = false;
					sb.Append(entry.Key);
					sb.Append(" : ");
					sb.Append(entry.Value);
				}
				sb.Append("}");
				return sb.ToString();
			}

			public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward)
			{
				return Get(isForward, Optional<Key>.Null);
			}

			public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward, Optional<Key> start)
			{
				return Get(isForward, start, Optional<Key>.Null);
			}

			public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward, Optional<Key> start, Optional<Key> end)
			{
				var startIndex = start.HasValue ? IndexOf(start.Value) : 0;
				var endIndex = end.HasValue ? IndexOf(end.Value) : Count; 
				if (isForward)
				{
					for (int i = startIndex; i < endIndex; i++)
						yield return _items[i];
				}
				else
				{
					for (int i = endIndex - 1; i >= startIndex; i--)
						yield return _items[i];
				}
			}

			public Optional<Key> GetKey(Func<Value, bool> predicate)
			{
				for (int i = 0; i < Count; i++)
				{
					if (predicate(_items[i].Value))
						return _items[i].Key;
				}
				return Optional<Key>.Null;
			}
		}

        //private class KVSortComparer : IComparer<KeyValuePair<Key,Value>>
        //{
        //    public KVSortComparer(IComparer<Key> comparer)
        //    {
        //        _comparer = comparer;
        //    }
        //    private IComparer<Key> _comparer;

        //    public int Compare(KeyValuePair<Key, Value> x, KeyValuePair<Key, Value> y)
        //    {
        //        return _comparer.Compare(x.Key, y.Key);
        //    }
        //}

		private interface INode
		{
			InsertResult Insert(Key key, Value value, out IKeyValueLeaf<Key, Value> leaf);
			IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward);
			IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward, Optional<Key> start);
			IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward, Optional<Key> start, Optional<Key> end);
		}

		private struct InsertResult
		{
			public Optional<Value> Found;
			public Split Split;
		}

		private class Split
		{
			public Key Key;
			public INode Right;
		}
	}
}
