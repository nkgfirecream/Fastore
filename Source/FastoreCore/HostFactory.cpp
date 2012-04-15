#include "HostFactory.h"

Host HostFactory::Create(Topology topo)
{
	Host host;

	for (int i = 0; i < topo.size(); i++)
	{
		host.CreateColumn(topo[i]);
	}

	return host;
}