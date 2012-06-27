using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
	public class TransactionIDComparer : IComparer<TransactionID>
	{
		public static readonly TransactionIDComparer Default = new TransactionIDComparer();

		public int Compare(TransactionID x, TransactionID y)
		{
			return x.Revision.CompareTo(y.Revision);
		}
	}
}
