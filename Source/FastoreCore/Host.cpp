#include "Host.h"
#include "Column\HashBuffer.h"
#include "Column\TreeBuffer.h"
#include "Column\UniqueBuffer.h"
#include "Util\utilities.h"

void Host::CreateColumn(ColumnDef  def)
{
	IColumnBuffer* newbuffer;
	//Maybe I should just pass in the entire column def...
	if(def.IsUnique)
	{
		newbuffer = new UniqueBuffer(def.ColumnID, def.RowIDType, def.ValueType, def.Name);
	}
	else
	{
		newbuffer = new TreeBuffer(def.ColumnID, def.RowIDType, def.ValueType, def.Name);
	}

	_columns.push_back(PointerDefPair(newbuffer, def));
	_columnMap.insert(eastl::pair<int, int>(def.ColumnID, _columns.size() - 1));
}

void Host::DeleteColumn(const int& columnId)
{
	int index = _columnMap.find(columnId)->second;

	IColumnBuffer* toDelete = _columns[index].first;

	_columns.erase(_columns.begin() + index);

	delete toDelete;

	//rebuild index
	_columnMap.clear();

	for (unsigned int i = 0; i < _columns.size(); i++)
	{
		IColumnBuffer* buf = _columns.at(i).first;
		ColumnDef def = _columns.at(i).second;

		_columnMap.insert(eastl::pair<int, int>(buf->GetID(), i));
	}
}

PointerDefPair Host::GetColumn(const int& columnId)
{
	return _columns[_columnMap.find(columnId)->second];
}

bool Host::ExistsColumn(const int& columnId)
{
	return _columnMap.find(columnId) != _columnMap.end();
}
