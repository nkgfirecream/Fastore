#pragma once

#include "RangeBound.h"
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
			class Range
			{
			public:
				int ColumnID;

				Nullable<RangeBound> Start;
				Nullable<RangeBound> End;
				bool Ascending;
			};
		}
	}
}
