using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.Diagnostics;

namespace Fastore.Core
{

    //Todo: Delete
    public class PatriciaTrie<Value>
    {
        private Node _root;

        public PatriciaTrie()
        {
            _root = new Node();
        }       

        public void Insert(string key, Value value)
        {
            AddChild(_root, key, value);
        }

        private int CommonPrefixLength(string left, string right)
        {
            int size = Math.Min(left.Length, right.Length);

            for (int i = 0; i < size; i++)
                if (left[i] != right[i])
                    return i;

            return size;
        }

        private void InternalInsert(Node node, string key, Value value)
        {
            int matchlength = CommonPrefixLength(key, node.Key);

            //The nodes entire key was matched.
            //Add the tail as a child
            if (matchlength == node.Key.Length)
            {
                key = key.Substring(matchlength, key.Length - matchlength);
                AddChild(node, key, value);
            }
            //A subpart of the nodes key was matched
            //That means it needs to split into two nodes The node must have a minimum of two children.
            else
            {
                string common = node.Key.Substring(0, matchlength);

                //Our old tail goes to the left child
                //TODO: Some sort of ordering on the children will speed up access. Right
                //now they are randomly ordered.
                Node left = new Node();
                left.Key = node.Key.Substring(matchlength, node.Key.Length - matchlength);
                left.Value = node.Value;
                left.Children = node.Children;

                //Our new tail goes to the right child.
                Node right = new Node();
                right.Key = key.Substring(matchlength, key.Length - matchlength);
                right.Value = value;


                //The node now just has the common value
                node.Key = common;
                node.Value = default(Value); //Optional.. Need to add that.
                node.Children = new List<Node>();
                node.Children.Add(left);
                node.Children.Add(right);
            }
        }
      
        private void AddChild(Node node, string key, Value value)
        {
            //No children on this node yet,
            //Add entire key (which is a tail of another key)
            if (node.Children.Count == 0)
            {
                node.Children = new List<Node>();
                Node newchild = new Node();
                newchild.Key = key;
                newchild.Value = value;
                node.Children.Add(newchild);

            }
            else
            {
                //Find closest matching child to append to (it may split, have further children, whatever)
                //TODO: Some sort of ordering would help us match faster
                int bestindex = -1;
                int bestlength = 0;
                for (int i = 0; i < node.Children.Count; i++)
                {
                    int b = CommonPrefixLength(node.Children[i].Key, key);

                    if (b > bestlength)
                    {
                        bestlength = b;
                        bestindex = i;
                    }
                }

                //If we didn't find a close match, create a new child.
                if (bestindex == -1)
                {
                    Node right = new Node();
                    right.Key = key;
                    right.Value = value;
                    node.Children.Add(right);
                }
                else
                {
                    InternalInsert(node.Children[bestindex], key, value);
                }
            }
        }

        public Value Get(string key)
        {
            Node curnode = _root;
            while (curnode != null)
            {
                //Key wasn't a match, no more children to search
                if (curnode.Children.Count == 0)
                    return default(Value);

                //Find closest match
                int bestindex = -1;
                int bestlength = 0;
                for (int i = 0; i < curnode.Children.Count; i++)
                {
                    int b = CommonPrefixLength(curnode.Children[i].Key, key);
                    if (b > bestlength)
                    {
                        bestlength = b;
                        bestindex = i;
                    }
                }

                //Found a potentential match;
                if (bestindex != -1)
                {
                    key = key.Substring(bestlength, key.Length - bestlength);
                    curnode = curnode.Children[bestindex];

                    //All out of tail.
                    if (key.Length == 0)
                        return curnode.Value;
                }
                else
                {
                    //No match
                    return default(Value);
                }
            }

            return default(Value);
        }

        public void DisplayAsTree()
        {
            DisplayAsTree(_root, 0);
        }

        private void DisplayAsTree(Node n, int offset)
        {
            for (int i = 0; i < offset; i++)
                Debug.Write("   ");
            Debug.WriteLine("<{0}> = <{1}>", n.Key, n.Value);
            if (n.Children != null)
            {
                foreach (Node c in n.Children)
                {
                    DisplayAsTree(c, offset + 1);
                }
            }
        }

        public class Node
        {
            public List<Node> Children = new List<Node>();
            public string Key { get; set; }
            public Value Value { get; set; }
        }
    }
}
