#pragma once
#include <sqlite3.h>
#include "fastore.h"
#include "Statement.h"
#include "Address.h"
#include "Database.h"


namespace client = fastore::client;
namespace module = fastore::module;

namespace fastore 
{
	namespace provider
	{
		class Connection
		{

		private:
			std::shared_ptr<sqlite3> _sqliteConnection;
			std::shared_ptr<module::Database> _database;
		public:
			Connection(std::vector<Address> addresses);
			std::unique_ptr<Statement> execute(const std::string &sql);

		private:

		};

		typedef std::shared_ptr<Connection> ConnectionObject; 
		typedef ConnectionObject * PConnectionObject;
	}
}

