#include "optional.h"
#include <EASTL\vector.h>


struct RangeBound
{
	RangeBound(void* value, optional<void*> rowId, bool inclusive) :
		Value(value), RowId(rowId), Inclusive(inclusive) {}
		
	void* Value;
	bool Inclusive;
	optional<void*> RowId;
};

struct Range
{
	const static int MaxLimit = 50;

	Range(int limit, optional<RangeBound> start, optional<RangeBound> end, bool ascending = true):
		Limit(limit), Start(start), End(end), Ascending(ascending) {}	

	int Limit;
	bool Ascending;

	optional<RangeBound> Start;
	optional<RangeBound> End;
};

struct GetResult
{
	bool Limited;
	eastl::vector<eastl::pair<void*,void*>> Data;
};