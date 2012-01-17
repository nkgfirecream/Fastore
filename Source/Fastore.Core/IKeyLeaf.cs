using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
	public interface IKeyLeaf<K>
	{
		IEnumerator<K> Get(bool isForward);
		KeyBTree<K> Tree { get; }
	}
}
