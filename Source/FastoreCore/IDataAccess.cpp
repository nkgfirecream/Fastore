#include "IDataAccess.h"
#include "Table\dataset.h"

//IDataAccess
void IDataAccess::Exclude(void* rowId, eastl::vector<int>& columns)
{
	fs::KeyVector kv;
	kv.push_back(rowId);

	for (unsigned int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]).first;

		fs::ValueVector values = cb->GetValues(kv);

		for (unsigned int j = 0; j < values.size(); j++)
		{
			cb->Exclude(values[j], rowId);
		}
	}
}

DataSet IDataAccess::GetRange(eastl::vector<int>& columns, eastl::vector<Order>& orders, eastl::vector<Range>& ranges)
{
	//TODO: Fix this assumption: Orderby is always first range passed.
	ColumnDefVector ctv(columns.size());
	for (unsigned int i = 0; i < columns.size(); i++)
	{ 
		ctv[i] = _host.GetColumn(columns[i]).second;
	}
	
	TupleType tt(ctv);
	GetResult result;
	
	if (ranges.size() > 0)
	{
		result = _host.GetColumn(ranges[0].ColumnID).first->GetRows(ranges[0]);
	}
	else
	{
		Range range(columns[0]);
		result = _host.GetColumn(range.ColumnID).first->GetRows(range);
	}

	KeyVector kv;

	for (unsigned int i = 0; i < result.Data.size(); i++)
	{
		for (unsigned int j = 0; j < result.Data[i].second.size(); j++)
		{
			kv.push_back(result.Data[i].second[j]);
		}
	}


	DataSet ds(tt, kv.size());
	ds.Limited = result.Limited;

	//TODO: DataSet could easily be filled in a multi-thread fashion with a pointer the its buffer, a rowsize, and a rowoffset (each thread fills one column)
	//TODO: It's also the case that we don't need to call back into the column buffer to get the values for the ranged rows, we just need to write the code to materialize the data
	
	if (kv.size() > 0)
	{
		for (unsigned int i = 0; i < columns.size(); i++)
		{
			IColumnBuffer* cb = _host.GetColumn(columns[i]).first;

			fs::ValueVector result = cb->GetValues(kv);
			for (unsigned int j = 0; j < kv.size(); j++)
			{
				ds.SetCell(j, i, result[j]);
			}
		}
	}

	return ds;
}

DataSet IDataAccess::GetRows(eastl::vector<void*>& rowIds, eastl::vector<int>& columns)
{
	ColumnDefVector ctv(columns.size());
	for (unsigned int i = 0; i < columns.size(); i++)
	{ 
		ctv[i] = _host.GetColumn(columns[i]).second;
	}

	TupleType tt(ctv);

	DataSet ds(tt, rowIds.size());

	for (unsigned int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]).first;

		fs::ValueVector result = cb->GetValues(rowIds);
		for (unsigned int j = 0; j < rowIds.size(); j++)
		{			
			ds.SetCell(j, i, result[j]);
		}
	}

	//We always return the complete set of rowsIds. Buffering will be handled on a higher level (consider rethinking this...)
	ds.Limited = false;
	return ds;
}

//int IDataAccess::Include(eastl::vector<void*>& row, eastl::vector<fs::wstring>& columns, bool isPicky)
//{
//	for (unsigned int i = 0; i < columns.size(); i++)
//	{
//		IColumnBuffer* cb = _host.GetColumn(columns[i]);
//
//		cb->Include(row[i], &_currentID);
//	}
//
//	_currentID++;
//
//	return _currentID - 1;
//}

void IDataAccess::Include(void* rowID, eastl::vector<void*>& row, eastl::vector<int>& columns)
{
	for (unsigned int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]).first;

		cb->Include(row[i], rowID);
	}
}

Statistics IDataAccess::GetStatistics(const int& columnId)
{
	return _host.GetColumn(columnId).first->GetStatistics();
}
