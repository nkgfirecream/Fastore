using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public interface IKeyValueTree<Key, Value>
	{
		Optional<Value> Insert(Key key, Value value, out IBTreeLeaf<Key, Value> leaf);
		IEnumerator<KeyValuePair<Key, Value>> Get(bool isForward);
		IEnumerator<KeyValuePair<Key, Value>> Get(Key start, bool isForward);
		IEnumerator<KeyValuePair<Key, Value>> Get(Key start, Key end, bool isForward);
		event ValueMovedHandler<Key, Value> ValueMoved;
    }
}
