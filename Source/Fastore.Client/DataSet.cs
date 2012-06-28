using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace Alphora.Fastore.Client
{
    public class DataSet : IEnumerable<DataSetRow>
    {
        private object[][] _data = null;
		private object[] _rowIds = null;
        private int _columns;

        public DataSet(int rows, int columns)
        {
            _data = new object[rows][];
			_rowIds = new object[rows];
            _columns = columns;
        }

        public DataSetRow this[int index]
        {
            get
            {
                if (_data[index] == null)
                    _data[index] = new object[_columns];

                return new DataSetRow(_rowIds[index], _data[index]);
            }

            set
            {
                _data[index] = value.Values;
				_rowIds[index] = value.ID;
            }
        }

        public int Count
        {
            get { return _data.Length; }
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
        public bool Eof = false;
        public bool Bof = false;
        public bool Limited = false;
	}

    public struct DataSetRow
    {
        public DataSetRow(object id, object[] values)
        {
            Values = values;
			ID = id;
        }

        public object[] Values;
        public object ID;
    }
}
