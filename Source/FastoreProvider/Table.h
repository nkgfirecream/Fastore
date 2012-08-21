#pragma once
#include "Connection.h"

namespace provider = fastore::provider;
namespace client = fastore::client;

namespace fastore
{
	namespace provider
	{
		class Connection;
	}
}

namespace fastore
{
	namespace module
	{
		class Table
		{
		private:
			boost::shared_ptr<client::Transaction> _transaction;
			provider::Connection* _connection;
			std::string _name;
			std::vector<client::ColumnDef> _columns;

		public:
			Table(provider::Connection* connection, std::string name, std::vector<client::ColumnDef> columns);

			//Transaction processing...
			void begin();
			void sync();
			void commit();
			void rollback();

			//Destroy backing store
			void drop();

			//Probably a no-op for now
			void disconnect();

			//Create columns
			void create();

			//Ensure tables exists in backing store. If not, error. Also, update our columns definitions for the ids we pull.
			void connect();

			//Cursor operations: get range, etc.
			
		};
	}
}
		