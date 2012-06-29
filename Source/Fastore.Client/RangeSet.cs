using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
	public class RangeSet
	{
		public bool Eof { get; set; }
		public bool Bof { get; set; }
		public bool Limited { get; set; }
		public DataSet Data { get; set; }
	}
}
