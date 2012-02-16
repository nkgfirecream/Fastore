#pragma once

#include <memory>
#include "../Schema/tuple.h"
#include "../Column/columnbuffer.h"
#include "../Column/ColumnHash.h"
#include "../typedefs.h"
#include "ranges.h"
#include "dataset.h"
#include <vector>

class Table
{
		typedef eastl::vector<int> Selection;

		TupleType _type; 
		ScalarType _rowType;
		eastl::vector<std::unique_ptr<ColumnBuffer>> _buffers;	

		TupleType SelectionTupleType(Selection selection);
	public:		

		Table(const TupleType& type);

		const TupleType& getType() { return _type; };

		// Gets a single row by ID
		DataSet	GetRow(void* rowID, const Selection selection);

		// Sorts and groups the given rowIDs by value
		//ValuesRows GetSorted(eastl::vector<void*> rowIDs, const Ranges& ranges);

		// Gets a set of rows given a range
		DataSet GetRows(const Ranges& ranges, const Selection selection);

		// Replaces a given range of rows with the given values
		DataSet Include(const Ranges& ranges, const DataSet newData, const Selection selection);

		// Removes the given range of rows
		DataSet Exclude(const Ranges& ranges);

		// TODO: Vacuum
};