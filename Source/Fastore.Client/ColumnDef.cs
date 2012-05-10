using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    public class ColumnDef
    {
        public int ColumnID { get; set; }
        public string Name { get; set; }
        public string Type { get; set; }
        public string IDType { get; set; }
        public bool IsUnique { get; set; }
    }
}
