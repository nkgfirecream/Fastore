using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace Alphora.Fastore.Client
{
    //This class should probably be more strict about type enforcement.
    public class DataSet : IEnumerable<DataSetRow>
    {
        private DataSetRow[] _data = null;
        private int _columns;

        public DataSet(int rows, int columns)
        {
            _data = new DataSetRow[rows];
            _columns = columns;
        }

        public DataSetRow this[int index]
        {
            get
            {
                if (_data[index] == null)
                    _data[index] = new DataSetRow(_columns);

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

        public IEnumerator<DataSetRow> GetEnumerator()
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
        public bool EndOfFile = false;
        public bool BeginOfFile = false;
        public bool Limited = false;
	}

    public class DataSetRow
    {
        public DataSetRow(int columns)
        {
            Values = new object[columns];
        }

        public object[] Values;
        public object ID;
    }
}
