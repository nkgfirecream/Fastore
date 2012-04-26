#include "IDataAccess.h"
#include "Table\dataset.h"

//IDataAccess
void IDataAccess::Exclude(eastl::vector<void*>& rowIds, eastl::vector<fs::wstring>& columns, bool isPicky)
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

DataSet IDataAccess::GetRange(eastl::vector<fs::wstring>& columns, eastl::vector<Range>& ranges)
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
	GetResult result = _host.GetColumn(ranges[0].Column)->GetRows(ranges[0]);

	//Store old ids...
	//eastl::vector<void*> rowIds;
	//eastl::vector<void*> rowIdsOrdered;
	//for (unsigned int i = 0; i < ranges.size(); i++)
	//{
	//	
	//	if (i == 0)
	//	{
	//		for (unsigned int k = 0; k < result.Data.size(); k++)
	//		{
	//			fs::ValueKeys keys = result.Data[k];
	//			for (unsigned int j = 0; j < keys.second.size(); j++)
	//			{
	//				rowIds.push_back(keys.second[j]);
	//				rowIdsOrdered.push_back(keys.second[j]);
	//			}
	//		}
	//	}
	//	else
	//	{
	//		//Holy Nested Loops batman!!! (rework this once we have a substitute for the hash_set)
	//		eastl::vector<void*> temp;
	//		for (unsigned int k = 0; k < result.Data.size(); k++)
	//		{
	//			fs::ValueKeys keys = result.Data[k];
	//			for (unsigned int j = 0; j < keys.second.size(); j++)
	//			{
	//				for (unsigned int l = 0; l < rowIds.size(); l++)
	//				{
	//					if (keys.second[j] == rowIds[l])
	//					{
	//						temp.push_back(keys.second[j]);
	//						break;
	//					}
	//				}
	//			}
	//		}

	//		rowIds = temp;
	//	}

	//	//filtered everything, just skip
	//	if(rowIds.size() == 0)
	//		break;
	//}

	//KeyVector kv(rowIds.size());
	////Put stuff back into a vector... but in the right order
	//int index = 0;
	//for (unsigned int i = 0; i < rowIdsOrdered.size(); i++)
	//{
	//	for (unsigned int j = 0; j < rowIds.size(); j++)
	//	{
	//		if (rowIds[j] == rowIdsOrdered[i])
	//		{
	//			kv[index] = rowIdsOrdered[i];
	//			index++;
	//			break;
	//		}
	//	}
	//}

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

DataSet IDataAccess::GetRows(eastl::vector<void*>& rowIds, eastl::vector<fs::wstring>& columns  /*, sorting */)
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

int IDataAccess::Include(eastl::vector<void*>& row, eastl::vector<fs::wstring>& columns, bool isPicky)
{
	for (unsigned int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]);

		cb->Include(row[i], &_currentID);
	}

	_currentID++;

	return _currentID - 1;
}

void IDataAccess::Include(void* rowID, eastl::vector<void*>& row, eastl::vector<fs::wstring>& columns, bool isPicky)
{
	for (unsigned int i = 0; i < columns.size(); i++)
	{
		IColumnBuffer* cb = _host.GetColumn(columns[i]);

		cb->Include(row[i], rowID);
	}
}
