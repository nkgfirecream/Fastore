#include "Client.h"

namespace fastore
{
	boost::shared_ptr<Database> Client::Connect(ServiceAddress addresses[])
	{
		return boost::make_shared<Database>(addresses);
	}
}
