using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.Diagnostics;

namespace Fastore.Core
{
    //TODO: Implement Splitting
    //Split algorithm --
    //Find middle node (walk the tree)
    //Duplicate every parent of node
    //Move every child, every right sibling, and every right sibling of every parent to new tree.
    public class PatriciaTrie<Value>
    {
        private static int MinArraySize = 4;
        private Node _root;
        private IComparer<char> _comparer = Comparer<char>.Default;

        //Count is indicative of the actual number of values in tree,
        //not the number of nodes. Whereas the count on each node is
        //indicative of the number of children it has.
        //Some nodes may not have a value (routing nodes), so even though
        //they are present and have children, they do not add to the count;
        public int Count { get; private set; }

        public PatriciaTrie()
        {
            _root = new Node();
        }

        public PatriciaTrie(Node branch)
        {
            _root = new Node();
            InsertChild(_root, branch, 0);
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

        public Optional<Value> Insert(string key, Value value)
        {
            return AddSubNode(_root, key, 0, value);
        }

        //We are doing a char by char comparison so we can get the matchlength as the same time while comparing
        //It may be faster to do the search first, and then count the characters in common.
        //Just have to test and see.
        //index is the location of a node would go if it's inserted at the current level. bestindex is the index of the node to use
        //if we need to insert it as a child.
        private int NodeArrayBinarySearch(string key, int keyoffset, Node[] array, int count, out int matchlength, out int longestmatchingindex)
        {
            int lo = 0;
			int hi = count - 1;
			int localIndex = 0;
			int result = -1;

            int bestindex = -1;
            int bestlength = 0;
            int lastlength;

			while (lo <= hi)
			{
				localIndex = (lo + hi) / 2;
                result = OffsetCompare(key, keyoffset, array[localIndex].Key, out lastlength);

                if (lastlength > bestlength)
                {
                    bestlength = lastlength;
                    bestindex = localIndex;
                }

				if (result == 0)
					break;
				else if (result < 0)
					hi = localIndex - 1;
				else
					lo = localIndex + 1;
			}

            matchlength = bestlength;
            longestmatchingindex = bestindex;
            if (result == 0)
                return localIndex;
            else
                return lo;
        }

        private int OffsetCompare(string left, int offset, string right, out int matchlength)
        {
            int size = Math.Min(left.Length - offset, right.Length);
            int result = 0;
            int i;
            for(i = 0; i < size; i++)
            {
                result = _comparer.Compare(left[offset + i],right[i]);
                if(result != 0)
                    break;
            }
            matchlength = i;
            return result;
        }

        private Optional<Value> InternalInsert(Node node, string key, int keyoffset, int matchlength, Value value)
        {        
              //Full match on node key
            if (matchlength == node.Key.Length)
            {
                //Take off the match part
                int remaining = key.Length - keyoffset - matchlength;

                //If we have no more key, return value if it exists
                if (remaining == 0)
                {
                    //we found an existing node
                    if (node.Value != Optional<Value>.Null)
                        return node.Value;
                    else
                    {
                        //Adding value to an existing routing node.
                        Count++;
                        node.Value = value;
                        return Optional<Value>.Null;
                    }
                }
                //Otherwise, continue to add tail
                else
                {
                    return AddSubNode(node, key, keyoffset + matchlength, value);
                }
            }
            //A subpart of the nodes key was matched
            //That means it needs to split into two nodes.
            else
            {
                string common = String.Intern(node.Key.Substring(0, matchlength));

                //Our old tail goes into one child
                Node left = new Node();
                left.Key = string.Intern(node.Key.Substring(matchlength, node.Key.Length - matchlength));
                left.Value = node.Value;
                left.Children = node.Children;
                left.Count = node.Count;

                node.Key = common;
                node.Children = new Node[PatriciaTrie<Value>.MinArraySize];
                node.Count = 0;
                InsertChild(node, left, 0);

                //Our new tail goes to the other child, if we still have a tail. If we no longer have a tail, we go into the current node.
                var start = matchlength + keyoffset;
                var tail = String.Intern(key.Substring(start, key.Length - start));
                if (tail != String.Empty)
                {
                    node.Value = Optional<Value>.Null;

                    Node right = new Node();
                    right.Key = tail;
                    right.Value = value;
                    //The first character of the tails
                    //Should be sufficient to distinguish whether we need to go in front or in back.
                    if (_comparer.Compare(left.Key[0], right.Key[0]) > 0)
                        InsertChild(node, right, 0);
                    else
                        InsertChild(node, right, 1);
                }
                else
                {
                    node.Value = value;
                }
                
                //Added a new Value
                Count++;

                return Optional<Value>.Null;
            }
        }
      
        private Optional<Value> AddSubNode(Node node, string key, int keyoffset, Value value)
        {
            //No children on this node yet,
            //Add entire key (which is a tail of another key)
            if (node.Count == 0)
            {
                Node newchild = new Node();
                newchild.Key = String.Intern(key.Substring(keyoffset, key.Length - keyoffset));
                newchild.Value = value;
                InsertChild(node, newchild, 0);
                //Add a child with a new value;
                Count++;
                return Optional<Value>.Null;
            }
            else
            {
                //Find closest matching child to append to (it may split, have further children, whatever)               
                int bestlength = 0;
                int bestindex = -1;
           
                int index = NodeArrayBinarySearch(key, keyoffset, node.Children, node.Count, out bestlength, out bestindex); 

                //If we didn't find a close match, create a new child.
                if (bestlength == 0)
                {
                    Node right = new Node();
                    right.Key = string.Intern(key.Substring(keyoffset, key.Length - keyoffset));
                    right.Value = value;
                    InsertChild(node, right, index);
                    //Add a child with a new Value;
                    Count++;
                    return Optional<Value>.Null;
                }
                else
                {
                    return InternalInsert(node.Children[bestindex], key, keyoffset, bestlength, value);
                }
            }
        }

        //returns trued if node found and deleted
        public bool Delete(string key)
        {
            return Delete(_root, key, 0);
        }

        private bool Delete(Node node, string key, int keyoffset)
        {
            int matchlength;
            int bestindex;
            int index = NodeArrayBinarySearch(key, keyoffset, node.Children, node.Count, out matchlength, out bestindex);

            //No Match
            if (matchlength == 0)
                return false;
            //Complete Match
            else if (matchlength + keyoffset == key.Length)
            {
                var child = node.Children[index];
                child.Value = Optional<Value>.Null;

                //If the child only has one child, merge it.
                if (child.Count == 1)
                {
                    child.Value = child.Children[0].Value;
                    child.Key = child.Key + child.Children[0].Key;
                    child.Count = child.Children[0].Count;
                    child.Children = child.Children[0].Children;
                }
                else if (child.Count == 0)
                {
                    RemoveChild(node, index);
                }

                Count--;
                //Otherwise, it's a routing node, so leave it.
                return true;   
            }
            //Partial Match. Search Children
            else
            {
                var result = Delete(node.Children[index], key, keyoffset + matchlength);
                if (result == true)
                {
                    //Check and see if the child is empty. If so, remove it. If not, atttempt to merge.
                    var child = node.Children[index];
                    if (child.Value == Optional<Value>.Null)
                    {
                        if (child.Count == 0)
                            RemoveChild(node, index);
                        else if (child.Count == 1)
                        {
                            child.Value = child.Children[0].Value;
                            child.Key = child.Key + child.Children[0].Key;
                            child.Count = child.Children[0].Count;
                            child.Children = child.Children[0].Children;
                        }
                    }
                }

                return result;
            }
        }

        public Optional<Value> GetValue(string key)
        {
            return GetValue(_root, key, 0);
        }

        private Optional<Value> GetValue(Node node, string key, int keyoffset)
        {
            int matchlength;
            int bestindex;
            int index = NodeArrayBinarySearch(key, keyoffset, node.Children, node.Count, out matchlength, out bestindex);
            
            //No Match
            if(matchlength == 0)
                return Optional<Value>.Null;
            //Complete Match
            else if (matchlength + keyoffset == key.Length)
            {
                return node.Children[index].Value;
            }
            //Partial Match. Search Children
            else
            {
                return GetValue(node.Children[index], key, keyoffset + matchlength);
            }
        }

        public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward)
        {
            return Get(isForward, Optional<string>.Null);
        }

