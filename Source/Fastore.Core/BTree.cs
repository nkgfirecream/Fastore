using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    //Need to implement Delete, Get, Contains, Binary searching, etc.
    public class BTree<Key, Value> 
    {
        public int BranchingFactor = 10;
        public int LeafSize = 100;
        private Node<Key,Value> Root;
        private ILeafSubscriber<Key,Value> Parent;
        private int Column;

        public BTree(int branching, int leafsize, ILeafSubscriber<Key,Value> parent, int column)
        {
            if (branching < 2)
                throw new ArgumentException("Minimum branching factor is 2.");

            BranchingFactor = branching;
            LeafSize = leafsize;
            Parent = parent;
            Column = column;
            
            Root = new Leaf<Key,Value>(this);
        }

        public void Dump()
        {
            Root.Dump();
        }

        public IEnumerable<Value> OrderedValues()
        {
            return Root.OrderedValues();
        }

        public void Insert(Key key, Value value, out Leaf<Key,Value> leaf)
        {
            var result = Root.Insert(key, value, out leaf);
            if(result != null)
            {
                var tmp = new Branch<Key,Value>(this);
                tmp.children[0] = result.left;
                tmp.children[1] = result.right;
                tmp.keys[0] = result.key;
                tmp.count = 1;
                Root = tmp;
            }
        }

        public void UpdateLinks(Leaf<Key, Value> leaf)
        {
            if (Parent != null)
            {
                for (int i = 0; i < leaf.count; i++)
                {
                    foreach (var item in leaf.values[i])
                        Parent.UpdateLink(item, Column, leaf);
                }
            }
        }
    }


    public class Split<Key, Value>
    {
        public Key key;
        public Node<Key, Value> left;
        public Node<Key, Value> right;
    }
}
