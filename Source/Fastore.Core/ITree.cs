using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public interface IKeyValueTree<Key, Value>
    {
        Value Insert(Key key, Value value, out IBTreeLeaf<Key, Value> leaf);
        IEnumerable<Value> Get(bool isForward);
		event ValueMovedHandler<Key, Value> ValueMoved;
    }
}
