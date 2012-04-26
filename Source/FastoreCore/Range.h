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

		Range(fs::wstring column, int limit = MaxLimit, Optional<RangeBound> start = Optional<RangeBound>(), Optional<RangeBound> end = Optional<RangeBound>()):
			Limit(limit), Start(start), End(end), Column(column) {}	

		int Limit;
		fs::wstring Column;

		Optional<RangeBound> Start;
		Optional<RangeBound> End;


	};
}
