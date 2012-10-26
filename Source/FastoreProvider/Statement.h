#pragma once

#include <sqlite3.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include "fastore.h"
#include "LexCompare.h"
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
			ArgumentType type;
			std::string logicalType;
			std::string name;
		};

		class Statement
		{
		private:
			sqlite3_stmt* _statement;
			void internalBind(int index, ArgumentType type, std::string& value);
			bool _eof;

			std::map<int, ColumnInfo> _infos;
			std::map<std::string, ArgumentType, LexCompare> _types;

		public:
			Statement(sqlite3* db, const std::string &sql);
			~Statement();

			int columnCount();
			void bind(std::vector<Argument> arguments);
			bool next();
			bool eof();
			void reset();
			ColumnInfo getColumnInfo(int index);
			boost::optional<int64_t> getColumnValueInt64(int index);
			boost::optional<double> getColumnValueDouble(int index);
			boost::optional<std::string> getColumnValueAString(int index);
		};

		typedef std::shared_ptr<Statement> StatementObject; 
		typedef StatementObject* PStatementObject;
	}
}
