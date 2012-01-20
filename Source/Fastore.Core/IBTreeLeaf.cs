using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public interface IBTreeLeaf<Key, Value>
    {
		IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward);
		Optional<Key> GetKey(Func<Value, bool> predicate);
    }
}
