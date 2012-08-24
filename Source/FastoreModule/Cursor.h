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

				void next();
				int eof();
				void close();
				void setColumnResult(sqlite3_context *pContext, int index);
				void setRowId(sqlite3_int64 *pRowid);
				void filter(int idxNum, const char *idxStr, int argc, sqlite3_value **argv);

			private:
				fastore::module::Table* _table;
				client::RangeSet _set;
				client::Range _range;

				int _index;
				void getNextSet();
		};
	}
}