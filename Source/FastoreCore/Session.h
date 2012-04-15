#pragma once
#include "Transaction.h"
#include "IDataAccess.h"
#include "Host.h"

using namespace fs;

class Session : public IDataAccess
{
	private:
		//TODO: Abstraction... 
	

	public:
		void Dispose();
		Transaction Begin(bool ReadIsolation, bool WriteIsolation);
		Session() {}
		Session(Host host): IDataAccess(host) {}
};
