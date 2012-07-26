#include "Client.h"

namespace fastore
{
	boost::shared_ptr<Database> Client::Connect(ServiceAddress addresses[])
	{
		return boost::shared_ptr<Database>(new Database(addresses));
	}
}
