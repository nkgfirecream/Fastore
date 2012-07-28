#pragma once

#include <sqlite3.h>
#include <vector>
#include <memory>
#include "fastore.h"
#include "Transaction.h"
#include "IDataAccess.h"

namespace fastore 
{
	namespace provider
	{
		struct ServerAddress
		{
			std::string hostName;
			int port;
		};

		struct Database	: IDataAccess
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
		};

		typedef std::shared_ptr<Database> DatabaseObject; 
		typedef DatabaseObject * PDatabaseObject;
	}
}

