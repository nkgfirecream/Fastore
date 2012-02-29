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


class ColumnBuffer
{
	public:
		virtual ValueVector GetValues(KeyVector rowIds) = 0;
		virtual Value Include(Value value, Key rowID) = 0;
		virtual Value Exclude(Value value, Key rowID) = 0;
		virtual ValueKeysVectorVector GetSorted(KeyVectorVector keyvalues) = 0; 
		virtual GetResult GetRows(Range) = 0;
};
