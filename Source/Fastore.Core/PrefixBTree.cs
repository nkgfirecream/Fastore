using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    //Need to implement Delete, Get, Contains, Binary searching, etc.
    public class PrefixBTree<Value> : IKeyValueTree<string, Value>
    {
		public PrefixBTree(IComparer<string> comparer = null)
		{
			Comparer = comparer ?? Comparer<string>.Default;
			_root = new Leaf(this);
		}

		public IComparer<string> Comparer { get; private set; }

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
		public event ValueMovedHandler<string, Value> ValueMoved;
		
		private INode _root;

        public override string ToString()
        {
            return _root.ToString();
        }

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
            var result = _root.Insert(key, 0, value, out leaf);
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
            public Branch(PrefixBTree<Value> tree)
			{
				_tree = tree;
				_keys = new string[tree.BranchingFactor - 1];
				_children = new INode[tree.BranchingFactor];
			}

			public Branch(PrefixBTree<Value> tree, INode left, INode right, string key)
				: this(tree)
			{
				_children[0] = left;
				_children[1] = right;
                //Assuming no shared prefix, this should adjust later as the node fills.
				_keys[0] = key;
                Prefix = "";
				Count = 1;
			}           

			private string[] _keys;
			private INode[] _children;
            private PrefixBTree<Value> _tree;
            public string Prefix { get; set; }

            private string CommonPrefix()
            {
                if (_keys.Length == 0)
                    return "";

                if (_keys.Length == 1)
                    return _keys[0];


                int prefixLength = 0;

                foreach (char c in _keys[0])
                {
                    foreach (string s in _keys)
                        if (s.Length <= prefixLength || s[prefixLength] != c)
                            return _keys[0].Substring(0, prefixLength);

                    prefixLength++;
                }

                //If we get here, something went awry. That means we have
                //move than 1 entry, and all the entries are the same.
                throw new Exception("Identical entries in keys!");
            }

            private void AdjustPrefixPostSplit()
            {
                var newprefix = CommonPrefix();

                //There's a larger common prefix due to a split 
                if (!string.IsNullOrEmpty(newprefix))
                {
                    for (int i = 0; i < Count; i++)
                        _keys[i] = _keys[i].Substring(newprefix.Length, _keys[i].Length - newprefix.Length);
                }

                //prefix gets longer
                Prefix = Prefix + newprefix;
            }

            private void AdjustPrefixPreInsert()
            {
                for (int i = 0; i < Count; i++)
                    _keys[i] = Prefix + _keys[i];

                Prefix = "";
            }

			/// <summary> Count is numbers of keys. Number of children is keys + 1. </summary>
			public int Count { get; private set; }

			public InsertResult Insert(string key, int stripped,  Value value, out IBTreeLeaf<string, Value> leaf)
			{
                var insertionkey = key.Substring(stripped, key.Length - stripped);

                if(!insertionkey.StartsWith(Prefix))
                {
                    AdjustPrefixPreInsert();
                }

				var index = IndexOf(insertionkey);
                
                //We need to entire value at the leaf, but we don't store the entire value here, only the parts we
                //need to distinguish keys at this level. So, tell the next level to ignore all the characters we did
                //plus, whatever characters we used as a prefix.
				var result = _children[index].Insert(key, stripped + Prefix.Length, value, out leaf);

				// If child split, add the adjacent node. The key for the node excludes whatever our parent uses as a prefix (stripped characters)
				if (result.Split != null)
					result.Split = InsertWithSplit(index + 1, result.Split.Key.Substring(stripped, result.Split.Key.Length - stripped), result.Split.Right);
				return result;
			}

			private Split InsertWithSplit(int index, string key, INode child)
			{
                //Adjust key to remove our common prefix (should be guaranteed to have a common prefix at the point,
                //because it came from our children, even if that prefix is empty
                var insertionkey = key.Substring(Prefix.Length, key.Length - Prefix.Length);

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

					Split result = new Split() { Key = Prefix + _keys[mid], Right = node };

					if (index < Count)
						InternalInsert(index, insertionkey, child);
					else
						node.InternalInsert(index - Count, insertionkey, child);

                    node.AdjustPrefixPostSplit();
                    AdjustPrefixPostSplit();

					return result;
				}
				else
				{
					InternalInsert(index, key, child);
					return null;
				}
			}

            //The key here MUST already be shortened to exclude the common prefix
			private void InternalInsert(int index, string key, INode child)
			{
				int size = Count - index;
				Array.Copy(_keys, index, _keys, index + 1, size);
				Array.Copy(_children, index, _children, index + 1, size + 1);

                _keys[index] = key;
				_children[index + 1] = child;
				Count++;
			}

            //The key here MUST already be shortened to exclude the common prefix
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

		class Leaf : INode, IBTreeLeaf<string, Value>
		{
			private Value[] _values { get; set; }
			private string[] _keys { get; set; }
			private int Count { get; set; }
            public string Prefix { get; private set; }

			private PrefixBTree<Value> _tree;

			public Leaf(PrefixBTree<Value> parent)
			{
				_tree = parent;
				_keys = new string[parent.LeafSize];
				_values = new Value[parent.LeafSize];
			}

            private string CommonPrefix()
            {
                if (_keys.Length == 0)  
                    return "";

                if (_keys.Length == 1)
                    return _keys[0];
                

                int prefixLength = 0;

                foreach (char c in _keys[0])
                {
                    foreach (string s in _keys)
                        if (s.Length <= prefixLength || s[prefixLength] != c)
                            return _keys[0].Substring(0, prefixLength);

                    prefixLength++;
                }

                //If we get here, something went awry. That means we have
                //move than one entry, and all the entries are the same.
                throw new Exception("Identical entries in keys!");
            }

            private void AdjustPrefixPostSplit()
            {
                var newprefix = CommonPrefix();

                //There's a larger common prefix due to a split 
                if(!string.IsNullOrEmpty(newprefix))
                {
                    for (int i = 0; i < Count; i++)
                        _keys[i] = _keys[i].Substring(newprefix.Length, _keys[i].Length - newprefix.Length);
                }

                //prefix gets longer
                Prefix = Prefix + newprefix;
            }

            private void AdjustPrefixPreInsert()
            {
                for (int i = 0; i < Count; i++)
                    _keys[i] = Prefix + _keys[i];

                Prefix = "";
            }

			public InsertResult Insert(string key, int stripped, Value value, out IBTreeLeaf<string, Value> leaf)
			{
                //This could happen in the case we are an end node or a root node.
                //An optimization would be not to start saving prefixes until
                // we are several levels deep, but this only happen in very rare cases
                // (such as bad,baby,baggage are the first three inserts, and then Dog is inserted)
                if (!key.StartsWith(Prefix))
                {
                    AdjustPrefixPreInsert();
                }           

                var insertionkey = key.Substring(Prefix.Length, key.Length  - Prefix.Length);

				int pos = IndexOf(insertionkey);
				var result = new InsertResult();
				if (Count == _tree.LeafSize)
				{
					var node = new Leaf(_tree);
					node.Count = (_tree.LeafSize + 1) / 2;
					Count = Count - node.Count;

					Array.Copy(_keys, node.Count, node._keys, 0, node.Count);
					Array.Copy(_values, node.Count, node._values, 0, node.Count);

                    //We know the nodes shared a prefix prior to splitting, so we can simple insert the temp value
                    //Once they are split, the can internally decided if they need a new prefix.
					if (pos < Count)
						result.Found = InternalInsert(insertionkey, value, pos, out leaf);
					else
						result.Found = node.InternalInsert(insertionkey, value, pos - Count, out leaf);                   

                    _tree.DoValuesMoved(node);

                    //When we return a key to the parent, we just return the entire thing,
                    //and let the parent sort it out (it should know to skip the first stripped characters)
                    //this let's us adjust our internal prefix independently of our parent (it stores short prefixes
                    //not long ones.)
					result.Split = new Split() { Key = Prefix + node._keys[0], Right = node };

                    //We are adjusting our internal prefix here. The parent
                    //bases its prefix on whatever we return as a key.
                    node.AdjustPrefixPostSplit();
                    AdjustPrefixPostSplit();
				}
				else
					result.Found = InternalInsert(key, value, pos, out leaf);

				return result;
			}

            //The key here MUST already be shortened to exclude the common prefix
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

            //The key here MUST already be shortened to exclude the common prefix
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
            InsertResult Insert(string key,int stripped, Value value, out IBTreeLeaf<string, Value> leaf);
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
