#pragma once
#include "typedefs.h"
#include "TransactionID.h"
#include "Session.h"

using namespace fs;

class Database
{
	public:
		Session Start();
		TransactionID GetID(/* Token */);

		//Low priority
		//Topology GetTopology();
		//ILock GetLock(/*name, mode */);
};