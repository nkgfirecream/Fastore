#pragma once

#include "../typedefs.h"
#include "../Schema/scalar.h"
#include "../Range.h"

using namespace fs;

struct GetResult
{
	GetResult() : Limited(false), BeginOfRange(false), EndOfRange(false) {}
	//You could derive Limited based off the EOF and BOF flags and the knowledge
	//of which direction you were querying, but I decided to make it explicit for now.
	//Also, there's a bit of a mismatch here as well with regards to dataphor. Dataphor only cares 
	//about EOF and BOF, and not the end of range or beginning of ranges. It only uses ranges for
	//searching/buffering, and doesn't yet recognize what a specific range means.
	bool Limited;
	bool BeginOfRange;
	bool EndOfRange;
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
};
