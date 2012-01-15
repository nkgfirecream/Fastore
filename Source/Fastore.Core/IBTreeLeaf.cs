using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public interface IBTreeLeaf<Key, Value>
    {
		IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward);
        Key? GetKey(Value value, IComparer<Value> comparer);
    }
}
