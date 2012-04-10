#pragma once
#include "Transaction.h"
#include "IDataAccess.h"

using namespace fs;

class Session : public IDataAccess
{
	public:
		void Dispose();
		Transaction Begin(bool ReadIsolation, bool WriteIsolation);
};
