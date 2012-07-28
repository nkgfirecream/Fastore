#pragma once

#include <string>
#include <memory>
#include "IDataAccess.h"

namespace fastore 
{
	namespace provider
	{
		struct Argument
		{
			ArgumentTypes type;
			std::string value;
		};

		struct ColumnInfo
		{
			std::string type;
			std::string value;
		};

		class Cursor
		{
			shared_ptr<IDataAccess> _dataAccess;
		public:
			Cursor(IDataAccess *dataAccess, const std::string &sql);

			int columnCount();
			void bind(std::vector<Argument> arguments);
			bool next();
			ColumnInfo getColumnInfo(int index);
			string getColumn(int index);
		};

		typedef std::shared_ptr<Cursor> TransactionObject; 
		typedef TransactionObject * PTransactionObject;
	}
}
