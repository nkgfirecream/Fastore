#pragma once
#include <sqlite3.h>
#include "fastore.h"
#include "Statement.h"
#include "Address.h"
#include "..\FastoreClient\Database.h"
#include "..\FastoreClient\Generator.h"
#include "Table.h"


namespace client = fastore::client;

namespace fastore
{
	namespace module
	{
		class Table;
	}
}

namespace fastore 
{
	namespace provider
	{
		class Connection
		{
			friend class fastore::module::Table;

		private:
			std::shared_ptr<sqlite3> _sqliteConnection;
			boost::shared_ptr<client::Database> _database;
			boost::shared_ptr<client::Generator> _generator;

		public:
			Connection(std::vector<Address> addresses);
			std::unique_ptr<Statement> execute(const std::string &sql);

		private:

		};

		typedef std::shared_ptr<Connection> ConnectionObject; 
		typedef ConnectionObject * PConnectionObject;
	}
}

