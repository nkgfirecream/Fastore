#pragma once
#include "..\FastoreClient\Client.h"
#include "..\FastoreClient\Generator.h"
#include "Address.h"

namespace fastore
{
	namespace module
	{
		class Connection
		{
		public:
			boost::shared_ptr<client::Database> _database;
			boost::shared_ptr<client::Generator> _generator;

			Connection(std::vector<Address>& addresses)
			{
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
			}
		};
	}
}