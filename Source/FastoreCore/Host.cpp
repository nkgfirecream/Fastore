#include "Host.h"
#include "Column\HashBuffer.h"
#include "Column\UniqueBuffer.h"
#include "Util\utilities.h"

void Host::CreateColumn(ColumnDef  def)
{
	IColumnBuffer* newbuffer;
	ScalarType rowType = GetScalarTypeFromString(def.Type);
	ScalarType keyType = GetScalarTypeFromString(def.KeyType);
	if(def.IsUnique)
	{
		newbuffer = new UniqueBuffer(rowType, keyType, def.Name);
	}
	else
	{
		newbuffer = new HashBuffer(rowType, keyType, def.Name);
	}

	_columns.push_back(newbuffer);
	_columnMap.insert(eastl::pair<fs::wstring, int>(def.Name, _columns.size() - 1));
}

void Host::DeleteColumn(fs::wstring name)
{
	int index = _columnMap.find(name)->second;

	IColumnBuffer* toDelete = _columns[index];

	_columns.erase(_columns.begin() + index);

	delete toDelete;

	//rebuild index
	_columnMap.clear();

	for (int i = 0; i < _columns.size(); i++)
	{
		IColumnBuffer* buf = _columns.at(i);

		_columnMap.insert(eastl::pair<fs::wstring, int>(buf->GetName(), i));
	}
}

IColumnBuffer* Host::GetColumn(const fs::wstring name)
{
	return _columns[_columnMap.find(name)->second];
}


ScalarType Host::GetScalarTypeFromString(fs::wstring tname)
{
	if (tname == L"String")
	{
		return standardtypes::GetStringType();
	}
	else if (tname == L"Int")
	{
		return standardtypes::GetIntType();
	}
	else if (tname == L"Long")
	{
		return standardtypes::GetLongType();
	}
	else if (tname == L"Bool")
	{
		throw; //TODO: Impelement Bool type
	}
	else
	{
		throw;
	}
}