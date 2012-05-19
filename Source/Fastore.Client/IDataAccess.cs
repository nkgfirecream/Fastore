using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
    interface IDataAccess
    {
        //Having the startId exposed on the client API still feels wrong to me. 
        //It exposes the inner workings of the buffering to the client. Perhaps a happy medium
        //would be to make the the client make buffered calls to the stores until it reaches a "clean break"
        //and then return that set.
        DataSet GetRange(int[] columnIds, Order[] orders, Range[] ranges, object startId = null);
        
        void Include(int[] columnIds, object rowId, object[] row);
        void Exclude(int[] columnIds, object rowId);

		Statistic[] GetStatistics(int[] columnIds);
    }
}
