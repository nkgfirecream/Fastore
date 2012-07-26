#pragma once

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
			/// <summary> The column IDs for the core and the client. </summary>
			/// <remarks> ColumnIDs 0-9,999 are reserved for the core engine.  Column IDs 10,000 through 
			/// 19,999 are reserved for the client. </remarks>
			class Dictionary
			{
			public:
				static const int MaxSystemColumnID = 9999;
				static const int MaxClientColumnID = 19999;

				static const int ColumnID = 0;
				static const int ColumnName = 1;
				static const int ColumnValueType = 2;
				static const int ColumnRowIDType = 3;
				static const int ColumnBufferType = 4;

				static const int ColumnColumns[5] = {ColumnID, ColumnName, ColumnValueType, ColumnRowIDType, ColumnBufferType};

				static const int TopologyID = 100;

				static const int TopologyColumns[1] = {TopologyID};

				static const int HostID = 200;

				static const int HostColumns[1] = {HostID};

				static const int PodID = 300;
				static const int PodHostID = 301;

				static const int TablePodColumns[2] = {PodID, PodHostID};

				static const int PodColumnPodID = 400;
				static const int PodColumnColumnID = 401;

				static const int PodColumnColumns[2] = {PodColumnPodID, PodColumnColumnID};

				static const int GeneratorNextValue = 10000;

				static const int GeneratorColumns[1] = {GeneratorNextValue};
			};
		}
	}
}
