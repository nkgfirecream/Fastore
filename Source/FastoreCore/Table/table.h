#pragma once

#include <memory>
#include "../Schema/tuple.h"
#include "../Column/columnbuffer.h"
#include "../Column/ColumnHash.h"
#include "../typedefs.h"
#include "ranges.h"
#include "dataset.h"

class Table
{
		TupleType _type; 
		ScalarType _rowType;
		eastl::vector<std::unique_ptr<ColumnBuffer>> _buffers;	

		TupleType SelectionTupleType(SelectionType selection);
	public:
		typedef eastl::vector<int> SelectionType;

		Table(const TupleType& type);

		const TupleType& getType() { return _type; };

		// Gets a single row by ID
		DataSet	GetRow(void* rowID, const SelectionType selection);

		// Sorts and groups the given rowIDs by value
		//ValuesRows GetSorted(eastl::vector<void*> rowIDs, const Ranges& ranges);

		// Gets a set of rows given a range
		DataSet GetRows(const Ranges& ranges, const SelectionType selection);

		// Replaces a given range of rows with the given values
		DataSet Include(const Ranges& ranges, const DataSet newData, const SelectionType selection);

		// Removes the given range of rows
		DataSet Exclude(const Ranges& ranges);

		// TODO: Vacuum
};