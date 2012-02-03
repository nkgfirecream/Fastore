#pragma once

#include "Optional.h"
#include <EASTL\vector.h>


struct RangeBound
{
	RangeBound(void* value = NULL, Optional<void*> rowId = NULL, bool inclusive = true) :
		Value(value), RowId(rowId), Inclusive(inclusive) {}
		
	void* Value;
	bool Inclusive;
	Optional<void*> RowId;
};

struct Range
{
	const static int MaxLimit = 50;

	Range(int limit, Optional<RangeBound> start, Optional<RangeBound> end, bool ascending = true):
		Limit(limit), Start(start), End(end), Ascending(ascending) {}	

	int Limit;
	bool Ascending;

	Optional<RangeBound> Start;
	Optional<RangeBound> End;
};
