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
		virtual ValueVector GetValues(const KeyVector& rowIds) = 0;
		virtual bool Include(Value value, Key rowID) = 0;
		virtual bool Exclude(Value value, Key rowID) = 0;
		virtual ValueKeysVectorVector GetSorted(const KeyVectorVector &keyValues) = 0; 
		virtual GetResult GetRows(Range &range) = 0;
};
