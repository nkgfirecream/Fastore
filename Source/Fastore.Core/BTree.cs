using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    //Need to implement Delete, Get, Contains, Binary searching, etc.
    public class BTree<Key, Value> : IKeyValueTree<Key, Value>
    {
		public BTree(IComparer<Key> comparer = null)
		{
			Comparer = comparer ?? Comparer<Key>.Default;
			_root = new Leaf(this);
		}

		public IComparer<Key> Comparer { get; private set; }

		private int _branchingFactor = 10;
        public int BranchingFactor
		{
			get { return _branchingFactor; }
			set 
			{
				if (value < 2)
					throw new ArgumentException("Minimum branching factor is 2.");
				_branchingFactor = value;
			}
		}

        public int LeafSize = 100;
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

		public IEnumerable<KeyValuePair<Key, Value>> Get(Key start, bool isForward)
		{
			return _root.Get(start, isForward);
		}

		public IEnumerable<KeyValuePair<Key, Value>> Get(Key start, Key end, bool isForward)
		{
			return _root.Get(start, end, isForward);
		}

		/// <summary> Attempts to insert a given key/value</summary>
		/// <param name="leaf"> The leaf in which the entry was located. </param>
		/// <returns> If the key already exists, nothing is changed and the existing value is returned. </returns>
        public Value? Insert(Key key, Value value, out IBTreeLeaf<Key,Value> leaf)
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
				_keys = new Key[tree.BranchingFactor - 1];
				_children = new INode[tree.BranchingFactor];
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
					result.Split = InsertWithSplit(index + 1, result.Key, result.Right);
				return result;
			}

			private Split InsertWithSplit(int index, Key key, INode child)
			{
				// If full, split
				if (Count == _tree.BranchingFactor - 1)
				{
					int mid = (Count + 1) / 2;

					// Create new sibling node
					Branch node = new Branch(_tree);
					node.Count = Count - mid - 1;
					Array.Copy(_keys, mid + 1, node._keys, 0, node.Count);
					Array.Copy(_children, mid + 1, node._children, 0, node.Count + 1);

					Count = mid;

					Split result = new Split() { Key = _keys[mid], Right = node };

					if (index < Count)
						InternalInsert(index, key, child);
					else
						node.InternalInsert(index - Count, key, child);

					return result;
				}
				else
				{
					InternalInsert(index, key, child);
					return null;
				}
			}

			private void InternalInsert(int index, Key key, INode child)
			{
				int size = Count - index;
				Array.Copy(_keys, index, _keys, index + 1, size);
				Array.Copy(_children, index, _children, index + 1, size + 1);

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
				var sb = new StringBuilder();
				for (int i = 0; i <= Count; i++)
					sb.AppendLine(_children[i].ToString());
				return sb.ToString();
			}

			public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward)
			{
				if (isForward)
				{
					for (int i = 0; i <= Count; i++)
						foreach (var entry in _children[i].Get(isForward))
							yield return entry;
				}
				else
				{
					for (int i = Count; i >= 0; i--)
						foreach (var entry in _children[i].Get(isForward))
							yield return entry;
				}
			}

			public IEnumerable<KeyValuePair<Key, Value>> Get(Key start, bool isForward)
			{
				var index = IndexOf(start);
				if (isForward)
				{
					for (int i = index; i <= Count; i++)
						foreach (var entry in _children[i].Get(isForward))
							yield return entry;
				}
				else
				{
					for (int i = Count; i >= index; i--)
						foreach (var entry in _children[i].Get(isForward))
							yield return entry;
				}
			}

			public IEnumerable<KeyValuePair<Key, Value>> Get(Key start, Key end, bool isForward)
			{
				var startIndex = IndexOf(start);
				var endIndex = IndexOf(end);
				if (isForward)
				{
					for (int i = startIndex; i <= endIndex; i++)
						foreach (var entry in _children[i].Get(isForward))
							yield return entry;
				}
				else
				{
					for (int i = endIndex; i >= startIndex; i--)
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
				_keys = new Key[parent.LeafSize];
				_values = new Value[parent.LeafSize];
			}

			public InsertResult Insert(Key key, Value value, out IBTreeLeaf<Key, Value> leaf)
			{
				int pos = IndexOf(key);
				var result = new InsertResult();
				if (Count == _tree.LeafSize)
				{
					var node = new Leaf(_tree);
					node.Count = (_tree.LeafSize + 1) / 2;
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

			public Value? InternalInsert(Key key, Value value, int index, out IBTreeLeaf<Key, Value> leaf)
			{
				leaf = this;
				if (_tree.Comparer.Compare(_keys[index], key) == 0)
					return _values[index];
				else
				{
					Array.Copy(_keys, index, _keys, index + 1, Count - index);
					Array.Copy(_values, index, _values, index + 1, Count - index);
					_keys[index] = key;
					_values[index] = value;
					Count++;
					return null;
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
				foreach (var entry in Get(true))
				{
					sb.Append(entry.Key);
					sb.Append(" : ");
					sb.Append(entry.Value);
				}
				sb.Append("}");
				return sb.ToString();
			}

			public IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward)
			{
				if (isForward)
				{
					for (int i = 0; i < Count; i++)
						yield return new KeyValuePair<Key, Value>(_keys[i], _values[i]);
				}
				else
				{
					for (int i = Count - 1; i >= 0; i--)
						yield return new KeyValuePair<Key, Value>(_keys[i], _values[i]);
				}
			}

			public Key? GetKey(Value value, IComparer<Value> comparer)
			{
				for (int i = 0; i < Count; i++)
				{
					if (comparer.Compare(_values[i], value) == 0)
						return _keys[i];
				}
				return null;
			}
		}

		private interface INode
		{
			InsertResult Insert(Key key, Value value, out IBTreeLeaf<Key, Value> leaf);
			IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward);
			IEnumerable<KeyValuePair<Key, Value>> Get(Key start, bool isForward);
			IEnumerable<KeyValuePair<Key, Value>> Get(Key start, Key end, bool isForward);
		}

		private struct InsertResult
		{
			public Value? Found;
			public Split Split;
		}

		private class Split
		{
			public Key Key;
			public INode Right;
		}
	}

	public delegate void ValueMovedHandler<Key, Value>(Value row, IBTreeLeaf<Key, Value> newLeaf);
}
