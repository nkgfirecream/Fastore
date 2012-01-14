using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public interface INode<Key, Value>
    {
        InsertResult<Key, Value> Insert(Key key, Value value, out ILeaf<Key, Value> leaf);
        void Dump();
		IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward);
	}

    public interface ILeaf<Key, Value> : INode<Key, Value>
    {
        Key? GetKey(Value value, IComparer<Value> comparer);
		IEnumerator<KeyValuePair<Key, Value>> Get(bool isForward);
    }

	public struct InsertResult<Key, Value>
	{
		public Value? Found;
		public Split<Key, Value> Split;
	}

    public class Split<Key, Value>
    {
        public Key Key;
        public INode<Key, Value> Right;
    }
}
