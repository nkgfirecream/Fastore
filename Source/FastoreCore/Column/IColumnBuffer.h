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


		//TODO: Either get rid of these as we refactor, or push them to the base class to reduce the number of virtual calls.
		virtual ScalarType GetRowType() = 0;
		virtual ScalarType GetKeyType() = 0;
		virtual fs::wstring GetName() = 0;
		virtual bool GetUnique() = 0;
		virtual bool GetRequired() = 0;
};
