using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    interface IDataAccess
    {
        RangeSet GetRange(int[] columnIds, Range range, int limit, object startId = null);
		DataSet GetValues(int[] columnIds, object[] rowIds);
        
        void Include(int[] columnIds, object rowId, object[] row);
        void Exclude(int[] columnIds, object rowId);

		Statistic[] GetStatistics(int[] columnIds);

		Dictionary<int, TimeSpan> Ping();
    }
}
