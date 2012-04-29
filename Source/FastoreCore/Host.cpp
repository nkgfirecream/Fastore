#include "Host.h"
#include "Column\HashBuffer.h"
#include "Column\TreeBuffer.h"
#include "Column\UniqueBuffer.h"
#include "Util\utilities.h"

void Host::CreateColumn(ColumnDef  def)
{
	IColumnBuffer* newbuffer;
	if(def.IsUnique)
	{
		newbuffer = new UniqueBuffer(def.IDType, def.KeyType, def.Name);
	}
	else
	{
		newbuffer = new TreeBuffer(def.IDType, def.KeyType, def.Name);
	}

	_columns.push_back(PointerDefPair(newbuffer, def));
	_columnMap.insert(eastl::pair<fs::wstring, int>(def.Name, _columns.size() - 1));
}

void Host::DeleteColumn(fs::wstring name)
{
	int index = _columnMap.find(name)->second;

	IColumnBuffer* toDelete = _columns[index].first;

	_columns.erase(_columns.begin() + index);

	delete toDelete;

	//rebuild index
	_columnMap.clear();

	for (unsigned int i = 0; i < _columns.size(); i++)
	{
		IColumnBuffer* buf = _columns.at(i).first;
		ColumnDef def = _columns.at(i).second;

		_columnMap.insert(eastl::pair<fs::wstring, int>(buf->GetName(), i));
	}
}

PointerDefPair Host::GetColumn(const fs::wstring name)
{
	return _columns[_columnMap.find(name)->second];
}
