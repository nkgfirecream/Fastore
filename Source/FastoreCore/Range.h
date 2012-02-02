#include "optional.h"
#include <EASTL\vector.h>


struct RangeBound
{
	RangeBound(void* value = NULL, optional<void*> rowId = NULL, bool inclusive = true) :
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
