#pragma once

#include <string>
#include "Cursor.h"

namespace fastore 
{
	namespace provider
	{
		class IDataAccess
		{
		public:
			virtual Cursor prepare(const std::string &sql) = 0;
		};
	}
}