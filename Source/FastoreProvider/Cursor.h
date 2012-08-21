#pragma once
#include "Table.h"
#include <string>

namespace fastore
{
	namespace module
	{
		class Cursor
		{
			public:
				Cursor(fastore::module::Table* table);

				void next();
				int eof();
				void close();
				std::string column(int index);
				std::string rowId();

			private:
				fastore::module::Table* _table;
		};
	}
}