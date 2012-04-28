#include "IDataAccess.h"
#include "Table\dataset.h"

//IDataAccess
void IDataAccess::Exclude(eastl::vector<void*>& rowIds, eastl::vector<fs::wstring>& columns)
{
	for (unsigned int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]);

		fs::ValueVector values = cb->GetValues(rowIds);

		for (unsigned int j = 0; j < rowIds.size(); j++)
		{
			cb->Exclude(values[j], rowIds[j]);
		}
	}
}

DataSet IDataAccess::GetRange(eastl::vector<fs::wstring>& columns, eastl::vector<Order>& orders, eastl::vector<Range>& ranges)
{
	//TODO: Fix this assumption: Orderby is always first range passed.
	ColumnTypeVector ctv;
	for (unsigned int i = 0; i < columns.size(); i++)
	{ 
		ColumnType ct;
		IColumnBuffer* cb = _host.GetColumn(columns[i]);
		ct.IsRequired = cb->GetRequired();
		ct.IsUnique = cb->GetUnique();
		ct.Name = columns[i];
		ct.Type = cb->GetKeyType();

		ctv.push_back(ct);
	}

	TupleType tt(ctv);
	GetResult result = _host.GetColumn(ranges[0].Column)->GetRows(ranges[0], true);

	KeyVector kv;

	for (unsigned int i = 0; i < result.Data.size(); i++)
	{
		for (unsigned int j = 0; j < result.Data[i].second.size(); j++)
		{
			kv.push_back(result.Data[i].second[j]);
		}
	}



	DataSet ds(tt, kv.size());

	//TODO: DataSet could easily be filled in a multi-thread fashion with a pointer the its buffer, a rowsize, and a rowoffset (each thread fills one column)
	//TODO: It's also the case that we don't need to call back into the column buffer to get the values for the ranged rows, we just need to write the code to materialize the data
	
	if (kv.size() > 0)
	{
		for (unsigned int i = 0; i < columns.size(); i++)
		{
			IColumnBuffer* cb = _host.GetColumn(columns[i]);

			fs::ValueVector result = cb->GetValues(kv);
			for (unsigned int j = 0; j < kv.size(); j++)
			{
				ds.SetCell(j, i, result[j]);
			}
		}
	}

	return ds;
}

DataSet IDataAccess::GetRows(eastl::vector<void*>& rowIds, eastl::vector<fs::wstring>& columns)
{
	ColumnTypeVector ctv;
	for (unsigned int i = 0; i < columns.size(); i++)
	{
		ColumnType ct;
		IColumnBuffer* cb = _host.GetColumn(columns[i]);
		ct.IsRequired = cb->GetRequired();
		ct.IsUnique = cb->GetUnique();
		ct.Name = columns[i];
		ct.Type = cb->GetKeyType();

		ctv.push_back(ct);
	}

	TupleType tt(ctv);

	DataSet ds(tt, rowIds.size());

	for (unsigned int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]);

		fs::ValueVector result = cb->GetValues(rowIds);
		for (unsigned int j = 0; j < rowIds.size(); j++)
		{			
			ds.SetCell(j, i, result[j]);
		}
	}

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

void IDataAccess::Include(void* rowID, eastl::vector<void*>& row, eastl::vector<fs::wstring>& columns)
{
	for (unsigned int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]);

		cb->Include(row[i], rowID);
	}
}

Statistics IDataAccess::GetStatistics(fs::wstring column)
{
	return _host.GetColumn(column)->GetStatistics();
}
