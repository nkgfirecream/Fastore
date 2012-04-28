#pragma once
#include "typedefs.h"
#include "Table\dataset.h"
#include "Range.h"
#include "Order.h"
#include <EASTL\vector.h>
#include "Host.h"


using namespace fs;

class IDataAccess
{
	protected: 
		Host _host;
		//need multiple id generators to handle multiple tables.
	public:
		IDataAccess(Host host): _host(host) {}


		DataSet GetRange(eastl::vector<fs::wstring>& columns, eastl::vector<Order>& orders, eastl::vector<Range>& ranges);
		DataSet GetRows(eastl::vector<void*>& rowdIds, eastl::vector<fs::wstring>& columns  /*, sorting */);
		//int Include(eastl::vector<void*>& row, eastl::vector<fs::wstring>& columns, bool isPicky);
		void Include(void* rowID, eastl::vector<void*>& row, eastl::vector<fs::wstring>& columns);
		void Exclude(eastl::vector<void*>& rowIds, eastl::vector<fs::wstring>& columns);
		Statistics GetStatistics(fs::wstring column);
		//void Exclude(range, columns, isPicky);
};
