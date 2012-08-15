#pragma once

#include <string>
#include <memory>
#include <vector>
#include "IDataAccess.h"
#include "ArgumentType.h"

namespace fastore 
{
	namespace provider
	{
		class IDataAccess;

		struct Argument
		{
			ArgumentType type;
			std::string value;
		};

		struct ColumnInfo
		{
			std::string type;
			std::string value;
		};

		class Cursor
		{
		private:
			std::shared_ptr<fastore::provider::IDataAccess> _dataAccess;
		public:
			Cursor(fastore::provider::IDataAccess *dataAccess, const std::string &sql);

			int columnCount();
			void bind(std::vector<Argument> arguments);
			bool next();
			ColumnInfo getColumnInfo(int index);
			std::string getColumn(int index);
		};

		typedef std::shared_ptr<Cursor> CursorObject; 
		typedef CursorObject* PCursorObject;
	}
}
