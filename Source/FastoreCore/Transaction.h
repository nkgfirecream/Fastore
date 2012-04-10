#pragma once
#include "typedefs.h"
#include "IDataAccess.h"

using namespace fs;

class Transaction : public IDataAccess
{
	public:
		void Dispose();
		void Commit();
};