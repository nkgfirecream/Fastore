#include "IDataAccess.h"
#include "Table\dataset.h"

//IDataAccess
void IDataAccess::Exclude(eastl::vector<void*>& rowIds, eastl::vector<fs::wstring>& columns, bool isPicky)
{
	for (int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]);

		fs::ValueVector values = cb->GetValues(rowIds);

		for (int j = 0; j < rowIds.size(); j++)
		{
			cb->Exclude(values[j], rowIds[j]);
		}
	}
}

DataSet IDataAccess::GetRange(eastl::vector<fs::wstring>& columns, Range& range, int rangecolumn /*, [sorting]*/)
{
	//TODO: Fix this assumption: Range is always based on first column passed
	ColumnTypeVector ctv;
	for (int i = 0; i < columns.size(); i++)
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
	
	GetResult result =  _host.GetColumn(columns[rangecolumn])->GetRows(range);

	//TODO: need to count results to get size of dataset to allocate.. How can we do this better? Pass count back with result?
	int numrows = 0;

	KeyVector kv(numrows);
	for (int i = 0; i < result.Data.size(); i++)
	{
		fs::ValueKeys keys = result.Data[i];
		for (int j = 0; j < keys.second.size(); j++)
		{
			kv.push_back(keys.second[j]);
			numrows++;
		}
	}

	DataSet ds(tt, numrows);

	//TODO: DataSet could easily be filled in a multi-thread fashion with a pointer the its buffer, a rowsize, and a rowoffset (each thread fills one column)
	//TODO: It's also the case that we don't need to call back into the column buffer to get the values for the ranged rows, we just need to write the code to materialize the data
	

	for (int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]);

		fs::ValueVector result = cb->GetValues(kv);
		for (int j = 0; j < kv.size(); j++)
		{
			ds.SetCell(j, i, result[j]);
		}
	}

	return ds;
}

DataSet IDataAccess::GetRows(eastl::vector<void*>& rowIds, eastl::vector<fs::wstring>& columns  /*, sorting */)
{
	ColumnTypeVector ctv;
	for (int i = 0; i < columns.size(); i++)
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

	for (int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]);

		fs::ValueVector result = cb->GetValues(rowIds);
		for (int j = 0; j < rowIds.size(); j++)
		{			
			ds.SetCell(j, i, result[j]);
		}
	}

	return ds;
}

int IDataAccess::Include(eastl::vector<void*>& row, eastl::vector<fs::wstring>& columns, bool isPicky)
{
	for (int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]);

		bool result = cb->Include(row[i], &_currentID);
	}

	_currentID++;

	return _currentID - 1;
}

void IDataAccess::Include(void* rowID, eastl::vector<void*>& row, eastl::vector<fs::wstring>& columns, bool isPicky)
{
	for (int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]);

		bool result = cb->Include(row[i], rowID);
	}
}
