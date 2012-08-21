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
#include "..\FastoreClient\Client.h"
#include "..\FastoreClient\Generator.h"

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

	std::vector<client::ServiceAddress> sas;
	for (auto a : addresses)
	{
		client::ServiceAddress sa;
		sa.Name = a.Name;
		sa.Port = a.Port;
		sas.push_back(sa);
	}

	_database = client::Client::Connect(sas);
	_generator = boost::shared_ptr<client::Generator>(new client::Generator(_database, communication::ColumnIDs()));

	// Register our module
	checkSQLiteResult(sqlite3_create_module_v2(sqliteConnection, SQLITE_MODULE_NAME, &fastoreModule, this, nullptr), sqliteConnection);
}

unique_ptr<Statement> Connection::execute(const std::string &sql)
{
	return unique_ptr<Statement>(new Statement((this->_sqliteConnection).get(), sql));
}
