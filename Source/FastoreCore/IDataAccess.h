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
	public:
		IDataAccess(){}
		IDataAccess(Host host): _host(host) {}


		DataSet GetRange(eastl::vector<fs::wstring> columns, Range range /*, [sorting]*/);
		DataSet GetRows(eastl::vector<void*> rowdIds, eastl::vector<fs::wstring> columns  /*, sorting */);
		void* Include(eastl::vector<void*> row, eastl::vector<fs::wstring> columns, bool isPicky);
		void Include(void* rowID, eastl::vector<void*> row, eastl::vector<fs::wstring> columns, bool isPicky);
		void Exclude(eastl::vector<void*> rowIds, eastl::vector<fs::wstring> columns, bool isPicky);
		//void Exclude(range, columns, isPicky);
};
