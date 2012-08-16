#pragma once

#include <sqlite3.h>
#include <vector>
#include <memory>
#include "fastore.h"
#include "Transaction.h"
#include "IDataAccess.h"
#include "..\FastoreClient\ColumnDef.h"

namespace fastore 
{
	namespace provider
	{
		struct ServerAddress
		{
			std::string hostName;
			int port;
		};

		class Database	: IDataAccess
		{
			friend class Transaction;
			friend class Cursor;
		private:
			std::shared_ptr<sqlite3> _sqliteConnection;
			//std::shared_ptr<fastore::client::Database> _client;
		public:
			Database(std::vector<ServerAddress> addresses);

			std::unique_ptr<Cursor> prepare(const std::string &sql) override;

			std::unique_ptr<Transaction> begin();
			void commit();
			void createTable(const char* name, const std::vector<fastore::client::ColumnDef>& columns);
		};

		typedef std::shared_ptr<Database> DatabaseObject; 
		typedef DatabaseObject * PDatabaseObject;
	}
}

