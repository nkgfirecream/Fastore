using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace Alphora.Fastore.Client
{
    public class DataSet : IEnumerable<object[]>
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

		public IEnumerator<object[]> GetEnumerator()
		{
			for (int i = 0; i < _data.Length; i++)
				yield return _data[i];
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
			return this.GetEnumerator();
		}

        //This is with regards to the range that was used to request the dataset.
        //We need a better way to tie the two together.
        public bool EndOfRange = false;
	}
}
