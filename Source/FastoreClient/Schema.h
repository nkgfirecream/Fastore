#pragma once

#include "ColumnDef.h"
#include "Dictionary.h"
#include <map>
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
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Collections::ObjectModel;

namespace Alphora
{
	namespace Fastore
	{
		namespace Client
		{
			class Schema : public std::map<int, ColumnDef>
			{
			public:
				Schema();
				Schema(const boost::shared_ptr<IDictionary<int, ColumnDef>> &initial);
			};
		}
	}
}
