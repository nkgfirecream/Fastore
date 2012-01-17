using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    //Need to implement Delete, Get, Contains, Binary searching, etc.
    public class BTree<Key, Value> : IKeyValueTree<Key, Value>
    {
		public BTree(IComparer<Key> comparer = null, int fanout = 128, int leafSize = 128)
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
        public Optional<Value> Insert(Key key, Value value, out IBTreeLeaf<Key,Value> leaf)
        {
            var result = _root.Insert(key, value, out leaf);
            if (result.Split != null)
                _root = new Branch(this, _root, result.Split.Right, result.Split.Key);
			return result.Found;
        }

        internal void DoValuesMoved(IBTreeLeaf<Key, Value> leaf)
        {
            if (ValueMoved != null)
				foreach (var entry in leaf.Get(true))
					ValueMoved(entry.Value, leaf);
        }

		class Branch : INode
		{
			public Branch(BTree<Key, Value> tree)
			{
				_tree = tree;
				_keys = new Key[tree.Fanout - 1];
				_children = new INode[tree.Fanout];
			}

			public Branch(BTree<Key, Value> tree, INode left, INode right, Key key)
				: this(tree)
			{
				_children[0] = left;
				_children[1] = right;
				_keys[0] = key;
				Count = 1;
			}

			private Key[] _keys;
			private INode[] _children;
			private BTree<Key, Value> _tree;

			/// <summary> Count is numbers of keys. Number of children is keys + 1. </summary>
			public int Count { get; private set; }

			public InsertResult Insert(Key key, Value value, out IBTreeLeaf<Key, Value> leaf)
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
				int size = Count - index;
				Array.Copy(_keys, index, _keys, index + 1, size);
				Array.Copy(_children, index + 1, _children, index + 2, size);

				_keys[index] = key;
				_children[index + 1] = child;
				Count++;
			}

			private int IndexOf(Key key)
			{
				var result = Array.BinarySearch(_keys, 0, Count, key);
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

		class Leaf : INode, IBTreeLeaf<Key, Value>
		{
			private Value[] _values { get; set; }
			private Key[] _keys { get; set; }
			private int Count { get; set; }

			private BTree<Key, Value> _tree;

			public Leaf(BTree<Key, Value> parent)
			{
				_tree = parent;
				_keys = new Key[parent._leafSize];
				_values = new Value[parent._leafSize];
			}

			public InsertResult Insert(Key key, Value value, out IBTreeLeaf<Key, Value> leaf)
			{
				int pos = IndexOf(key);
				var result = new InsertResult();
				if (Count == _tree._leafSize)
				{
					var node = new Leaf(_tree);
					node.Count = (_tree._leafSize + 1) / 2;
					Count = Count - node.Count;

					Array.Copy(_keys, node.Count, node._keys, 0, node.Count);
					Array.Copy(_values, node.Count, node._values, 0, node.Count);

					if (pos < Count)
						result.Found = InternalInsert(key, value, pos, out leaf);
					else
						result.Found = node.InternalInsert(key, value, pos - Count, out leaf);

					_tree.DoValuesMoved(node);

					result.Split = new Split() { Key = node._keys[0], Right = node };
				}
				else
					result.Found = InternalInsert(key, value, pos, out leaf);

				return result;
			}

			public Optional<Value> InternalInsert(Key key, Value value, int index, out IBTreeLeaf<Key, Value> leaf)
			{
				leaf = this;
				if (index < Count && _tree.Comparer.Compare(_keys[index], key) == 0)
					return _values[index];
				else
				{
					Array.Copy(_keys, index, _keys, index + 1, Count - index);
					Array.Copy(_values, index, _values, index + 1, Count - index);
					_keys[index] = key;
					_values[index] = value;
					Count++;
					return Optional<Value>.Null;
				}
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
						yield return new KeyValuePair<Key, Value>(_keys[i], _values[i]);
				}
				else
				{
					for (int i = endIndex - 1; i >= startIndex; i--)
						yield return new KeyValuePair<Key, Value>(_keys[i], _values[i]);
				}
			}

			public Optional<Key> GetKey(Value value, IComparer<Value> comparer)
			{
				for (int i = 0; i < Count; i++)
				{
					if (comparer.Compare(_values[i], value) == 0)
						return _keys[i];
				}
				return Optional<Key>.Null;
			}
		}

		private interface INode
		{
			InsertResult Insert(Key key, Value value, out IBTreeLeaf<Key, Value> leaf);
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
