#include "Transaction.h"
#include "Database.h"

using namespace std;
using namespace fastore::provider;

Transaction::Transaction(Database *database) : _database(database)
{
}

Cursor Transaction::prepare(const std::string &sql)
{
	
}
			
void Transaction::commit(bool flush)
{
	//_transaction.commit(flush);
	//_transaction.release();
}

void Transaction::rollback()
{
	//_transaction.rollback();
	//_transaction.release();
}

