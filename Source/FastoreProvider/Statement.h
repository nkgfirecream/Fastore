#pragma once

#include <sqlite3.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include "fastore.h"
#include "../FastoreCommon/Utility/LexCompare.h"
#include <boost/optional.hpp>

namespace fastore 
{
	namespace provider
	{
		struct Argument
		{
			ArgumentType type;
			std::string value;
		};

		struct ColumnInfo
		{
			ArgumentType physicalType;
			std::string logicalType;
			std::string name;
		};

		class Statement
		{
		private:
			sqlite3_stmt* _statement;
			bool _eof;

			std::map<int, ColumnInfo> _infos;
			static std::map<std::string, ArgumentType, LexCompare> Types;
			static void EnsureTypes();

			void internalBind(int32_t index);
			void checkBindResult(int result);
		public:
			Statement(sqlite3* db, const std::string &sql);
			~Statement();

			int columnCount();
			void bindInt64(int32_t index, int64_t value);
			void bindDouble(int32_t index, double value);
			void bindAString(int32_t index, std::string value);
			void bindWString(int32_t index, std::wstring value);
			bool next();
			bool eof();
			void reset();
			ColumnInfo getColumnInfo(int32_t index);
			boost::optional<int64_t> getColumnValueInt64(int32_t index);
			boost::optional<double> getColumnValueDouble(int32_t index);
			boost::optional<std::string> getColumnValueAString(int32_t index);
			boost::optional<std::wstring> getColumnValueWString(int32_t index);
		};

		typedef std::shared_ptr<Statement> StatementObject; 
		typedef StatementObject* PStatementObject;
	}
}
