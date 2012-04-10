#pragma once
#include "Host.h"
#include "Topology.h"

using namespace fs;

class HostFactory
{
	public:
		Host Create(Topology topo);
		//Lower Priority
		//IHost Connect(address);
		
};