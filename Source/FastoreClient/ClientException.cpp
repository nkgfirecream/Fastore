#include "ClientException.h"

//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Collections::Generic;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Linq;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Runtime::Serialization;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Text;
namespace Alphora
{
	namespace Fastore
	{
		namespace Client
		{

			ClientException::ClientException()
			{
			}

			ClientException::ClientException(const std::string &message) : Exception(message)
			{
			}

			ClientException::ClientException(const std::string &message, Codes code) : Exception(message)
			{
				getData()->Add("Code", code);
			}

			ClientException::ClientException(const boost::shared_ptr<SerializationInfo> &info, StreamingContext context) : Exception(info, context)
			{
			}

			ClientException::ClientException(const std::string &message, std::exception &innerException) : Exception(message, innerException)
			{
			}

			void ClientException::ThrowErrors(std::vector<std::exception> &errors)
			{
				if (errors.size() > 0)
				{
					if (errors.size() == 1)
						throw errors[0];
					else
						throw boost::make_shared<AggregateException>(errors);
				}
			}

			void ClientException::ForceCleanup(...)
			{
				auto errors = std::vector<std::exception>();
				for (unknown::const_iterator action = actions.begin(); action != actions.end(); ++action)
					try
					{
						action();
					}
					catch (std::exception &e)
					{
						errors->Add(e);
					}
				ThrowErrors(errors);
			}
		}
	}
}
