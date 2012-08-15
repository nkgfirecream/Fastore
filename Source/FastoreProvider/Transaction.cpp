#include "Transaction.h"
#include "Database.h"

using namespace std;
using namespace fastore::provider;

Transaction::Transaction(Database *database) : _database(database)
{
}

std::unique_ptr<Cursor> Transaction::prepare(const std::string &sql)
{
	return std::unique_ptr<Cursor>(new Cursor(this, sql));
}
			
void Transaction::commit(bool flush)
{
	//_clientTransaction->commit(flush);
	//_clientTransaction.reset();
}

void Transaction::rollback()
{
	//_clientTransaction.rollback();
	//_clientTransaction.reset();
}

