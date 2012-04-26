#include "HostFactory.h"

Host HostFactory::Create(Topology topo)
{
	Host host;

	for (unsigned int i = 0; i < topo.size(); i++)
	{
		host.CreateColumn(topo[i]);
	}

	return host;
}