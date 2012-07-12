#include "WorkerHandler.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <utility>
#include <hash_set>
#include "Schema\standardtypes.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using boost::shared_ptr;
using namespace ::fastore::communication;
using namespace std;

const int MaxSystemColumnID = 9999;

WorkerHandler::WorkerHandler(const PodID podId, const string path) : _podId(podId), _path(path)
{
	//* Attempt to open data file
	//* Attempt to open log file
	//* Check data directory for improper shut down - see Recovery

	//* If (new instance), bootstrap
	Bootstrap();
	//  else load system columns


	//* Read rest of topology columns into memory; play log files for the same
}

void WorkerHandler::Bootstrap()
{
	ColumnDef id;
	id.ColumnID = 0;
	id.Name = "Column.ID";
	id.ValueType = standardtypes::Int;
	id.RowIDType = standardtypes::Int;
	id.BufferType = BufferType::Identity;
	CreateRepo(id);	

	ColumnDef name;
	name.ColumnID = 1;
	name.Name = "Column.Name";
	name.ValueType = standardtypes::String;
	name.RowIDType = standardtypes::Int;
	name.BufferType = BufferType::Unique;
	CreateRepo(name);

	ColumnDef vt;
	vt.ColumnID = 2;
	vt.Name = "Column.ValueType";
	vt.ValueType = standardtypes::String;
	vt.RowIDType = standardtypes::Int;
	vt.BufferType = BufferType::Multi;
	CreateRepo(vt);	

	ColumnDef idt;
	idt.ColumnID = 3;
	idt.Name = "Column.RowIDType";
	idt.ValueType = standardtypes::String;
	idt.RowIDType = standardtypes::Int;
	idt.BufferType = BufferType::Multi;
	CreateRepo(idt);	

	ColumnDef unique;
	unique.ColumnID = 4;
	unique.Name = "Column.BufferType";
	unique.ValueType = standardtypes::Int;
	unique.RowIDType = standardtypes::Int;
	unique.BufferType == BufferType::Multi;
	CreateRepo(unique);	

	ColumnDef topo;
	topo.ColumnID = 100;
	topo.Name = "Topology.ID";
	topo.ValueType = standardtypes::Int;
	topo.RowIDType = standardtypes::Int;
	topo.BufferType = BufferType::Identity;
	CreateRepo(topo);

	ColumnDef hostId;
	hostId.ColumnID = 200;
	hostId.Name = "Host.ID";
	hostId.ValueType = standardtypes::Int;
	hostId.RowIDType = standardtypes::Int;
	hostId.BufferType = BufferType::Identity;
	CreateRepo(hostId);	

	ColumnDef podId;
	podId.ColumnID = 300;
	podId.Name = "Pod.ID";
	podId.ValueType = standardtypes::Int;
	podId.RowIDType = standardtypes::Int;
	podId.BufferType = BufferType::Unique;
	CreateRepo(podId);	

	ColumnDef podHostId;
	podHostId.ColumnID = 301;
	podHostId.Name = "Pod.HostID";
	podHostId.ValueType = standardtypes::Int;
	podHostId.RowIDType = standardtypes::Int;
	podHostId.BufferType = BufferType::Multi;
	CreateRepo(podHostId);	

	ColumnDef podColPodId;
	podColPodId.ColumnID = 400;
	podColPodId.Name = "PodColumn.PodID";
	podColPodId.ValueType = standardtypes::Int;
	podColPodId.RowIDType = standardtypes::Int;
	podColPodId.BufferType = BufferType::Multi;
	CreateRepo(podColPodId);	

	ColumnDef podColColId;
	podColColId.ColumnID = 401;
	podColColId.Name = "PodColumn.ColumnID";
	podColColId.ValueType = standardtypes::Int;
	podColColId.RowIDType = standardtypes::Int;
	podColColId.BufferType = BufferType::Multi;
	CreateRepo(podColColId);

	//This must come after the repos are initialized. Can't add a column to the schema if the schema doesn't exist
	AddColumnToSchema(id);
	AddColumnToSchema(name);
	AddColumnToSchema(vt);
	AddColumnToSchema(idt);
	AddColumnToSchema(unique);
	AddColumnToSchema(topo);
	AddColumnToSchema(hostId);
	AddColumnToSchema(podId);
	AddColumnToSchema(podHostId);
	AddColumnToSchema(podColPodId);
	AddColumnToSchema(podColColId);
}

