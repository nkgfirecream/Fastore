#include "Client.h"

Database Client::Connect(Host host)
{
	return Database(host);
}