#include "Client.h"

using namespace fastore::client;

Database Client::Connect(std::vector<ServiceAddress> addresses)
{
	return Database(addresses);
}

