#pragma once

#include "../typedefs.h"
#include "../Schema/scalar.h"
#include "../Range.h"

using namespace fs;

struct GetResult
{
	GetResult() : Limited(false) {}
	bool Limited;
	ValueKeysVector Data;
};

struct Statistics
{
	Statistics() : Total(0), Unique(0) {}
	Statistics(long long total, long long unique) : Total(total), Unique(unique) {}
	long long Total;
	long long Unique;
};


class IColumnBuffer
{
	public:
		virtual ValueVector GetValues(const KeyVector& rowIds) = 0;
		virtual Value GetValue(Key rowID) = 0;
		virtual bool Include(Value value, Key rowID) = 0;
		virtual bool Exclude(Value value, Key rowID) = 0;
		virtual bool Exclude(Key rowID) = 0;

		//Eventually will need a sort order. Now just assume natural ordering and reverse if necessary.
		virtual ValueKeysVectorVector GetSorted(const KeyVectorVector &keyValues) = 0; 
		virtual GetResult GetRows(Range &range) = 0;
		virtual Statistics GetStatistics() = 0;


		//TODO: Either get rid of these as we refactor, or push them to the base class to reduce the number of virtual calls.
		virtual ScalarType GetRowIDType() = 0;
		virtual ScalarType GetValueType() = 0;
		virtual fs::string GetName() = 0;
		virtual bool GetUnique() = 0;
		virtual bool GetRequired() = 0;
		virtual int GetID() = 0;
};
