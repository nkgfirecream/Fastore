#include "Connection.h"
#include "Statement.h"
#include "../FastoreModule/Module.h"

using namespace fastore::provider;

namespace module = fastore::module;
namespace client = fastore::client;
namespace communication = fastore::communication;

Connection::Connection(vector<Address> addresses)
{
	// Open and wrap the SQLite connection
	sqlite3 *sqliteConnection = nullptr;
	checkSQLiteResult(sqlite3_open(":memory:", &sqliteConnection), sqliteConnection);
	_sqliteConnection = shared_ptr<sqlite3>(sqliteConnection, sqlite3_close);
	std::vector<module::Address> mas;
	
	for (auto a : addresses)
	{
		module::Address ma;
		ma.Name = a.Name;
		ma.Port = a.Port;
		mas.push_back(ma);
	}

	intializeFastoreModule(_sqliteConnection.get(), mas);
}

unique_ptr<Statement> Connection::execute(const std::string &sql)
{
	return unique_ptr<Statement>(new Statement(_sqliteConnection.get(), sql));
}
