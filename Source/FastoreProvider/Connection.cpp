#include "Connection.h"

void checkSQLiteResult(int sqliteResult, sqlite3 *sqliteConnection)
{
	// TODO: thread safety
	if (SQLITE_OK != sqliteResult)
	{
		throw exception(sqlite3_errmsg(sqliteConnection));
	}
}

Connection::Connection(vector<ServerAddress> addresses)
{
	_sqliteConnection = nullptr;
	//_database = unique_ptr<fastore::Database>(new fastore::Database(addresses));
	checkSQLiteResult(sqlite3_open(":memory:", &_sqliteConnection), _sqliteConnection);
}

Connection::~Connection()
{
	if (_sqliteConnection)
	{
		sqlite3_close(_sqliteConnection);	// Ignore error on closing
		_sqliteConnection = nullptr;
	}
}
