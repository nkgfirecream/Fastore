using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public interface IKeyValueTree<Key,Value>
    {
        void Dump();
        Value Insert(Key key, Value value, out ILeaf<Key, Value> leaf);
        IEnumerable<Value> OrderedValues();
        void DoValuesMoved(ILeaf<Key, Value> leaf);
    }
}
