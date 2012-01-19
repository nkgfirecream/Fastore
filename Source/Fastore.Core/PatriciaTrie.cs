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
        private static int MinArraySize = 16;
        private Node _root;

        public PatriciaTrie()
        {
            _root = new Node();
        }

        public PatriciaTrie(Node branch)
        {
            _root = new Node();
            _root.Children[0] = branch;
            _root.Count++;
        }

        private void InsertChild(Node parent, Node child, int index)
        {
            var oldchildren = parent.Children;
            if (parent.Count == parent.Children.Length)
            {
                var newchildren = new Node[parent.Children.Length * 2];               
                parent.Children = newchildren;
            }

            Array.Copy(oldchildren, 0, parent.Children, 0, index);
            Array.Copy(oldchildren, index, parent.Children, index + 1, parent.Count - index);

            parent.Children[index] = child;
            parent.Count++;
        }

        private void RemoveChild(Node parent, int index)
        {
            var oldchildren = parent.Children;
            if (parent.Count < parent.Children.Length / 2 && parent.Children.Length > PatriciaTrie<Value>.MinArraySize)
            {
                var newchildren = new Node[parent.Children.Length / 2];
                parent.Children = newchildren;
            }

            Array.Copy(oldchildren, 0, parent.Children, 0, index);
            Array.Copy(oldchildren, index + 1, parent.Children, index, parent.Count - index);

            parent.Count--;
        }

        //public PatriciaTrie<Value> SplitTreeAtKey(string key)
        //{
        //    var node = GetNode(key);

        //    return SplitTreeAtNode(node);
        //}

        //private PatriciaTrie<Value> SplitTreeAtNode(Node node)
        //{
        //    node.Key = BuildKeyFromNode(node);

        //    var parent = node.Parent;
        //    parent.Children.Remove(node);

        //    if (parent.Children.Count == 1)
        //        MergeWithParent(parent.Children[0]);
        //    else if (parent.Children.Count == 0 && parent.Value == Optional<Value>.Null && parent != _root)
        //        Delete(parent);

        //    node.Parent = null;           

        //    return new PatriciaTrie<Value>(node);
        //}

        //private string BuildKeyFromNode(Node node)
        //{
        //    string key = "";
        //    var curnode = node;
        //    while(curnode != _root)
        //    {
        //        key = curnode.Key + key;
        //        curnode = curnode.Parent;
        //    }

        //    return key;
        //}


        public Optional<Value> Insert(string key, Value value)
        {
            return AddChild(_root, key, value);
        }

        private int CommonPrefixLength(string left, string right)
        {
            int size = Math.Min(left.Length, right.Length);

            for (int i = 0; i < size; i++)
                if (left[i] != right[i])
                    return i;

            return size;
        }

        private int NodeArrayBinarySearch(string key, int keyoffset, Node[] array)
        {


        }

        private Optional<Value> InternalInsert(Node node, string key, Value value)
        {        
            int matchlength = CommonPrefixLength(key, node.Key);

            //Full match on node key
            if (matchlength == node.Key.Length)
            {
                //Take off the match part
                key = key.Substring(matchlength, key.Length - matchlength);

                //If we have no more key, return value if it exists
                if (key == String.Empty)
                {
                    //we found an existing node
                    if (node.Value != Optional<Value>.Null)
                        return node.Value;
                    else
                    {
                        node.Value = value;
                        return Optional<Value>.Null;
                    }
                }
                //Otherwise, continue add tail
                else
                {
                    return AddChild(node, key, value);
                }
            }
            //A subpart of the nodes key was matched
            //That means it needs to split into two nodes.
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
                left.Count = node.Count;

                node.Key = common;
                node.Children = new Node[PatriciaTrie<Value>.MinArraySize];
                node.Count = 0;
                InsertChild(node, left, 0);

                //Our new tail goes to the right child, if we still have a tail. If we no longer have a tail, we go into the current node.
                var tail = key.Substring(matchlength, key.Length - matchlength);
                if (tail != String.Empty)
                {
                    node.Value = Optional<Value>.Null;

                    Node right = new Node();
                    right.Key = tail;
                    right.Value = value;
                    InsertChild(node, right, node.Count);           
                }
                else
                {
                    node.Value = value;
                }            

                return Optional<Value>.Null;
            }
        }
      
        private Optional<Value> AddChild(Node node, string key, Value value)
        {
            //No children on this node yet,
            //Add entire key (which is a tail of another key)
            if (node.Count == 0)
            {
                Node newchild = new Node();
                newchild.Key = key;
                newchild.Value = value;
                InsertChild(node, newchild, 0);
                return Optional<Value>.Null;
            }
            else
            {
                //Find closest matching child to append to (it may split, have further children, whatever)
                //TODO: Some sort of ordering would help us match faster
                int bestindex = -1;
                int bestlength = 0;
                for (int i = 0; i < node.Count; i++)
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
                    InsertChild(node, right, node.Count);
                    return Optional<Value>.Null;
                }
                else
                {
                    return InternalInsert(node.Children[bestindex], key, value);
                }
            }
        }

        //Returns true if found and deleted
        //public bool Delete(string key)
        //{
        //    var node = GetNode(key);
        //    if(node != null)
        //    {
        //        return Delete(node);
        //    }

        //    return false;
        //}

        //private bool Delete(Node node)
        //{


        //    node.Value = Optional<Value>.Null;
        //    //If we are less than two, attempt to merge with parent
        //    if (node.Children.Count == 0)
        //    {
        //        node.Value = Optional<Value>.Null;
        //        //TODO: Add some sort of linking. Otherwise we have to retraverse to find the parent.
        //        parent.Children.Remove(node);
        //        if (parent.Children.Count == 0 && parent.Value == Optional<Value>.Null && parent != _root)
        //        {
        //            return Delete(parent);
        //        }
        //        else if (parent.Children.Count == 1) 
        //        {
        //            MergeWithParent(parent.Children[0]);
        //        }
        //    }
        //    else if(node.Children.Count == 1)
        //    {               
        //        MergeWithParent(node.Children[0]);
        //    }

        //    return true;
        //}

        //private void MergeWithParent(Node child)
        //{
        //    var parent = child.Parent;

        //    //Merge if parent is not storing its own value, and it's not a routing node.
        //    if (parent.Value == Optional<Value>.Null && parent != _root)
        //    {
        //        parent.Key = parent.Key + child.Key;
        //        parent.Value = child.Value;
        //        parent.Children.Remove(child);
        //        foreach (var item in child.Children)
        //        {
        //            item.Parent = parent;
        //            parent.Children.Add(item);
        //        }

        //        if(parent.Children.Count == 1)
        //            MergeWithParent(parent);
        //    }
        //    //If we've merged to the top of the tree, and haven't picked up any values, delete the child node
        //    else if(parent == _root && child.Value == Optional<Value>.Null && child.Children.Count == 0)
        //    {
        //        Delete(child);
        //    }
        //}

        //private Node GetNode(string key)
        //{
        //    Node curnode = _root;
        //    while (curnode != null)
        //    {
        //        //Key wasn't a match, no more children to search
        //        if (curnode.Children.Count == 0)
        //            return null;

        //        //Find closest match
        //        int bestindex = -1;
        //        int bestlength = 0;
        //        for (int i = 0; i < curnode.Children.Count; i++)
        //        {
        //            int b = CommonPrefixLength(curnode.Children[i].Key, key);
        //            if (b > bestlength)
        //            {
        //                bestlength = b;
        //                bestindex = i;
        //            }
        //        }

        //        //Found a potentential match;
        //        if (bestindex != -1)
        //        {
        //            key = key.Substring(bestlength, key.Length - bestlength);
        //            curnode = curnode.Children[bestindex];

        //            //All out of tail.
        //            if (key.Length == 0)
        //                return curnode;
        //        }
        //        else
        //        {
        //            //No match
        //            return null;
        //        }
        //    }

        //    return null;
        //}

        //public Optional<Value> GetValue(string key)
        //{
        //    var node = GetNode(key);
        //    if (node != null)
        //        return node.Value;

        //    return Optional<Value>.Null;
        //}

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
                for (int i = 0; i < n.Count; i++)
                {
                    DisplayAsTree(n.Children[i], offset + 1);
                }
            }
        }

        public class Node
        {
            public Node[] Children = new Node[PatriciaTrie<Value>.MinArraySize];
            public string Key;
            public Optional<Value> Value;
            public int Count = 0;
        }
    }
}
