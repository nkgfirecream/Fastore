#include "FastoreHost.h"
#include "Column\TreeBuffer.h"
#include "Column\UniqueBuffer.h"
#include "Util\utilities.h"

FastoreHost::FastoreHost()
{
	BootStrap();
}

void FastoreHost::BootStrap()
{
	ColumnDef id;
	ColumnDef name;
	ColumnDef vt;
	ColumnDef idt;
	ColumnDef unique;

	id.ColumnID = 0;
	id.Name = L"ID";
	id.ValueType = standardtypes::GetIntType();
	id.RowIDType = standardtypes::GetIntType();
	id.IsUnique = true;

	name.ColumnID = 1;
	name.Name = L"Name";
	name.ValueType = standardtypes::GetStringType();
	name.RowIDType = standardtypes::GetIntType();
	name.IsUnique = false;

	vt.ColumnID = 3;
	vt.Name = L"ValueType";
	vt.ValueType = standardtypes::GetStringType();
	vt.RowIDType = standardtypes::GetIntType();
	vt.IsUnique = false;

	idt.ColumnID = 4;
	idt.Name = L"RowIDType";
	idt.ValueType = standardtypes::GetStringType();
	idt.RowIDType = standardtypes::GetIntType();
	idt.IsUnique = false;

	unique.ColumnID = 5;
	unique.Name = L"IsUnique";
	unique.ValueType = standardtypes::GetBoolType();
	unique.RowIDType = standardtypes::GetIntType();
	unique.IsUnique = false;	

	IColumnBuffer* idp = InstantiateColumn(id);
	IColumnBuffer* namep = InstantiateColumn(name);
	IColumnBuffer* vtp = InstantiateColumn(vt);
	IColumnBuffer* idtp = InstantiateColumn(idt);
	IColumnBuffer* uniquep = InstantiateColumn(unique);

	_columnMap.insert(std::pair<int, PointerDefPair>(id.ColumnID, PointerDefPair(idp, id)));
	_columnMap.insert(std::pair<int, PointerDefPair>(name.ColumnID, PointerDefPair(namep, name)));
	_columnMap.insert(std::pair<int, PointerDefPair>(vt.ColumnID, PointerDefPair(vtp, vt)));
	_columnMap.insert(std::pair<int, PointerDefPair>(idt.ColumnID, PointerDefPair(idtp, id)));
	_columnMap.insert(std::pair<int, PointerDefPair>(unique.ColumnID, PointerDefPair(uniquep, unique)));

	AddColumnToSchema(id);
	AddColumnToSchema(name);
	AddColumnToSchema(vt);
	AddColumnToSchema(idt);
	AddColumnToSchema(unique);

	//TODO: Make modification to these columns occur to every column in the group (pseudo-table).
	//Whenever there is an addition see if we need to instantiate...
}

IColumnBuffer* FastoreHost::InstantiateColumn(ColumnDef def)
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

	return newbuffer;
}

void FastoreHost::AddColumnToSchema(ColumnDef def)
{
	_columnMap.find(0)->second.first->Include(&def.ColumnID, &def.ColumnID);
	_columnMap.find(1)->second.first->Include(&def.Name, &def.ColumnID);
	_columnMap.find(2)->second.first->Include(&def.ValueType.Name, &def.ColumnID);
	_columnMap.find(3)->second.first->Include(&def.RowIDType.Name, &def.ColumnID);
	_columnMap.find(4)->second.first->Include(&def.IsUnique, &def.ColumnID);	
}

void FastoreHost::RemoveColumnFromSchema(int columnId)
{
	/*_columnMap.find(0)->second.first->Exclude(&columnId);
	_columnMap.find(1)->second.first->Exclude(&columnId);
	_columnMap.find(2)->second.first->Exclude(&columnId);
	_columnMap.find(3)->second.first->Exclude(&columnId);
	_columnMap.find(3)->second.first->Exclude(&columnId);*/
}

void FastoreHost::CreateColumn(ColumnDef  def)
{
	IColumnBuffer* buffer = InstantiateColumn(def);
	_columnMap.insert(std::pair<int, PointerDefPair>(def.ColumnID, PointerDefPair(buffer, def)));
	AddColumnToSchema(def);	
}

void FastoreHost::DeleteColumn(const int& columnId)
{
	//int index = _columnMap.find(columnId)->second;

	//IColumnBuffer* toDelete = _columns[index].first;

	//_columns.erase(_columns.begin() + index);

	//delete toDelete;

	////rebuild index
	//_columnMap.clear();

	//for (unsigned int i = 0; i < _columns.size(); i++)
	//{
	//	IColumnBuffer* buf = _columns.at(i).first;
	//	ColumnDef def = _columns.at(i).second;

	//	_columnMap.insert(std::pair<int, int>(buf->GetID(), i));
	//}
}

PointerDefPair FastoreHost::GetColumn(const int& columnId)
{
	return _columnMap.find(columnId)->second;
}

bool FastoreHost::ExistsColumn(const int& columnId)
{
	return _columnMap.find(columnId) != _columnMap.end();
}
