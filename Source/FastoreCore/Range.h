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
		const static int MaxLimit = 500;

		Range(const int& columnId, const int& limit = MaxLimit, Optional<RangeBound> start = Optional<RangeBound>(), Optional<RangeBound> end = Optional<RangeBound>()):
			Limit(limit), Start(start), End(end), ColumnID(columnId) {}	

		int Limit;
		int ColumnID;
		bool Ascending;

		Optional<RangeBound> Start;
		Optional<RangeBound> End;


	};
}
