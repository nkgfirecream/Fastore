#pragma once

#include <sqlite3.h>
#include <vector>
#include <memory>
#include "fastore.h"

using namespace std;

struct ServerAddress
{
	string hostName;
	int port;
};

struct Database
{
private:
	sqlite3 *_sqliteConnection;
	//shared_ptr<fastore::Database> _database;
public:
	Database(vector<ServerAddress> addresses);
	~Database();
};