void WorkerHandler::CreateRepo(ColumnDef def)
{
	boost::shared_ptr<Repository> repo(new Repository(def.ColumnID, _path));
	repo->create(def);
	_repositories.insert(std::pair<ColumnID, boost::shared_ptr<Repository>>(def.ColumnID, repo));
}

void WorkerHandler::SyncToSchema()
{
	//pull all columns associated with this pod
	std::string podId;
	podId.assign((char*)&_podId, sizeof(PodID));

	RangeBound first;
	first.__set_inclusive(true);
	first.__set_value(podId);

	RangeBound last;
	last.__set_inclusive(true);
	last.__set_value(podId);

	
	//I doubt there will be more than 200000 columns associated with this pod.. But we should
	//add handling for that just in case
	RangeRequest range;
	range.__set_limit(200000);
	range.__set_ascending(true);
	range.__set_first(first);
	range.__set_last(last);

	std::vector<RangeRequest> ranges;
	ranges.push_back(range);

	Query query;
	query.__set_ranges(ranges);
	Answer answer = _repositories[400]->query(query);

	if (answer.rangeValues.at(0).valueRowsList.size() > 0)
	{
		//Got all the rows associated with this pod, now get their columnIds
		//(since the result should only have one value in it, so just be able to pass it right back in.
		query = Query();
		query.__set_rowIDs(answer.rangeValues.at(0).valueRowsList.at(0).rowIDs);

		answer = _repositories[401]->query(query);

		std::hash_set<ColumnID> schemaIds;

		for (int i = 0; i < answer.rowIDValues.size(); i++)
		{
			schemaIds.insert(*(ColumnID*)(answer.rowIDValues.at(i).data()));
		}

		// drop repos that we should no longer have
		for (auto repo = _repositories.begin(); repo != _repositories.end(); ) 
		{
			if (schemaIds.find(repo->first) == schemaIds.end() && repo->first > MaxSystemColumnID)
			{
				repo->second->drop();
				repo = _repositories.erase(repo);
			}
			else
				 ++repo;
		}

		//create repos we do need
		for (auto id = schemaIds.begin(); id != schemaIds.end(); ++id)
		{
			if (_repositories.find(*id) == _repositories.end())
			{
				ColumnDef def = GetDefFromSchema(*id);
				CreateRepo(def);
			}
		}	
	}
}

void WorkerHandler::AddColumnToSchema(ColumnDef def)
{
	ColumnWrites writes;

	//id column
	std::string columnId;
	columnId.assign((char*)&def.ColumnID, sizeof(ColumnID));

	std::vector<Include> includes;
	Include include;
	include.__set_value(columnId);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[0]->apply(0, writes);


	//name column
	includes.clear();

	include.__set_value(def.Name);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[1]->apply(0, writes);


	//valueType column
	includes.clear();

	include.__set_value(def.ValueType.Name);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[2]->apply(0, writes);


	//rowType column
	includes.clear();

	include.__set_value(def.RowIDType.Name);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[3]->apply(0, writes);


	//unique column
	includes.clear();

	std::string unique;
	unique.assign((char*)&def.BufferType, sizeof(int));

	include.__set_value(unique);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[4]->apply(0, writes);	
}

ColumnDef WorkerHandler::GetDefFromSchema(ColumnID id)
{
	//for the columns table, the rowId is the columnId
	ColumnDef def;
	def.ColumnID = id;	

	std::string rowId;
	rowId.assign((char*)&id, sizeof(ColumnID));

	std::vector<std::string> rowIds;
	rowIds.push_back(rowId);

	Query query;
	query.__set_rowIDs(rowIds);	

	//Name column
	Answer answer = _repositories[1]->query(query);
	def.Name = answer.rowIDValues.at(0);

	//ValueType
	answer = _repositories[2]->query(query);
	def.ValueType = GetTypeFromName(answer.rowIDValues.at(0));

	//RowType
	answer = _repositories[3]->query(query);
	def.RowIDType = GetTypeFromName(answer.rowIDValues.at(0));

	//Unique
	answer = _repositories[4]->query(query);
	def.BufferType = (BufferType)*(int*)(answer.rowIDValues.at(0).data());

	return def;
}

