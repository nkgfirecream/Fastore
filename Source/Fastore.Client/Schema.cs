using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;

namespace Alphora.Fastore.Client
{
	public class Schema : Dictionary<int, ColumnDef>
	{
		public Schema() {}
		public Schema(IDictionary<int, ColumnDef> initial) : base(initial) { }
	}
}
