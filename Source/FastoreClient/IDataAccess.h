#pragma once

#include "Range.h"
#include "RangeSet.h"
#include "DataSet.h"
#include "Statistic.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Collections::Generic;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Linq;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Text;

namespace Alphora
{
	namespace Fastore
	{
		namespace Client
		{
			class IDataAccess
			{
			public:
				virtual boost::shared_ptr<RangeSet> GetRange(int columnIds[], Range range, int limit, const boost::shared_ptr<object> &startId = nullptr) = 0;
				virtual boost::shared_ptr<DataSet> GetValues(int columnIds[], object rowIds[]) = 0;

				virtual void Include(int columnIds[], const boost::shared_ptr<object> &rowId, object row[]) = 0;
				virtual void Exclude(int columnIds[], const boost::shared_ptr<object> &rowId) = 0;

				virtual Statistic *GetStatistics(int columnIds[]) = 0;

				virtual std::map<int, TimeSpan> Ping() = 0;
			};
		}
	}
}
