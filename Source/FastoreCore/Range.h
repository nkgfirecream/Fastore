#pragma once

#include "optional.h"
#include <EASTL\vector.h>

namespace fs
{
	struct RangeBound
	{
		RangeBound() : Value(NULL), Inclusive(true) {}

		RangeBound(void* value, Optional<void*> rowId = NULL, bool inclusive = true) :
			Value(value), RowId(rowId), Inclusive(inclusive) {}
		
		void* Value;
		bool Inclusive;
		Optional<void*> RowId;
	};

	struct Range
	{
		const static int MaxLimit = 500;

		Range(int limit = MaxLimit, Optional<RangeBound> start = Optional<RangeBound>(), Optional<RangeBound> end = Optional<RangeBound>(), bool ascending = true):
			Limit(limit), Start(start), End(end), Ascending(ascending) {}	

		int Limit;
		bool Ascending;

		Optional<RangeBound> Start;
		Optional<RangeBound> End;
	};
}
