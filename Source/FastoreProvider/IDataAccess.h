#pragma once

#include <string>
#include "../FastoreModule/Cursor.h"

namespace fastore 
{
	namespace provider
	{
		//Circular dependence on Cursor..
		class Cursor;

		class IDataAccess
		{
		public:
			virtual std::unique_ptr<Cursor> prepare(const std::string &sql) = 0;
		};
	}
}
