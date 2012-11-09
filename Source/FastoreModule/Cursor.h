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
				bool _needsReset;
				std::vector<std::string> _lastValues;
				std::string _lastIdxString;
				int _lastIdxNum;

				client::Range createRange(bool ascending, int colIndex, std::string& idxStr, std::vector<std::string>& values);
				client::RangeBound getBound(int col, char op, std::string& boundValue);
				std::string convertSqliteValueToString(int col, sqlite3_value* arg);
				bool compareVectors(std::vector<std::string> left, std::vector<std::string> right);
				std::vector<std::string> getVector(int columnIndex, int count, sqlite3_value **args);
		};
	}
}
