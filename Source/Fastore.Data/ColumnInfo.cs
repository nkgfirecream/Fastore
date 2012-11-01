using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Data
{
	public struct ColumnInfo
	{
		public string Name;
		public string LogicalType;
		public Provider.ArgumentType PhysicalType;
	}
}
