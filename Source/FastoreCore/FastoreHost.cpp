#include "FastoreHost.h"
#include "Column\TreeBuffer.h"
#include "Column\UniqueBuffer.h"
#include <hash_set>

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
	id.Name = "ID";
	id.ValueType = standardtypes::GetIntType();
	id.RowIDType = standardtypes::GetIntType();
	id.IsUnique = true;

	name.ColumnID = 1;
	name.Name = "Name";
	name.ValueType = standardtypes::GetStringType();
	name.RowIDType = standardtypes::GetIntType();
	name.IsUnique = false;

	vt.ColumnID = 2;
	vt.Name = "ValueType";
	vt.ValueType = standardtypes::GetStringType();
	vt.RowIDType = standardtypes::GetIntType();
	vt.IsUnique = false;

	idt.ColumnID = 3;
	idt.Name = "RowIDType";
	idt.ValueType = standardtypes::GetStringType();
	idt.RowIDType = standardtypes::GetIntType();
	idt.IsUnique = false;

	unique.ColumnID = 4;
	unique.Name = "IsUnique";
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
	_columnMap.insert(std::pair<int, PointerDefPair>(idt.ColumnID, PointerDefPair(idtp, idt)));
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
	if (def.IsUnique)
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
	_columnMap.find(0)->second.first->Exclude(&columnId);
	_columnMap.find(1)->second.first->Exclude(&columnId);
	_columnMap.find(2)->second.first->Exclude(&columnId);
	_columnMap.find(3)->second.first->Exclude(&columnId);
	_columnMap.find(4)->second.first->Exclude(&columnId);
}

void FastoreHost::CreateColumn(ColumnDef  def)
{
	IColumnBuffer* buffer = InstantiateColumn(def);
	_columnMap.insert(std::pair<int, PointerDefPair>(def.ColumnID, PointerDefPair(buffer, def)));
}

void FastoreHost::CreateColumn(int columnId)
{
	ColumnDef def;
	def.ColumnID = columnId;
	def.Name = *(fs::string*)GetColumn(1).first->GetValue(&columnId);
	def.ValueType = GetScalarTypeFromString(*(fs::string*)GetColumn(2).first->GetValue(&columnId));
	def.RowIDType = GetScalarTypeFromString(*(fs::string*)GetColumn(3).first->GetValue(&columnId));
	def.IsUnique = *(bool*)GetColumn(4).first->GetValue(&columnId);

	CreateColumn(def);
}

void FastoreHost::DeleteColumn(const int& columnId)
{
	IColumnBuffer* toDelete   = _columnMap.find(columnId)->second.first;

	delete toDelete;

	_columnMap.erase(columnId);
}

PointerDefPair FastoreHost::GetColumn(const int& columnId)
{
	return _columnMap.find(columnId)->second;
}

bool FastoreHost::ExistsColumn(const int& columnId)
{
	return _columnMap.find(columnId) != _columnMap.end();
}

void FastoreHost::SyncToSchema()
{
	//TODO: This will eventually be replaced by the client sending repo updates and the host
	//instantiating according to that. This algorithm is not going to scale well to lots and lots
	//of columns.

	std::hash_set<int> schemaIds;

	fs::Range range(2000000, true);

	auto result = GetColumn(0).first->GetRows(range);

	for (int i = 0; i < result.Data.size(); i++)
	{
		schemaIds.insert(*(int*)(result.Data[i].first));
	}

	auto ss = schemaIds.begin();
	while (ss != schemaIds.end())
	{
		int id = (*ss);

		if (_columnMap.find(id) == _columnMap.end())
			CreateColumn(id);

		ss++;
	}

	auto ms = _columnMap.begin();
	while (ms != _columnMap.end())
	{
		int id = (*ms).first;

		if (schemaIds.find(id) == schemaIds.end())
			DeleteColumn(id);

		ms++;
	}	
}

ScalarType FastoreHost::GetScalarTypeFromString(std::string typestring)
{
//TODO: Consider putting this into a hash to avoid branches.
	if (typestring == "WString")
	{
		return standardtypes::GetWStringType();
	}
	else if (typestring == "String")
	{
		return standardtypes::GetStringType();
	}
	else if (typestring == "Int")
	{
		return standardtypes::GetIntType();
	}
	else if (typestring == "Long")
	{
		return standardtypes::GetLongType();
	}
	else if (typestring == "Bool")
	{
		return standardtypes::GetBoolType();
	}
	else
	{
		throw;
	}
}
