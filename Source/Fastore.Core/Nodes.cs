using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public abstract class Node<Key, Value>
    {
        public int count;
        public Key[] keys;
        public abstract Split<Key, Value> Insert(Key key, Value value, out Leaf<Key, Value> leaf);
        public abstract void Dump();
        public abstract IEnumerable<Value> OrderedValues();
    }

    public class Branch<Key, Value> : Node<Key, Value>
    {
        public Node<Key, Value>[] children;
        private BTree<Key, Value> parent;

        public Branch(BTree<Key,Value> parent)
        {
            this.parent = parent;
            keys = new Key[parent.BranchingFactor - 1];
            children = new Node<Key, Value>[parent.BranchingFactor];
        }

        public override Split<Key, Value> Insert(Key key, Value value, out Leaf<Key, Value> leaf)
        {
            //count is numbers of keys. Number of children is keys + 1;
            //An optimization would to split from the bottom up, rather than to top down
            //That way we only split when we actually need to.
            if (count == parent.BranchingFactor - 1)
            {
                int mid = (count + 1) / 2;
                int size = count - mid;
                Branch<Key, Value> node = new Branch<Key, Value>(parent);
                node.count = size;

                Array.Copy(keys, mid, node.keys, 0, size);
                Array.Copy(children, mid, node.children, 0, size + 1);

                count = mid - 1;

                Split<Key, Value> result = new Split<Key, Value>() { key = keys[mid - 1], left = this, right = node };
                if (Comparer<Key>.Default.Compare(key, result.key) < 0)
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

        public void InternalInsert(Key key, Value value, out Leaf<Key, Value> leaf)
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

        private int IndexOf(Key key)
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

        public override void Dump()
        {
            for (int i = 0; i <= count; i++)
            {
                children[i].Dump();
            }
        }

        public override IEnumerable<Value> OrderedValues()
        {
            for (int i = 0; i <= count; i++)
            {
                foreach( var value in children[i].OrderedValues())
                {
                    yield return value;
                }
            }
        }
    }  

    public class Leaf<Key, Value> : Node<Key, Value>
    {
        public HashSet<Value>[] values;
        private BTree<Key, Value> parent;

        public Leaf(BTree<Key, Value> parent)
        {
            this.parent = parent;
            keys = new Key[parent.LeafSize];
            values = new HashSet<Value>[parent.LeafSize];
        }


        public override Split<Key, Value> Insert(Key key, Value value, out Leaf<Key, Value> leaf)
        {
            int pos = IndexOf(key);
            if (count == parent.LeafSize)
            {
                int mid = (parent.LeafSize + 1) / 2;
                int size = count - mid;
                var node = new Leaf<Key, Value>(parent);
                node.count = mid;

                Array.Copy(keys, mid, node.keys, 0, size);
                Array.Copy(values, mid, node.values, 0, size);
                count = mid;

                if (pos < mid)
                {
                    InternalInsert(key, value, pos, out leaf);
                }
                else
                {
                    node.InternalInsert(key, value, pos - count, out leaf);
                }

                parent.UpdateLinks(node);

                var result = new Split<Key, Value>() { key = node.keys[0], left = this, right = node };

                return result;
            }
            else
            {
                InternalInsert(key, value, pos, out leaf);
                return null;
            }
        }

        public void InternalInsert(Key key, Value value, int index, out Leaf<Key, Value> leaf)
        {
            if (keys[index] != null && keys[index].Equals(key))
            {
                if (values[index] == null)
                    values[index] = new HashSet<Value>();
                values[index].Add(value);
            }
            else
            {
                Array.Copy(keys, index, keys, index + 1, count - index);
                Array.Copy(values, index, values, index + 1, count - index);
                keys[index] = key;
                values[index] = new HashSet<Value>();
                values[index].Add(value);
                count++;
            }

            leaf = this;
        }

        private int IndexOf(Key key)
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

        public override void Dump()
        {
            for (int i = 0; i < count; i++)
            {
                Console.WriteLine(keys[i]);
            }
        }

        public override IEnumerable<Value> OrderedValues()
        {
            for (int i = 0; i < count; i++)
            {
                foreach (var value in values[i])
                {
                    yield return value;
                }
            }
        }

        public Key GetKey(Value value)
        {
            for (int i = 0; i < count; i++)
            {
                if (values[i].Contains(value))
                    return keys[i];
            }
            //Still missing leaves... what the deal?
            //throw new Exception("Incorrect leaf!");
            return default(Key);
        }
    }
}
