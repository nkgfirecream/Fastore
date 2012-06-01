using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    interface IDataAccess
    {
        DataSet GetRange(int[] columnIds, Range range, int limit, object startId = null);
        
        void Include(int[] columnIds, object rowId, object[] row);
        void Exclude(int[] columnIds, object rowId);

		Statistic[] GetStatistics(int[] columnIds);
    }
}
