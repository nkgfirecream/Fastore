#include "Client.h"

using namespace fastore::client;

boost::shared_ptr<Database> Client::Connect(std::vector<ServiceAddress> addresses)
{
	return boost::shared_ptr<Database>(new Database(addresses));
}

