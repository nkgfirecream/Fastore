#pragma once

#include <sqlite3.h>
#include <vector>
#include <memory>

using namespace std;

struct ServerAddress
{
	string hostName;
	int port;
};

class Connection
{
	sqlite3 *_sqliteConnection;
	//shared_ptr<fastore::Database> _database;
public:
	Connection(vector<ServerAddress> addresses);
	~Connection();
};

