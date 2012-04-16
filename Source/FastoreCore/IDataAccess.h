#pragma once
#include "typedefs.h"
#include "Table\dataset.h"
#include "Range.h"
#include <EASTL\vector.h>
#include "Host.h"


using namespace fs;

class IDataAccess
{
	protected: 
		Host _host;
		//need multiple id generators to handle multiple tables.
		long long _currentID;
	public:
		IDataAccess()
		{ 
			_currentID = 0;
		}

		IDataAccess(Host host): _host(host)
		{
			_currentID = 0;
		}


		DataSet GetRange(eastl::vector<fs::wstring> columns, Range range /*, [sorting]*/);
		DataSet GetRows(eastl::vector<void*> rowdIds, eastl::vector<fs::wstring> columns  /*, sorting */);
		long long Include(eastl::vector<void*> row, eastl::vector<fs::wstring> columns, bool isPicky);
		void Include(void* rowID, eastl::vector<void*> row, eastl::vector<fs::wstring> columns, bool isPicky);
		void Exclude(eastl::vector<void*> rowIds, eastl::vector<fs::wstring> columns, bool isPicky);
		//void Exclude(range, columns, isPicky);
};
