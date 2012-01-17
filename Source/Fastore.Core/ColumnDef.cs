using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
	public class ColumnDef
	{
		public ColumnDef(string name, Type type, bool isUnique)
		{
			this.Name = name;
			this.Type = type;
			this.IsUnique = isUnique;
		}
		
		public string Name { get; private set; }
		public Type Type { get; private set; }
		public bool IsUnique { get; private set; }
	}
}
