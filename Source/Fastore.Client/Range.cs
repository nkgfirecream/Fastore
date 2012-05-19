using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    public struct Range
    {
        public int ColumnID;

        public RangeBound? Start;
        public RangeBound? End;
		public int Limit;
    }
}
