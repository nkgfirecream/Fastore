#pragma once

#include <sqlite3.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include "fastore.h"

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
			std::string name;
		};

		class Statement
		{
		private:
			sqlite3_stmt* _statement;
			void internalBind(int index, ArgumentType type, std::string& value);
			bool _eof;

			std::map<int, ColumnInfo> _infos;
			std::map<int, ArgumentType> _types;

		public:
			Statement(sqlite3* db, const std::string &sql);
			~Statement();

			int columnCount();
			void bind(std::vector<Argument> arguments);
			bool next();
			bool eof();
			void reset();
			ColumnInfo getColumnInfo(int index);
			std::string getColumn(int index);
		};

		typedef std::shared_ptr<Statement> StatementObject; 
		typedef StatementObject* PStatementObject;
	}
}
