#pragma once

#include "../Column/IColumnBuffer.h"
#include "../Schema/tuple.h"
#include "ranges.h"
#include "dataset.h"

class Table
{
		typedef eastl::vector<int> ColumnNumbers;

		TupleType _type; 
		ScalarType _rowType;
		eastl::vector<IColumnBuffer*> _buffers;	

		TupleType ColumnsTupleType(ColumnNumbers selection);
	public:		

		Table(const TupleType& type);

		const TupleType& getType() { return _type; };

		//Gets a single row by ID
		DataSet	GetRow(void* rowID, const ColumnNumbers outputColumns);

		// Sorts and groups the given rowIDs by value
		//ValuesRows GetSorted(eastl::vector<void*> rowIDs, const Ranges& ranges);

		// Gets a set of rows given a range
		DataSet GetRows(const Ranges& ranges, const ColumnNumbers outputColumns);

		// Replaces a given range of rows with the given values
		DataSet Include(const Ranges& ranges, const DataSet newData, const ColumnNumbers inputColumns);

		// Removes the given range of rows
		DataSet Exclude(const Ranges& ranges);

		// TODO: Vacuum
};