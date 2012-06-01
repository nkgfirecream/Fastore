#pragma once

#include "optional.h"
#include "typedefs.h"

namespace fs
{
	struct RangeBound
	{
		RangeBound() : Value(NULL), Inclusive(true) {}

		RangeBound(void* value,  bool inclusive = true) :
			Value(value), Inclusive(inclusive) {}
		
		void* Value;
		bool Inclusive;
	};

	struct Range
	{
		Range(int limit, bool ascending, Optional<RangeBound> start = Optional<RangeBound>(), Optional<void*> startId = Optional<void*>(), Optional<RangeBound> end = Optional<RangeBound>()):
			Limit(limit), Start(start), StartId(startId), End(end), Ascending(ascending) {}	

		int Limit;
		bool Ascending;

		Optional<RangeBound> Start;
		Optional<RangeBound> End;

		Optional<void*> StartId;
	};
}
