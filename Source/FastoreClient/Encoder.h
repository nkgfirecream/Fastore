#pragma once

#include "ColumnDef.h"
#include <string>
#include <stdexcept>
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
			class Encoder
			{
			public:
				static unsigned char *WriteString(const std::string &item);

				static std::string ReadString(unsigned char item[]);

				static unsigned char *WriteBool(bool item);

				static Nullable<bool> ReadBool(unsigned char item[]);

				static unsigned char *WriteLong(long long item);

				static Nullable<long long> ReadLong(unsigned char item[]);

				static unsigned char *WriteInt(int item);

				static Nullable<int> ReadInt(unsigned char item[]);

				static unsigned char *Encode(const boost::shared_ptr<object> &item);

				static boost::shared_ptr<object> Decode(unsigned char item[], const std::string &type);
			};
		}
	}
}
