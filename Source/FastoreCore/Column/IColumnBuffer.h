#pragma once

#include "../typedefs.h"
#include "../Range.h"

using namespace fs;

struct GetResult
{
	GetResult() : Limited(false) {}
	bool Limited;
	ValueKeysVector Data;
};


class IColumnBuffer
{
	public:
		virtual ValueVector GetValues(KeyVector rowIds) = 0;
		virtual bool Include(Value value, Key rowID) = 0;
		virtual bool Exclude(Value value, Key rowID) = 0;
		virtual ValueKeysVectorVector GetSorted(KeyVectorVector keyvalues) = 0; 
		virtual GetResult GetRows(Range) = 0;
};
