#include "FastoreHost.h"
#include "Column\TreeBuffer.h"
#include "Column\UniqueBuffer.h"
#include <hash_set>

FastoreHost::FastoreHost(const CoreConfig& config) : _config(config)
{
	//* Attempt to lock data directory - throw if ex can't be taken
	//* Check data directory for improper shut down - see Recovery
	//* If new instance, bootstrap
	BootStrap();
	//* Read topology columns into memory; play log files for the same
	//* Apply any host address overrides
	//* Determine host ID if by reversing host address - if fails throw with report of current host name
	//* Mode: Initializing - Notify peers (hosts that share redundancy), storing which peers are reachable in Grid Health memory structures.
	//* Initialize objects for this host (simultaneously unless it hurts)
	//	* Column Buffer - Read data dump into memory; play log file
	//	* Lock Manager - sync w/ peer(s)
	//	* Transactor - sync w/ peer(s)
	//* Mode: Online - Notify peers
}

void FastoreHost::BootStrap()
{
	ColumnDef id;
	id.ColumnID = 0;
	id.Name = "Column.ID";
	id.ValueType = standardtypes::Int;
	id.RowIDType = standardtypes::Int;
	id.IsUnique = true;
	CreateColumn(id);
	AddColumnToSchema(id);

	ColumnDef name;
	name.ColumnID = 1;
	name.Name = "Column.Name";
	name.ValueType = standardtypes::String;
	name.RowIDType = standardtypes::Int;
	name.IsUnique = false;
	CreateColumn(name);
	AddColumnToSchema(name);

	ColumnDef vt;
	vt.ColumnID = 2;
	vt.Name = "Column.ValueType";
	vt.ValueType = standardtypes::String;
	vt.RowIDType = standardtypes::Int;
	vt.IsUnique = false;
	CreateColumn(vt);
	AddColumnToSchema(vt);

	ColumnDef idt;
	idt.ColumnID = 3;
	idt.Name = "Column.RowIDType";
	idt.ValueType = standardtypes::String;
	idt.RowIDType = standardtypes::Int;
	idt.IsUnique = false;
	CreateColumn(idt);
	AddColumnToSchema(idt);

	ColumnDef unique;
	unique.ColumnID = 4;
	unique.Name = "Column.IsUnique";
	unique.ValueType = standardtypes::Bool;
	unique.RowIDType = standardtypes::Int;
	unique.IsUnique = false;	
	CreateColumn(unique);
	AddColumnToSchema(unique);

	ColumnDef topo;
	topo.ColumnID = 100;
	topo.Name = "Topology.ID";
	topo.ValueType = standardtypes::Int;
	topo.RowIDType = standardtypes::Int;
	topo.IsUnique = true;	
	CreateColumn(topo);
	AddColumnToSchema(topo);

	ColumnDef hostId;
	hostId.ColumnID = 200;
	hostId.Name = "Host.ID";
	hostId.ValueType = standardtypes::Int;
	hostId.RowIDType = standardtypes::Int;
	hostId.IsUnique = true;	
	CreateColumn(hostId);
	AddColumnToSchema(hostId);

	ColumnDef hostTopoId;
	hostTopoId.ColumnID = 201;
	hostTopoId.Name = "Host.TopologyID";
	hostTopoId.ValueType = standardtypes::Int;
	hostTopoId.RowIDType = standardtypes::Int;
	hostTopoId.IsUnique = false;	
	CreateColumn(hostTopoId);
	AddColumnToSchema(hostTopoId);

	ColumnDef podId;
	podId.ColumnID = 300;
	podId.Name = "Pod.ID";
	podId.ValueType = standardtypes::Int;
	podId.RowIDType = standardtypes::Int;
	podId.IsUnique = true;	
	CreateColumn(podId);
	AddColumnToSchema(podId);

	ColumnDef podHostId;
	podHostId.ColumnID = 301;
	podHostId.Name = "Pod.HostID";
	podHostId.ValueType = standardtypes::Int;
	podHostId.RowIDType = standardtypes::Int;
	podHostId.IsUnique = false;	
	CreateColumn(podHostId);
	AddColumnToSchema(podHostId);

	ColumnDef podColPodId;
	podColPodId.ColumnID = 400;
	podColPodId.Name = "PodColumn.PodID";
	podColPodId.ValueType = standardtypes::Int;
	podColPodId.RowIDType = standardtypes::Int;
	podColPodId.IsUnique = false;	
	CreateColumn(podColPodId);
	AddColumnToSchema(podColPodId);

	ColumnDef podColColId;
	podColColId.ColumnID = 401;
	podColColId.Name = "PodColumn.ColumnID";
	podColColId.ValueType = standardtypes::Int;
	podColColId.RowIDType = standardtypes::Int;
	podColColId.IsUnique = false;	
	CreateColumn(podColColId);
	AddColumnToSchema(podColColId);
}

IColumnBuffer* FastoreHost::InstantiateColumn(const ColumnDef& def)
{
	IColumnBuffer* newbuffer;

	if (def.IsUnique)
	{
		newbuffer = new UniqueBuffer(def.RowIDType, def.ValueType);
	}
	else
	{
		newbuffer = new TreeBuffer(def.RowIDType, def.ValueType);
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

void FastoreHost::CreateColumn(const ColumnDef& def)
{
	IColumnBuffer* buffer = InstantiateColumn(def);
	_columnMap.insert(std::pair<int, PointerDefPair>(def.ColumnID, PointerDefPair(buffer, def)));
}

void FastoreHost::CreateColumn(int columnId)
{
	ColumnDef def;
	def.ColumnID = columnId;
	def.Name = *(std::string*)GetColumn(1).first->GetValue(&columnId);
	def.ValueType = GetScalarTypeFromString(*(std::string*)GetColumn(2).first->GetValue(&columnId));
	def.RowIDType = GetScalarTypeFromString(*(std::string*)GetColumn(3).first->GetValue(&columnId));
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

	std::vector<int> curId;
	auto cs = _columnMap.begin();
	while (cs != _columnMap.end())
	{
		curId.push_back((*cs).first);
		cs++;
	}

	auto cis = curId.begin();
	while (cis != curId.end())
	{
		if (schemaIds.find(*cis) == schemaIds.end())
			DeleteColumn(*cis);

		cis++;
	}

	auto ss = schemaIds.begin();
	while (ss != schemaIds.end())
	{
		int id = (*ss);

		if (_columnMap.find(id) == _columnMap.end())
			CreateColumn(id);

		ss++;
	}

	

	
}

ScalarType FastoreHost::GetScalarTypeFromString(std::string typestring)
{
//TODO: Consider putting this into a hash to avoid branches.
	if (typestring == "WString")
	{
		return standardtypes::WString;
	}
	else if (typestring == "String")
	{
		return standardtypes::String;
	}
	else if (typestring == "Int")
	{
		return standardtypes::Int;
	}
	else if (typestring == "Long")
	{
		return standardtypes::Long;
	}
	else if (typestring == "Bool")
	{
		return standardtypes::Bool;
	}
	else
	{
		throw;
	}
}

