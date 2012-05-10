using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    public class DataSet
    {
        private object[][] _data = null;

        public object[] this[int index]
        {
            get
            {
                return _data[index];
            }
        }

        public int Count
        {
            get
            {
                return _data.Length;
            }
        }
    }
}
