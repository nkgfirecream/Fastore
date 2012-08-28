#pragma once

#include <memory>
#include "IDataAccess.h"


#if defined(__GNUC__) &&  __GNUC__ < 5 &&  __GNUC_MINOR__ < 7
// override keyword not supported 
# define override
#endif

namespace fastore 
{
	namespace provider
	{
		class Database;

		class Transaction: IDataAccess
		{
			std::shared_ptr<Database> _database;
			//std::shared_ptr<fastore::client::Transaction> _clientTransaction;
		public:
			Transaction(Database *database);

			std::unique_ptr<Cursor> prepare(const std::string &sql) override;
			
			void commit(bool flush = false);
			void rollback();
		};

		typedef std::shared_ptr<Transaction> TransactionObject; 
		typedef TransactionObject* PTransactionObject;
	}
}
