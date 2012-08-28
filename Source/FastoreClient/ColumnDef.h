#pragma once
#include <string>

namespace fastore 
{ 
	namespace client
	{
		enum class BufferType_t
		{
			Identity = 0,
			Unique = 1,
			Multi = 2
		};

		class ColumnDef
		{
		public:
			int ColumnID;
			std::string Name;
			std::string Type;
			std::string IDType;
			BufferType_t BufferType;
			bool Required;
		};
	}
}