        public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward, Optional<string> start)
        {
            return Get(isForward, start, Optional<string>.Null);
        }

        //TODO: Implement IndexOf, starting and ending at given index;
        public IEnumerable<KeyValuePair<string, Value>> Get(bool isForward, Optional<string> start, Optional<string> end)
        {
            //var startIndex = start.HasValue ? IndexOf(start.Value) : 0;
            //var endIndex = end.HasValue ? IndexOf(end.Value) : Count + 1;
            //var startIndex = 0;
            //var endIndex = Count + 1;
            //if (isForward)
            //{
            //    for (int i = startIndex; i < endIndex; i++)
            //        foreach (var entry in _children[i].Get(isForward))
            //            yield return entry;
            //}
            //else
            //{
            //    for (int i = endIndex - 1; i >= startIndex; i--)
            //        foreach (var entry in _children[i].Get(isForward))
            //            yield return entry;
            //}

            yield return new KeyValuePair<string,Value>();
        }

        public override string ToString()
        {
            return ToString(_root, 0);
        }

        private string ToString(Node n, int offset)
        {
            var sb = new StringBuilder();
            for (int i = 0; i < offset; i++)
                sb.Append("   ");
            sb.AppendLine("<" + n.Key + "> = <" + n.Value + ">");
            if (n.Children != null)
            {
                for (int i = 0; i < n.Count; i++)
                {
                    sb.Append(ToString(n.Children[i], offset + 1));
                }
            }

            return sb.ToString();
        }

        public class Node
        {
            public Node[] Children = new Node[PatriciaTrie<Value>.MinArraySize];
            public string Key;
            public Optional<Value> Value;
            public int Count = 0;
           // private int PublicCount;
        }
    }
}
