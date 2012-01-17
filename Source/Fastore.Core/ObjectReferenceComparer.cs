using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
	public class ObjectReferenceComparer<T> : IComparer<T>
		where T : class
	{
		public static readonly ObjectReferenceComparer<T> Instance = new ObjectReferenceComparer<T>();

		public int Compare(T x, T y)
		{
			var xHash = x == null ? 0 : x.GetHashCode();
			var yHash = y == null ? 0 : y.GetHashCode();
			return xHash.CompareTo(yHash);
		}
	}
}
