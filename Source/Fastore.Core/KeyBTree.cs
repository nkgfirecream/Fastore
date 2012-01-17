using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
	/// <summary> Maintains a set of keys ordered by a given comparer. </summary>
	/// <typeparam name="K"> Key type. </typeparam>
	public class KeyBTree<K>
	{
        public static int BranchingFactor = 10;
        public static int LeafSize = 100;

        private IKeyNode _root;

		public KeyBTree(IComparer<K> comparer)
		{
			Comparer = comparer ?? Comparer<K>.Default;
            _root = new Leaf(this);
		}

		public IComparer<K> Comparer { get; private set; }

		public object Owner { get; set; }	// Can't make this generic or we recurse with main BTree
		

		public IEnumerable<K> Get(bool isForward)
		{
            return _root.Get(isForward);
		}

		/// <returns> True if an item was added (not already existing). </returns>
		public bool Insert(K rowId, out IKeyLeaf<K> leaf)
		{
            var result = _root.Insert(rowId, out leaf);
            if (result.Split != null)
                _root = new Branch(this, _root, result.Split.Right, result.Split.Key);

            return result.Added;
		}

        class Branch : IKeyNode
        {
            public Branch(KeyBTree<K> tree)
            {
                _tree = tree;
                _keys = new K[KeyBTree<K>.BranchingFactor - 1];
                _children = new IKeyNode[KeyBTree<K>.BranchingFactor];
            }

            public Branch(KeyBTree<K> tree, IKeyNode left, IKeyNode right, K key)
                : this(tree)
            {
                _children[0] = left;
                _children[1] = right;
                _keys[0] = key;
                Count = 1;
            }

            private K[] _keys;
            private IKeyNode[] _children;
            private KeyBTree<K> _tree;

            /// <summary> Count is numbers of keys. Number of children is keys + 1. </summary>
            public int Count { get; private set; }

            public InsertResult Insert(K key, out IKeyLeaf<K> leaf)
            {
                var index = IndexOf(key);
                var result = _children[index].Insert(key, out leaf);

                // If child split, add the adjacent node
                if (result.Split != null)
                    result.Split = InsertWithSplit(index, result.Split.Key, result.Split.Right);
                return result;
            }

            private Split InsertWithSplit(int index, K key, IKeyNode child)
            {
                // If full, split
                if (Count == KeyBTree<K>.BranchingFactor - 1)
                {
                    int mid = (Count + 1) / 2;

                    // Create new sibling node
                    Branch node = new Branch(_tree);
                    node.Count = Count - mid;
                    Array.Copy(_keys, mid, node._keys, 0, node.Count);
                    Array.Copy(_children, mid, node._children, 0, node.Count + 1);

                    Count = mid - 1;

                    Split result = new Split() { Key = _keys[mid - 1], Right = node };

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

            private void InternalInsert(int index, K key, IKeyNode child)
            {
                int size = Count - index;
                Array.Copy(_keys, index, _keys, index + 1, size);
                Array.Copy(_children, index, _children, index + 1, size + 1);

                _keys[index] = key;
                _children[index + 1] = child;
                Count++;
            }

            private int IndexOf(K key)
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

            public IEnumerable<K> Get(bool isForward)
            {
                if (isForward)
                {
                    for (int i = 0; i <= Count; i++)
                        foreach (var entry in _children[i].Get(isForward))
                            yield return entry;
                }
                else
                {
                    for (int i = Count - 1; i >= 0; i--)
                        foreach (var entry in _children[i].Get(isForward))
                            yield return entry;
                }
            }
        }

        class Leaf : IKeyNode, IKeyLeaf<K>
        {
            private K[] _keys { get; set; }

            private int Count { get; set; }

            public KeyBTree<K> Tree { get; private set;}

            public Leaf(KeyBTree<K> parent)
            {
                Tree = parent;
                _keys = new K[KeyBTree<K>.LeafSize];
            }

            public InsertResult Insert(K key, out IKeyLeaf<K> leaf)
            {
                int pos = IndexOf(key);
                var result = new InsertResult();
                if (Count == KeyBTree<K>.LeafSize)
                {
                    var node = new Leaf(Tree);
                    node.Count = (KeyBTree<K>.LeafSize + 1) / 2;
                    Count = Count - node.Count;

                    Array.Copy(_keys, node.Count, node._keys, 0, node.Count);

                    if (pos < Count)
                        result.Added = InternalInsert(key, pos, out leaf);
                    else
                        result.Added = node.InternalInsert(key, pos - Count, out leaf);

                    result.Split = new Split() { Key = node._keys[0], Right = node };
                }
                else
                    result.Added = InternalInsert(key, pos, out leaf);

                return result;
            }

            public bool InternalInsert(K key, int index, out IKeyLeaf<K> leaf)
            {
                leaf = this;
                if (Tree.Comparer.Compare(_keys[index], key) == 0)
                    return false;
                else
                {
                    Array.Copy(_keys, index, _keys, index + 1, Count - index);
                    _keys[index] = key;
                    Count++;
                    return true;
                }
            }

            private int IndexOf(K key)
            {
                var result = Array.BinarySearch(_keys, 0, Count, key, Tree.Comparer);
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
                    sb.Append(entry);
                }
                sb.Append("}");
                return sb.ToString();
            }

            public IEnumerable<K> Get(bool isForward)
            {
                if (isForward)
                {
                    for (int i = 0; i < Count; i++)
                        yield return _keys[i];
                }
                else
                {
                    for (int i = Count - 1; i >= 0; i--)
                        yield return _keys[i];
                }
            }
        }

        private interface IKeyNode
        {
            InsertResult Insert(K key, out IKeyLeaf<K> leaf);
            IEnumerable<K> Get(bool isForward);
        }

        private struct InsertResult
        {
            public bool Added;
            public Split Split;
        }

        private class Split
        {
            public K Key;
            public IKeyNode Right;
        }
	}
}
