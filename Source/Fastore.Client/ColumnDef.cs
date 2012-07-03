using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    public enum BufferType
    {
        Identity = 0,
        Unique = 1,
        Multi = 2
    }

    public struct ColumnDef
    {
        public int ColumnID { get; set; }
        public string Name { get; set; }
        public string Type { get; set; }
        public string IDType { get; set; }
        public BufferType BufferType { get; set; }
    }
}
