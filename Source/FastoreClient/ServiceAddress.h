#pragma once

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
			class ServiceAddress
			{
			public:
				static const int DefaultPort = 8765;

					private:
						int privatePort;
					public:
						const int &getPort() const;
						void setPort(const int &value);
					private:
						std::string privateName;
					public:
						const std::string &getName() const;
						void setName(const std::string &value);

				static boost::shared_ptr<ServiceAddress> ParseOne(const std::string &address);

				static ServiceAddress *ParseList(const std::string &composite);
			};
		}
	}
}
