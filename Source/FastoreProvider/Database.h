#pragma once
#include "..\FastoreClient\Client.h"
#include "Address.h"
#include <vector>
#include <string>

namespace client = fastore::client;
namespace provider = fastore::provider;

namespace fastore
{
	namespace module
	{
		//This class is designed to be a wrapper around our client. It provides the kind of "stateful" behavior sqlite expects
		//(as opposed to the object oriented behavior our client provides by default)
		class Database
		{
		private:
			boost::shared_ptr<client::Database> _client;
			boost::shared_ptr<client::Transaction> _transaction;
		public:
			Database(std::vector<provider::Address> addresses)
			{
				std::vector<client::ServiceAddress> _sas;
				for (auto a : addresses)
				{
					client::ServiceAddress sa;
					sa.Name = a.Name;
					sa.Port = a.Port;
					_sas.push_back(sa);
				}

				_client = client::Client::Connect(_sas);
			}

			void begin()
			{
				//No isolation for now... fix this later...
				_transaction = _client->Begin(false, false);
			}

			void commit()
			{				
				if (_transaction != NULL)
					_transaction->Commit();

				_transaction.reset();
			}

			void sync()
			{
				//Do nothing for now... We do a two stage commit during our commit.
			}

			void rollback()
			{
				if (_transaction != NULL)
					_transaction->Rollback();

				_transaction.reset();
			}

			void createTable(std::vector<client::ColumnDef> defs)
			{

			}

			void dropTable(std::vector<client::ColumnDef> defs)
			{

			}

			std::vector<client::ColumnDef> connectTable(std::string table)
			{
				std::vector<client::ColumnDef> defs;
				//TODO: pull defs from database
				return defs;
			}
		};
	}
}