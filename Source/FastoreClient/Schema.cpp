#include "Schema.h"

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

			Schema::Schema()
			{
			}

			Schema::Schema(const boost::shared_ptr<IDictionary<int, ColumnDef>> &initial) : Dictionary(initial)
			{
			}
		}
	}
}
