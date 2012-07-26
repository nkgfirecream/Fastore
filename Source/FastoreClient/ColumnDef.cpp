#include "ColumnDef.h"

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

			const int &ColumnDef::getColumnID() const
			{
				return privateColumnID;
			}

			void ColumnDef::setColumnID(const int &value)
			{
				privateColumnID = value;
			}

			const std::string &ColumnDef::getName() const
			{
				return privateName;
			}

			void ColumnDef::setName(const std::string &value)
			{
				privateName = value;
			}

			const std::string &ColumnDef::getType() const
			{
				return privateType;
			}

			void ColumnDef::setType(const std::string &value)
			{
				privateType = value;
			}

			const std::string &ColumnDef::getIDType() const
			{
				return privateIDType;
			}

			void ColumnDef::setIDType(const std::string &value)
			{
				privateIDType = value;
			}

			const Alphora::Fastore::Client::BufferType &ColumnDef::getBufferType() const
			{
				return privateBufferType;
			}

			void ColumnDef::setBufferType(const Alphora::Fastore::Client::BufferType &value)
			{
				privateBufferType = value;
			}
		}
	}
}
