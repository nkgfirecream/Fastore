using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    //Need to implement Delete, Get, Contains, Binary searching, etc.
    public class SimplePrefixBTree<Value> : IKeyValueTree<string, Value>
    {
		public SimplePrefixBTree(IComparer<string> comparer = null, int fanout = 16, int leafSize = 100)
		{
			if (fanout < 2 || leafSize < 2)
				throw new ArgumentException("Minimum fan-out and leaf size is 2.");
			_fanout = fanout;
			_leafSize = leafSize;

			Comparer = comparer ?? Comparer<string>.Default;
			_root = new Leaf(this);
		}

		public IComparer<string> Comparer { get; private set; }

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
		
		private INode _root;

        public override string ToString()
        {
            return _root.ToString();
        }

        public event ValueMovedHandler<string, Value> ValueMoved;

		public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward)
		{
			return _root.Get(isForward);
		}

        public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward, Optional<string> start)
        {
            return _root.Get(isForward, start);
        }

        public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward, Optional<string> start, Optional<string> end)
        {
            return _root.Get(isForward, start, end);
        }

		/// <summary> Attempts to insert a given key/value</summary>
		/// <param name="leaf"> The leaf in which the entry was located. </param>
		/// <returns> If the key already exists, nothing is changed and the existing value is returned. </returns>
        public Optional<Value> Insert(string key, Value value, out IBTreeLeaf<string,Value> leaf)
        {
            var result = _root.Insert(key, value, out leaf);
            if (result.Split != null)
                _root = new Branch(this, _root, result.Split.Right, result.Split.Key);
			return result.Found;
        }

        internal void DoValuesMoved(IBTreeLeaf<string, Value> leaf)
        {
            if (ValueMoved != null)
				foreach (var entry in leaf.Get(true))
					ValueMoved(entry.Value, leaf);
        }

		class Branch : INode
		{
            public Branch(SimplePrefixBTree<Value> tree)
			{
				_tree = tree;
				_keys = new string[tree.Fanout - 1];
				_children = new INode[tree.Fanout];
			}

			public Branch(SimplePrefixBTree<Value> tree, INode left, INode right, string key)
				: this(tree)
			{
				_children[0] = left;
				_children[1] = right;
				_keys[0] = key;
				Count = 1;
			}           

			private string[] _keys;
			private INode[] _children;
            private SimplePrefixBTree<Value> _tree;

			/// <summary> Count is numbers of keys. Number of children is keys + 1. </summary>
			public int Count { get; private set; }

			public InsertResult Insert(string key, Value value, out IBTreeLeaf<string, Value> leaf)
			{
				var index = IndexOf(key);
				var result = _children[index].Insert(key, value, out leaf);

				// If child split, add the adjacent node
				if (result.Split != null)
					result.Split = InsertWithSplit(index, result.Split.Key, result.Split.Right);
				return result;
			}

			private Split InsertWithSplit(int index, string key, INode child)
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
                        InternalInsert(index, key, child);
                    else
                        node.InternalInsert(index - (Count + 1), key, child);

                    return result;
                }
                else
                {
                    InternalInsert(index, key, child);
                    return null;
                }
			}
           

			private void InternalInsert(int index, string key, INode child)
			{
                int size = Count - index;
                Array.Copy(_keys, index, _keys, index + 1, size);
                Array.Copy(_children, index + 1, _children, index + 2, size);

                //This stores the shortest possible tring to differentiate between the key below the new one, and the new one.
                //If there is not a differentiating string, we will store the new key.
                _keys[index] = MinSeparation(_keys[index], key, _tree.Comparer);
				_children[index + 1] = child;
				Count++;
			}

            private string MinSeparation(string left, string right, IComparer<string> comparer)
            {
                if (String.IsNullOrEmpty(left))
                    return right;
                //Make sure they aren't equal beforehand. If they are, this will fail.
                int size = left.Length > right.Length ? right.Length : left.Length;

                //go until the strings are distinguished
                int i;
                for (i = 0; i < size; i++)
                {
                    if (left[i] != right[i])
                        break;
                }

                //If we haven't distinguished the strings, just use a key as long as the shortest string
                //This is why it's important to make sure the string aren't equal beforehand.
                if (i == size)
                    return right.Substring(0, Math.Min(right.Length, left.Length + 1));

                //If we have distinguished the strings, try to find one between the two
                var prefix = left.Substring(0, i) + (char)(((int)left[i] + (int)right[i]) / 2);

                //if we've blown it, just give up. The strings are probably so similar we aren't going to see any savings.
                //We could try for several more iterations, but there are cases such as aaaa and aaaaa where there simply aren't any distinguishing strings.
                if (comparer.Compare(prefix, left) < 0)
                    return right;

                return prefix;
            }

			private int IndexOf(string key)
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

            public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward)
            {
                return Get(isForward, Optional<string>.Null);
            }

            public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward, Optional<string> start)
            {
                return Get(isForward, start, Optional<string>.Null);
            }

            public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward, Optional<string> start, Optional<string> end)
            {
                var startIndex = start.HasValue ? IndexOf(start.Value) : 0;
                var endIndex = end.HasValue ? IndexOf(end.Value) : Count + 1;
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

		class Leaf : INode, IBTreeLeaf<string, Value>
		{
			private Value[] _values { get; set; }
			private string[] _keys { get; set; }
			private int Count { get; set; }

			private SimplePrefixBTree<Value> _tree;

			public Leaf(SimplePrefixBTree<Value> parent)
			{
				_tree = parent;
				_keys = new string[parent.LeafSize];
				_values = new Value[parent.LeafSize];
			}

			public InsertResult Insert(string key, Value value, out IBTreeLeaf<string, Value> leaf)
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

			public Optional<Value> InternalInsert(string key, Value value, int index, out IBTreeLeaf<string, Value> leaf)
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
					return Optional<Value>.Null;
				}
			}

			private int IndexOf(string key)
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

            public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward)
            {
                return Get(isForward, Optional<string>.Null);
            }

            public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward, Optional<string> start)
            {
                return Get(isForward, start, Optional<string>.Null);
            }

            public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward, Optional<string> start, Optional<string> end)
            {
                var startIndex = start.HasValue ? IndexOf(start.Value) : 0;
                var endIndex = end.HasValue ? IndexOf(end.Value) : Count;
                if (isForward)
                {
                    for (int i = startIndex; i < endIndex; i++)
                        yield return new KeyValuePair<string, Value>(_keys[i], _values[i]);
                }
                else
                {
                    for (int i = endIndex - 1; i >= startIndex; i--)
                        yield return new KeyValuePair<string, Value>(_keys[i], _values[i]);
                }
            }

            public Optional<string> GetKey(Value value, IComparer<Value> comparer)
            {
                for (int i = 0; i < Count; i++)
                {
                    if (comparer.Compare(_values[i], value) == 0)
                        return _keys[i];
                }
                return Optional<string>.Null;
            }
		}

        private interface INode
        {
            InsertResult Insert(string key, Value value, out IBTreeLeaf<string, Value> leaf);
            IEnumerable<KeyValuePair<string, Value>> Get(bool isForward);
            IEnumerable<KeyValuePair<string, Value>> Get(bool isForward, Optional<string> start);
            IEnumerable<KeyValuePair<string, Value>> Get(bool isForward, Optional<string> start, Optional<string> end);
        }

		private struct InsertResult
		{
			public Optional<Value> Found;
			public Split Split;
		}

		private class Split
		{
			public string Key;
			public INode Right;
		}      
    }
}
