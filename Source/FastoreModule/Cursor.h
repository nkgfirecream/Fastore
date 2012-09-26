#pragma once
#include "Table.h"

namespace fastore
{
	namespace module
	{
		class Cursor
		{
			public:
				Cursor(fastore::module::Table* table);

				int next();
				int eof();
				int close();
				int setColumnResult(sqlite3_context *pContext, int index);
				int setRowId(sqlite3_int64 *pRowid);
				int filter(int idxNum, const char *idxStr, int argc, sqlite3_value **argv);

			private:
				fastore::module::Table* _table;
				client::RangeSet _set;
				client::Range _range;

				int _index;
				void getNextSet();
				client::Range createRange(int idxNum, const char *idxStr, int argc, sqlite3_value **argv);
				client::RangeBound getBound(int col, char op, sqlite3_value* arg);
				std::string convertSqliteValueToString(int col, sqlite3_value* arg);
		};
	}
}
