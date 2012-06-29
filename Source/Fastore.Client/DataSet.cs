using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;

namespace Alphora.Fastore.Client
{
    public class DataSet : IEnumerable<DataSet.DataSetRow>
    {
        private DataSetRow[] _rows;
        private int _columnCount;

        public DataSet(int rows, int columnCount)
        {
            _rows = new DataSetRow[rows];
            _columnCount = columnCount;
        }

        public DataSetRow this[int index]
        {
            get
            {
                var result = _rows[index];
				if (result.Values == null)
                    result.Values[index] = new object[_columnCount];
                return result;
            }
            set
            {
				_rows[index] = value;
            }
        }

        public int Count
        {
            get { return _rows.Length; }
        }

        public IEnumerator<DataSetRow> GetEnumerator()
		{
			for (int i = 0; i < _rows.Length; i++)
				yield return this[i];
		}

		IEnumerator IEnumerable.GetEnumerator()
		{
			return this.GetEnumerator();
		}

		public struct DataSetRow
		{
			public object[] Values;
			public object ID;
		}	
	}


}
