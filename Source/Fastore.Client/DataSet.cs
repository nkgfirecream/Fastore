using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace Alphora.Fastore.Client
{
    //This class should probably be more strict about type enforcement.
    public class DataSet : IEnumerable<object[]>
    {
        private object[][] _data = null;
        private int _columns;

        public DataSet(int rows, int columns)
        {
            _data = new object[rows][];
            _columns = columns;
        }       

        public object[] this[int index]
        {
            get
            {
                if (_data[index] == null)
                    _data[index] = new object[_columns];

                return _data[index];
            }
			set
			{
				_data[index] = value;
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
				yield return this[i];
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
