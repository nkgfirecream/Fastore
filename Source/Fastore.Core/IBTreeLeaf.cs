using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public interface IBTreeLeaf<Key, Value>
    {
		IEnumerator<KeyValuePair<Key, Value>> Get(bool isForward);
		Optional<Key> GetKey(Value value, IComparer<Value> comparer);
    }
}