ScalarType WorkerHandler::GetTypeFromName(std::string typeName)
{
	//TODO: Consider putting this into a hash to avoid branches.
	if (typeName == "WString")
	{
		return standardtypes::WString;
	}
	else if (typeName == "String")
	{
		return standardtypes::String;
	}
	else if (typeName == "Int")
	{
		return standardtypes::Int;
	}
	else if (typeName == "Long")
	{
		return standardtypes::Long;
	}
	else if (typeName == "Bool")
	{
		return standardtypes::Bool;
	}
	else
	{
		throw "TypeName not recognized";
	}
}

void WorkerHandler::CheckState()
{
	//if (_state == WorkerState.
}

Revision WorkerHandler::prepare(const TransactionID& transactionID, const Writes& writes, const Reads& reads) 
{
	CheckState();

	// Your implementation goes here
	printf("Prepare\n");

	return 0;
}

void WorkerHandler::apply(TransactionID& _return, const TransactionID& transactionID, const Writes& writes) 
{
	bool syncSchema = false;

	auto start = writes.begin();

	while(start != writes.end())
	{
		fastore::communication::ColumnID id = start->first;

		//If pod or column table changes we need to check and see if we should update.
		if (id == 400 || id == 401)
			syncSchema = true;

		boost::shared_ptr<Repository> repo = _repositories[id];
		ColumnWrites writes = start->second;

		repo->apply(transactionID.revision, writes);
		
		++start;
	}

	if (syncSchema)
		SyncToSchema();
}

void WorkerHandler::commit(const TransactionID& transactionID) {
// Your implementation goes here
//printf("Commit\n");
}

void WorkerHandler::rollback(const TransactionID& transactionID) {
// Your implementation goes here
printf("Rollback\n");
}

void WorkerHandler::flush(const TransactionID& transactionID) {
// Your implementation goes here
printf("Flush\n");
}

bool WorkerHandler::doesConflict(const Reads& reads, const Revision source, const Revision target) {
// Your implementation goes here
printf("DoesConflict\n");

return false;
}

void WorkerHandler::update(TransactionID& _return, const TransactionID& transactionID, const Writes& writes, const Reads& reads) {
// Your implementation goes here
printf("Update\n");
}

void WorkerHandler::transgrade(Reads& _return, const Reads& reads, const Revision source, const Revision target) {
// Your implementation goes here
printf("Transgrade\n");
}


void WorkerHandler::query(ReadResults& _return, const Queries& queries)
{
	auto start = queries.begin();
	while (start != queries.end())
	{
		ColumnID id = start->first;
		boost::shared_ptr<Repository> repo = _repositories[id];

		Query query = start->second;
		Answer answer = repo->query(query);
		Revision rev = repo->getRevision();

		ReadResult result;
		result.__set_answer(answer);
		result.__set_revision(rev);

		_return.insert(std::pair<ColumnID, ReadResult>(id, result));

		start++;
	}
}

void WorkerHandler::getStatistics(std::vector<Statistic> & _return, const std::vector<ColumnID> & columnIDs)
{	
	for (int i = 0; i < columnIDs.size(); i++)
	{		
		boost::shared_ptr<Repository> repo = _repositories[columnIDs[i]];
		Statistic stat = repo->getStatistic();
		_return.push_back(stat);
	}
}

void WorkerHandler::getState(WorkerState& _return)
{
	_return.__set_podID(_podId);
	
	std::map<ColumnID, RepositoryStatus::type> statuses;

	for (auto iter = _repositories.begin(); iter != _repositories.end(); ++iter)
	{
		statuses.insert(pair<ColumnID, RepositoryStatus::type>(iter->first, iter->second->getStatus()));
	}
	
	_return.__set_repositoryStatus(statuses);
}

void WorkerHandler::handlerError(void* ctx, const char* fn_name)
{
	if (fn_name == "Worker.flush")
	{

	}

	//Do some cleaning of connections?

	return;
}

void* WorkerHandler::getContext(const char* fn_name, void* serverContext)
{
	_currentContext = *(boost::shared_ptr<TMultiConnectionContext>*)serverContext;

	return serverContext;
}
