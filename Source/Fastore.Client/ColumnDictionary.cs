using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
	/// <summary> The column IDs for the client. </summary>
	/// <remarks> Column IDs 10,000 through 19,999 are reserved for the client. </remarks>
	public static class ColumnDictionary
	{
		public const int GeneratorNextValue = 10000;
	}
}
