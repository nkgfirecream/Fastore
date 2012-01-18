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

        public PatriciaTrie(Node branch)
        {
            _root = new Node();
            branch.Parent = _root;
            _root.Children.Add(branch);
        }

        public PatriciaTrie<Value> SplitTreeAtKey(string key)
        {
            var node = GetNode(key);

            return SplitTreeAtNode(node);
        }

        private PatriciaTrie<Value> SplitTreeAtNode(Node node)
        {
            node.Key = BuildKeyFromNode(node);

            var parent = node.Parent;
            parent.Children.Remove(node);

            if (parent.Children.Count == 1)
                MergeWithParent(parent.Children[0]);
            else if (parent.Children.Count == 0 && parent.Value == Optional<Value>.Null && parent != _root)
                Delete(parent);

            node.Parent = null;           

            return new PatriciaTrie<Value>(node);
        }

        //public string GetKey(Value value)
        //{
            
        //}

        private string BuildKeyFromNode(Node node)
        {
            string key = "";
            var curnode = node;
            while(curnode != _root)
            {
                key = curnode.Key + key;
                curnode = curnode.Parent;
            }

            return key;
        }

        public IEnumerable<Value> GetValues()
        {
            return _root.GetValues();
        }

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
                foreach(var item in node.Children)
                {
                    item.Parent = left;
                }
                left.Parent = node;

                //Our new tail goes to the right child, if we still have a tail. If we no longer have a tail, we go into the current node.
                var tail = key.Substring(matchlength, key.Length - matchlength);
                node.Key = common;                
                node.Children = new List<Node>();
                node.Children.Add(left);

                if (tail != String.Empty)
                {
                    node.Value = Optional<Value>.Null;

                    Node right = new Node();
                    right.Key = tail;
                    right.Value = value;
                    right.Parent = node;
                    //The node now just has the common value                                   
                    node.Children.Add(right);
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
            if (node.Children.Count == 0)
            {
                node.Children = new List<Node>();
                Node newchild = new Node();
                newchild.Parent = node;
                newchild.Key = key;
                newchild.Value = value;
                node.Children.Add(newchild);
                return Optional<Value>.Null;
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
                    right.Parent = node;
                    right.Key = key;
                    right.Value = value;
                    node.Children.Add(right);
                    return Optional<Value>.Null;
                }
                else
                {
                    return InternalInsert(node.Children[bestindex], key, value);
                }
            }
        }

        //Returns true if found and deleted
        public bool Delete(string key)
        {
            var node = GetNode(key);
            if(node != null)
            {
                return Delete(node);
            }

            return false;
        }

        private bool Delete(Node node)
        {
            var parent = node.Parent;

            node.Value = Optional<Value>.Null;
            //If we are less than two, attempt to merge with parent
            if (node.Children.Count == 0)
            {
                node.Value = Optional<Value>.Null;
                //TODO: Add some sort of linking. Otherwise we have to retraverse to find the parent.
                parent.Children.Remove(node);
                if (parent.Children.Count == 0 && parent.Value == Optional<Value>.Null && parent != _root)
                {
                    return Delete(parent);
                }
                else if (parent.Children.Count == 1) 
                {
                    MergeWithParent(parent.Children[0]);
                }
            }
            else if(node.Children.Count == 1)
            {               
                MergeWithParent(node.Children[0]);
            }

            return true;
        }

        private void MergeWithParent(Node child)
        {
            var parent = child.Parent;

            //Merge if parent is not storing its own value, and it's not a routing node.
            if (parent.Value == Optional<Value>.Null && parent != _root)
            {
                parent.Key = parent.Key + child.Key;
                parent.Value = child.Value;
                parent.Children.Remove(child);
                foreach (var item in child.Children)
                {
                    item.Parent = parent;
                    parent.Children.Add(item);
                }

                if(parent.Children.Count == 1)
                    MergeWithParent(parent);
            }
            //If we've merged to the top of the tree, and haven't picked up any values, delete the child node
            else if(parent == _root && child.Value == Optional<Value>.Null && child.Children.Count == 0)
            {
                Delete(child);
            }
        }

        private Node GetNode(string key)
        {
            Node curnode = _root;
            while (curnode != null)
            {
                //Key wasn't a match, no more children to search
                if (curnode.Children.Count == 0)
                    return null;

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
                        return curnode;
                }
                else
                {
                    //No match
                    return null;
                }
            }

            return null;
        }

        public Optional<Value> GetValue(string key)
        {
            var node = GetNode(key);
            if (node != null)
                return node.Value;

            return Optional<Value>.Null;
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
            public Node Parent { get; set;}
            public List<Node> Children = new List<Node>();
            public string Key { get; set; }
            public Optional<Value> Value { get; set; }

            public IEnumerable<Value> GetValues()
            {
                if (Value != Optional<Value>.Null)
                    yield return Value.Value;

                foreach (var node in Children)
                    foreach (var item in node.GetValues())
                        yield return item;
            }
        }
    }
}
