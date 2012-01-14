using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
	/// <summary> Maintains a set of keys ordered by a given comparer. </summary>
	/// <typeparam name="K"> Key type. </typeparam>
	/// <typeparam name="O"> Owner type. </typeparam>
	public class KeyOnlyBTree<K>
	{
		public KeyOnlyBTree(IComparer<K> comparer)
		{
			Comparer = comparer ?? Comparer<K>.Default;
		}

		public IComparer<K> Comparer { get; private set; }

		public object Owner { get; set; }	// Can't make this generic or we recurse with main BTree
		
		// ... rest of BTree implementation

		// NOTE: the leaf must implement INode<K, KeyOnlyBTree<K>> by walking through the owner to find the leaf holding this tree

		public IEnumerator<K> Get(bool isForward)
		{
			throw new NotImplementedException();
		}

		public void Insert(K rowId)
		{
			throw new NotImplementedException();
		}
	}
}
