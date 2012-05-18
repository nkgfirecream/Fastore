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

        //This is with regards to the range that was used to request the dataset.
        //We need a better way to tie the two together.
        public bool EndOfRange = false;
    }
}
