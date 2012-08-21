#pragma once
#include "Database.h"

namespace fastore
{
	namespace module
	{
		class Cursor
		{
			public:
				Cursor(fastore::module::Database* database);

				void next();
				int eof();
				void close();
				std::string column(int index);
				std::string rowId();

			private:
				fastore::module::Database* _database;
		};
	}
}