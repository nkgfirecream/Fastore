//#include "table.h"
//#include "../Column/HashBuffer.h"
//
//Table::Table(const TupleType& type) : _type(type), _rowType(standardtypes::GetLongType()) 
//{
//	for (TupleType::iterator it = _type.begin(); it != _type.end(); ++it)
//	{
//		_buffers.push_back(new HashBuffer(_rowType, (*it).Type));
//	}
//}
//
//DataSet Table::GetRow(void* rowID, const ColumnNumbers outputColumns)
//{
//	ColumnTypeVector selectedColumns;
//
//	for (int i = 0; i < outputColumns.size(); i++)
//	{
//		selectedColumns.push_back(_type[outputColumns[i]]);
//	}
//
//	TupleType rowType = TupleType(selectedColumns);
//
//	//TODO: Look up required column to determine existence
//	//TODO: What if the row doesn't exist?
//	DataSet results = DataSet(rowType, 1);
//
//	for(int i = 0; i < selectedColumns.size(); i++)
//	{
//		results.SetCell(0, i, _buffers[outputColumns[i]]->GetValue(rowID));
//	}
//
//	return results;
//}
//
//TupleType Table::ColumnsTupleType(ColumnNumbers columnNumbers)
//{
//	ColumnTypeVector result = ColumnTypeVector();
//	for (int i = 0; i < columnNumbers.size(); i++)
//		result.push_back(_type[columnNumbers[i]]);
//	return TupleType(result);
//}
//
//DataSet Table::GetRows(const Ranges& ranges, const ColumnNumbers outputColumns)
//{
//	Range range;
//	int rangeColumn;
//	if (ranges.size() == 0)
//	{
//		// Attempt to find a unique column
//		bool found = false;
//		for (int i = 0; i < _type.size(); ++i)
//			if (_type[i].IsUnique)
//			{
//				range = Range();
//				rangeColumn = i;
//				found = true;
//				break;
//			}
//
//		// Default to the first column
//		if (!found)
//		{
//			range = Range();
//			rangeColumn = 0;
//		}
//	}
//	else
//	{
//		// TODO: Multi-column ranges - for now just taking the first
//		range = ranges[0].Range;
//		rangeColumn = ranges[0].ColumnNumber;
//	}
//
//	GetResult result = (*_buffers[rangeColumn]).GetRows(range);
//
//	TupleType resultType = ColumnsTupleType(outputColumns);
//	DataSet results = DataSet(resultType, result.Data.size());
//
//	//TODO: Consider other methods of insertion
//	for(int c = 0; c < outputColumns.size(); c++)
//	{
//		if(outputColumns[c] == rangeColumn)
//		{
//			//Copy data we pulled from our range request
//			//TODO: Handle nulls values (rows where column = null will return no entries if column = rangecolumn)
//			//So does that mean we need a special value for null in our ranges?
//			for(int r = 0; r < result.Data.size(); r++)
//			{
//				results.SetCell(r, c, result.Data[r].second);
//			}
//		}
//		else
//		{
//			for(int r = 0; r < result.Data.size(); r++)
//			{
//				results.SetCell(r, c, _buffers[outputColumns[c]]->GetValue(result.Data[r].first));
//			}
//		}
//	}
//
//	return results;
//}
//
////TODO: Restore const, propagate it through the HashBuffer and BTrees
//DataSet Table::Include(const Ranges& ranges, DataSet newData, const ColumnNumbers inputColumns)
//{
//	//Ranges identify which rows we want to affect. Ranges will eventually be able to be chained
//	//to allow us to narrow the the selection.
//	
//	//newData is the data we want to insert, which looks like 
//	//row { column, column, column }
//	//row { column, column, column }
//	//etc...
//
//	//input columns are which columns in the table the DataSet columns represent.
//
//	//When inserting new rows, the newData must include an ID column so all the column buffers can have the correct ID
//	//inserted into the values.
//	//So, conceptually a new row might look like
//	// row { ID, String, NULL }
//	//and the corresponding DataSet would be
//	// row { ID, String } -- columns {0, 1}
//	//(What's the range for a new insert? Empty? What happens if we get a range AND rowIds in a DataSet?
//	//Do we only update the intersection between the two?) 
//
//
//	//When updating old rows, that isn't necessary because we are simply shifting data around.
//	// EX update where column = "foo" set column = "bar"
//	// The range will be -- range { column start "foo" end "foo" } which will give us back all the rows the have that value 
//	// that column and then we simply need to insert "bar" into column for each row
//
//	//(So, how do we map multiple entries in newData to insertion? Should there not be only one value? what does it mean to 
//	//insert "bar" and "bubba" into the range "foo"?)
//	//(Also, how do we determine if we should optimize and use updateValues? The only case that would work is one range, start = end, and one row in the dataset)
//
//
//	//TODO: result will eventually be undo information
//	DataSet result(_type, 0);
//
//	Range range;
//	int rangeColumn;
//	if (ranges.size() == 0)
//	{
//		//Empty Range = all new rows??
//		//Key column needs row ids..
//		//Key Only Btree?
//	}
//	else
//	{
//		// TODO: Multi-column ranges - for now just taking the first
//		range = ranges[0].Range;
//		rangeColumn = ranges[0].ColumnNumber;
//	}
//
//	GetResult rangeResult = (*_buffers[rangeColumn]).GetRows(range);
//	
//	for (int c = 0; c < inputColumns.size(); c++)
//	{			
//		for (int r = 0; r < rangeResult.Data.size(); r++)
//		{
//			//Remove old value
//			_buffers[inputColumns[c]]->Exclude(rangeResult.Data[r].second, rangeResult.Data[r].first);			
//		}
//	}
//	
//	//TODO: For overlapping values use UpdateValues.
//
//	for (int c = 0; c < inputColumns.size(); c++)
//	{
//		for (int r = 0; r < newData.Size(); r++)
//		{
//			//TODO: Column zero is id column for now...
//			//Since our key-only tree is in flux,
//			//the id columns stores duplicate entries.
//			_buffers[inputColumns[c]]->Include(newData.Cell(r, c), newData.Cell(r, 0)); 
//		}
//	}
//
//	return result;
//}
//
//DataSet Table::Exclude(const Ranges& ranges)
//{
//	//TODO: result will eventually be undo information
//	DataSet result(_type, 0);
//
//
//	return result;
//}
