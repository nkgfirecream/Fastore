#pragma once

#include <memory>
#include "IDataAccess.h"

namespace fastore 
{
	namespace provider
	{
		class Transaction: IDataAccess
		{
			std::shared_ptr<Database> _database;
			//std::shared_ptr<fastore::client::Transaction> _clientTransaction;
		public:
			Transaction(Database *database);

			Cursor prepare(const std::string &sql) override;
			
			void commit(bool flush = false);
			void rollback();
		};

		typedef std::shared_ptr<Transaction> TransactionObject; 
		typedef TransactionObject * PTransactionObject;
	}
}
