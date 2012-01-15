using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
	/// <summary> Maintains a set of keys ordered by a given comparer. </summary>
	/// <typeparam name="K"> Key type. </typeparam>
	public class KeyBTree<K>
	{
		public KeyBTree(IComparer<K> comparer)
		{
			Comparer = comparer ?? Comparer<K>.Default;
		}

		public IComparer<K> Comparer { get; private set; }

		public object Owner { get; set; }	// Can't make this generic or we recurse with main BTree
		
		// ... rest of BTree implementation

		public IEnumerator<K> Get(bool isForward)
		{
			throw new NotImplementedException();
		}

		/// <returns> True if an item was added (not already existing). </returns>
		public bool Insert(long rowId, out IKeyLeaf<K> leaf)
		{
			throw new NotImplementedException();
		}
	}
}
