#include "Encoder.h"

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

			unsigned char *Encoder::WriteString(const std::string &item)
			{
				return Encoding::UTF8->GetBytes(item);
			}

			std::string Encoder::ReadString(unsigned char item[])
			{
				if (sizeof(item) / sizeof(item[0]) > 0)
					return Encoding::UTF8->GetString(item);
				else
					return "";
			}

			unsigned char *Encoder::WriteBool(bool item)
			{
				return BitConverter::GetBytes(item);
			}

			Nullable<bool> Encoder::ReadBool(unsigned char item[])
			{
				if (sizeof(item) / sizeof(item[0]) > 0)
					return BitConverter::ToBoolean(item, 0);
				else
					return nullptr;
			}

			unsigned char *Encoder::WriteLong(long long item)
			{
				return BitConverter::GetBytes(item);
			}

			Nullable<long long> Encoder::ReadLong(unsigned char item[])
			{
				if (sizeof(item) / sizeof(item[0]) > 0)
					return BitConverter::ToInt64(item, 0);
				else
					return nullptr;
			}

			unsigned char *Encoder::WriteInt(int item)
			{
				return BitConverter::GetBytes(item);
			}

			Nullable<int> Encoder::ReadInt(unsigned char item[])
			{
				if (sizeof(item) / sizeof(item[0]) > 0)
					return BitConverter::ToInt32(item, 0);
				else
					return nullptr;
			}

			unsigned char *Encoder::Encode(const boost::shared_ptr<object> &item)
			{
				//TODO: Fix this hack! Nulls should not be and empty string
				if (item == nullptr)
					return WriteString("");

				auto type = item->GetType();
				if (type == BufferType::typeid)
					return WriteInt(static_cast<int>(item));
				if (type == int::typeid)
					return WriteInt(static_cast<int>(item));
				if (type == std::string::typeid)
					return WriteString(static_cast<std::string>(item));
				if (type == long long::typeid)
					return WriteLong(static_cast<long long>(item));
				if (type == bool::typeid)
					return WriteBool(static_cast<bool>(item));

				throw std::exception("Unsupported Type");
			}

			boost::shared_ptr<object> Encoder::Decode(unsigned char item[], const std::string &type)
			{
				boost::shared_ptr<object> toReturn = nullptr;
//C# TO C++ CONVERTER NOTE: The following 'switch' operated on a string variable and was converted to C++ 'if-else' logic:
//				switch (type)
//ORIGINAL LINE: case "Int":
				if (type == "Int")
				{
						toReturn = ReadInt(item);
				}
//ORIGINAL LINE: case "String":
				else if (type == "String")
				{
						toReturn = ReadString(item);
				}
//ORIGINAL LINE: case "Long":
				else if (type == "Long")
				{
						toReturn = ReadLong(item);
				}
//ORIGINAL LINE: case "Bool":
				else if (type == "Bool")
				{
						toReturn = ReadBool(item);
				}
				else
				{
						throw std::exception("Unsupported Type");
				}

				return toReturn;
			}
		}
	}
}
