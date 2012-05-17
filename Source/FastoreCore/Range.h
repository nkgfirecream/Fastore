#pragma once

#include "optional.h"
#include "typedefs.h"

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
		Range(int limit, bool ascending, Optional<RangeBound> start = Optional<RangeBound>(), Optional<RangeBound> end = Optional<RangeBound>()):
			Limit(limit), Start(start), End(end), Ascending(ascending) {}	

		int Limit;
		bool Ascending;

		Optional<RangeBound> Start;
		Optional<RangeBound> End;


	};
}
