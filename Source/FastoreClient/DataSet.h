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
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Collections;

namespace Alphora
{
	namespace Fastore
	{
		namespace Client
		{
			class DataSet : public IEnumerable<DataSet::DataSetRow>
			{
			public:
				class DataSetRow
				{
				public:
//ORIGINAL LINE: public object[] Values;
//C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
					object *Values;
					boost::shared_ptr<object> ID;
				};
			private:
//ORIGINAL LINE: private DataSetRow[] _rows;
//C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
				DataSetRow *_rows;
				int _columnCount;

			public:
				DataSet(int rows, int columnCount);

				DataSetRow &operator [](int index);

				const int &getCount() const;

				boost::shared_ptr<IEnumerator<DataSetRow>> GetEnumerator();

				boost::shared_ptr<IEnumerator> IEnumerable_GetEnumerator();

			};


		}
	}
}
