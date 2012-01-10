using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    //Incomplete... Basically still just a copy of the BTree.
    public class PrefixBTree<Value> : IKeyValueTree<string, Value>
    {
        public int BranchingFactor = 10;
        public int LeafSize = 100;
        private INode<string,Value> Root;
        private ILeafSubscriber<string,Value> Parent;
        private int Column;

        public PrefixBTree(int branching, int leafsize, ILeafSubscriber<string,Value> parent, int column)
        {
            if (branching < 2)
                throw new ArgumentException("Minimum branching factor is 2.");

            BranchingFactor = branching;
            LeafSize = leafsize;
            Parent = parent;
            Column = column;
            
            Root = new PrefixLeaf<Value>(this);
        }

        public void Dump()
        {
            Root.Dump();
        }

        public IEnumerable<Value> OrderedValues()
        {
            return Root.OrderedValues();
        }

        public void Insert(string key, Value value, out ILeaf<string,Value> leaf)
        {
            var result = Root.Insert(key, value, out leaf);
            if(result != null)
            {
                var tmp = new PrefixBranch<Value>(this);
                tmp.children[0] = result.left;
                tmp.children[1] = result.right;
                tmp.keys[0] = result.key;
                tmp.count = 1;
                Root = tmp;
            }
        }

        public void UpdateLinks(ILeaf<string, Value> leaf)
        {
            if (Parent != null)
            {
                for (int i = 0; i < leaf.Count; i++)
                {
                    foreach (var item in leaf.Values[i])
                        Parent.UpdateLink(item, Column, leaf);
                }
            }
        }
    }

    public class PrefixBranch<Value> : INode<string, Value>
    {
        public INode<string, Value>[] children;
        private PrefixBTree<Value> parent;
        public int count;

        private string prefix;

        public string[] keys;

        public PrefixBranch(PrefixBTree<Value> parent)
        {
            this.parent = parent;
            keys = new string[parent.BranchingFactor - 1];
            children = new INode<string, Value>[parent.BranchingFactor];
        }

        public Split<string, Value> Insert(string key, Value value, out ILeaf<string, Value> leaf)
        {
            //count is numbers of keys. Number of children is keys + 1;
            //An optimization would to split from the bottom up, rather than to top down
            //That way we only split when we actually need to.
            if (count == parent.BranchingFactor - 1)
            {
                int mid = (count + 1) / 2;
                int size = count - mid;
                PrefixBranch<Value> node = new PrefixBranch<Value>(parent);
                node.count = size;

                Array.Copy(keys, mid, node.keys, 0, size);
                Array.Copy(children, mid, node.children, 0, size + 1);

                count = mid - 1;

                Split<string, Value> result = new Split<string, Value>() { key = keys[mid - 1], left = this, right = node };
                if (key.CompareTo(result.key) < 0)
                {
                    InternalInsert(key, value, out leaf);
                }
                else
                {
                    node.InternalInsert(key, value, out leaf);
                }

                return result;
            }
            else
            {
                InternalInsert(key, value, out leaf);
                return null;
            }
        }

        public void InternalInsert(string key, Value value, out ILeaf<string, Value> leaf)
        {
            var index = IndexOf(key);
            var result = children[index].Insert(key, value, out leaf);
            if (result != null)
            {
                int size = count - index;
                Array.Copy(keys, index, keys, index + 1, size);
                Array.Copy(children, index, children, index + 1, size + 1);

                keys[index] = result.key;
                children[index + 1] = result.right;
                count++;
            }
        }

        private int IndexOf(string key)
        {
            var result = Array.BinarySearch(keys, 0, count, key);
            if (result < 0)
            {
                var index = ~result;
                if (index > count)
                    return count;
                else
                    return index;
            }
            else
                return result;
        }

        public void Dump()
        {
            for (int i = 0; i <= count; i++)
            {
                children[i].Dump();
            }
        }

        public IEnumerable<Value> OrderedValues()
        {
            for (int i = 0; i <= count; i++)
            {
                foreach (var value in children[i].OrderedValues())
                {
                    yield return value;
                }
            }
        }
    }

    public class PrefixLeaf<Value> : ILeaf<string, Value>
    {
        public ISet<Value>[] Values { get; set; }
        public string[] Keys { get; set; }
        public int Count { get; set; }

        private string prefix;

        private PrefixBTree<Value> parent;

        public PrefixLeaf(PrefixBTree<Value> parent)
        {
            this.parent = parent;
            Keys = new string[parent.LeafSize];
            Values = new HashSet<Value>[parent.LeafSize];
        }


        public Split<string, Value> Insert(string key, Value value, out ILeaf<string, Value> leaf)
        {
            int pos = IndexOf(key);
            if (Count == parent.LeafSize)
            {
                int mid = (parent.LeafSize + 1) / 2;
                int size = Count - mid;
                var node = new PrefixLeaf<Value>(parent);
                node.Count = mid;

                Array.Copy(Keys, mid, node.Keys, 0, size);
                Array.Copy(Values, mid, node.Values, 0, size);
                Count = mid;

                if (pos < mid)
                {
                    InternalInsert(key, value, pos, out leaf);
                }
                else
                {
                    node.InternalInsert(key, value, pos - Count, out leaf);
                }

                parent.UpdateLinks(node);

                var result = new Split<string, Value>() { key = node.Keys[0], left = this, right = node };

                return result;
            }
            else
            {
                InternalInsert(key, value, pos, out leaf);
                return null;
            }
        }

        public void InternalInsert(string key, Value value, int index, out ILeaf<string, Value> leaf)
        {
            if (Keys[index] != null && Keys[index].Equals(key))
            {
                if (Values[index] == null)
                    Values[index] = new HashSet<Value>();
                Values[index].Add(value);
            }
            else
            {
                Array.Copy(Keys, index, Keys, index + 1, Count - index);
                Array.Copy(Values, index, Values, index + 1, Count - index);
                Keys[index] = key;
                Values[index] = new HashSet<Value>();
                Values[index].Add(value);
                Count++;
            }

            leaf = this;
        }

        private int IndexOf(string key)
        {
            var result = Array.BinarySearch(Keys, 0, Count, key);
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

        public void Dump()
        {
            for (int i = 0; i < Count; i++)
            {
                Console.WriteLine(Keys[i]);
            }
        }

        public IEnumerable<Value> OrderedValues()
        {
            for (int i = 0; i < Count; i++)
            {
                foreach (var value in Values[i])
                {
                    yield return value;
                }
            }
        }

        public string GetKey(Value value)
        {
            for (int i = 0; i < Count; i++)
            {
                if (Values[i].Contains(value))
                    return Keys[i];
            }
            //Still missing leaves... what the deal?
            //throw new Exception("Incorrect leaf!");
            return default(string);
        }
    }
}
