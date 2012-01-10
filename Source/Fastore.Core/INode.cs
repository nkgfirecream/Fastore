using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public interface INode<Key,Value>
    {
        Split<Key, Value> Insert(Key key, Value value, out ILeaf<Key, Value> leaf);
        void Dump();
        IEnumerable<Value> OrderedValues();
    }

    public interface ILeaf<Key, Value> : INode<Key,Value>
    {
        Key[] Keys { get; set; }
        int Count { get; set; }
        ISet<Value>[] Values {get; set;}
        Key GetKey(Value value);
    }

    public class Split<Key, Value>
    {
        public Key key;
        public INode<Key, Value> left;
        public INode<Key, Value> right;
    }
}
