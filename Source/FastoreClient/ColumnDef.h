#pragma once

#include <string>
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
			enum class BufferType
			{
				Identity = 0,
				Unique = 1,
				Multi = 2
			};

			class ColumnDef
			{
					private:
						int privateColumnID;
					public:
						const int &getColumnID() const;
						void setColumnID(const int &value);
					private:
						std::string privateName;
					public:
						const std::string &getName() const;
						void setName(const std::string &value);
					private:
						std::string privateType;
					public:
						const std::string &getType() const;
						void setType(const std::string &value);
					private:
						std::string privateIDType;
					public:
						const std::string &getIDType() const;
						void setIDType(const std::string &value);
					private:
						Alphora::Fastore::Client::BufferType privateBufferType;
					public:
						const Alphora::Fastore::Client::BufferType &getBufferType() const;
						void setBufferType(const Alphora::Fastore::Client::BufferType &value);
			};
		}
	}
}
