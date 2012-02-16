#include "table.h"
#include "../Column/columnbuffer.h"
#include "../range.h"

Table::Table(const TupleType& type) : _type(type), _rowType(GetLongType()) 
{
	for (TupleType::iterator it = _type.begin(); it != _type.end(); ++it)
	{
		_buffers.push_back(std::unique_ptr<ColumnBuffer>(new ColumnHash(_rowType, (*it).Type))); 
	}
}

DataSet Table::GetRow(void* rowID, const Selection selection)
{
	
}

TupleType Table::SelectionTupleType(Selection selection)
{
	ColumnTypeVector result = ColumnTypeVector();
	for (int i = 0; i < selection.size(); i++)
		result.push_back(_type[selection[i]]);
	return TupleType(result);
}

DataSet Table::GetRows(const Ranges& ranges, const Selection selection)
{
	Range range;
	int rangeColumn;
	if (ranges.size() == 0)
	{
		// Attempt to find a unique column
		bool found = false;
		for (int i = 0; i < _type.size(); ++i)
			if (_type[i].IsUnique)
			{
				range = Range();
				rangeColumn = i;
				found = true;
				break;
			}

		// Default to the first column
		if (!found)
		{
			range = Range();
			rangeColumn = 0;
		}
	}
	else
	{
		// TODO: Multi-column ranges - for now just taking the first
		range = ranges[0].Range;
		rangeColumn = ranges[0].ColumnNumber;
	}

	GetResult result = (*_buffers[rangeColumn]).GetRows(range);

	TupleType resultType = SelectionTupleType(selection);
	DataSet results = DataSet(resultType, result.Data.size());

	// ...

	return results;
}

DataSet Table::Include(const Ranges& ranges, const DataSet newData, const Selection selection)
{
	//
}

DataSet Table::Exclude(const Ranges& ranges)
{
	//
}
