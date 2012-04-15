#pragma once
#include "typedefs.h"
#include "IDataAccess.h"
#include "Host.h"

using namespace fs;

class Transaction : public IDataAccess
{
	public:
		Transaction(Host host): IDataAccess(host) {}
		void Dispose();
		void Commit();
};