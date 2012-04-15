#pragma once
#include "typedefs.h"
#include "TransactionID.h"
#include "Session.h"
#include "Host.h"

using namespace fs;

class Database
{

	private:
		//Some some of link to the host...
		Host _host;


	public:
		Session Start();
		TransactionID GetID(/* Token */);

		Database(Host host) : _host(host) {}

		//Low priority
		//Topology GetTopology();
		//ILock GetLock(/*name, mode */);
};