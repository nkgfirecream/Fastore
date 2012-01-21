using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public interface IKeyValueTree<Key, Value>
	{
		Optional<Value> Insert(Key key, Value value, out IKeyValueLeaf<Key, Value> leaf);
		IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward);
		IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward, Optional<Key> start);
		IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward, Optional<Key> start, Optional<Key> end);
		event ValueMovedHandler<Key, Value> ValueMoved;
    }

	public interface IKeyValueLeaf<Key, Value>
	{
		IEnumerable<KeyValuePair<Key, Value>> Get(bool isForward);
		Optional<Key> GetKey(Func<Value, bool> predicate);
	}
	
	public delegate void ValueMovedHandler<Key, Value>(Value row, IKeyValueLeaf<Key, Value> newLeaf);
}
