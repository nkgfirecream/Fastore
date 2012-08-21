#include "Connection.h"
#include "Statement.h"
#include "Module.h"
#include <vector>
#include <sstream>
#include <exception>
#include <map>
#include <functional>
#include <algorithm>
#include <string>
#include <locale>

using namespace fastore::provider;

namespace module = fastore::module;

Connection::Connection(vector<Address> addresses)
{
	// Open and wrap the SQLite connection
	sqlite3 *sqliteConnection = nullptr;
	checkSQLiteResult(sqlite3_open(":memory:", &sqliteConnection), sqliteConnection);
	_sqliteConnection = shared_ptr<sqlite3>(sqliteConnection, sqlite3_close);

	_database = shared_ptr<module::Database>(new module::Database(addresses));

	// Register our module
	checkSQLiteResult(sqlite3_create_module_v2(sqliteConnection, SQLITE_MODULE_NAME, &fastoreModule, _database.get(), nullptr), sqliteConnection);
}

unique_ptr<Statement> Connection::execute(const std::string &sql)
{
	return unique_ptr<Statement>(new Statement((this->_sqliteConnection).get(), sql));
}
