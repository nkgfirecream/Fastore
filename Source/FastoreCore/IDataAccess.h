#pragma once
#include "typedefs.h"
#include "Table\dataset.h"
#include "Range.h"


using namespace fs;

class IDataAccess
{
	public:
		DataSet GetRange(int columns[], Range range /*, [sorting]*/);
		DataSet GetRows(void* rowdIds[], int columns[]  /*, sorting */);
		void* Include(void* row[], int columns[], bool isPicky);
		void Exclude (void* rowIds[], int columns[], bool isPicky);
		//void Exclude(range, columns, isPicky);
};
