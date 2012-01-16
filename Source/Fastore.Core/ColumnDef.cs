using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
	public class ColumnDef
	{
		public ColumnDef(string name, Type type)
		{
			this.Name = name;
			this.Type = type;
		}
		
		public string Name { get; private set; }
		public Type Type { get; private set; }
	}
}
