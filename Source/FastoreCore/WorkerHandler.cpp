#include "WorkerHandler.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <utility>
#include <unordered_set>
#include "../FastoreCommon/Type/standardtypes.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using boost::shared_ptr;
using namespace ::fastore::communication;
using namespace std;

const int MaxSystemColumnID = 9999;

WorkerHandler::WorkerHandler(const PodID podId) 
  : _podId(podId)
{
	//* Attempt to open data file
	//* Check data directory for improper shut down - see Recovery
	//boost::filesystem::path fpath (_path);
	////If existing data, load it.
	//if (boost::filesystem::exists(fpath))
	//{
	//	boost::filesystem::directory_iterator iter(fpath), end;
	//	while(iter != end)
	//	{
	//		auto fnpath = iter->path();
	//		std::string fn = fnpath.filename().string();

	//		std::stringstream ss(fn);
	//		std::string id;
	//		std::getline(ss, id, '_');
	//		
	//		int64_t columnid = _atoi64(id.c_str());

	//		boost::shared_ptr<Repository> repo(new Repository(columnid, _path));
	//		_repositories[columnid] = repo;
	//		++iter;
	//	}
	//}
	//else
	//{
	//	//* If (new instance), bootstrap
	//	
	//}

	//* Read rest of topology columns into memory; play log files for the same

	Bootstrap();
}

WorkerHandler::~WorkerHandler()
{
	_repositories.clear();
}

void WorkerHandler::Bootstrap()
{
	ColumnDef id;
	id.ColumnID = 0;
	id.Name = "Column.ID";
	id.ValueType = standardtypes::Long;
	id.RowIDType = standardtypes::Long;
	id.BufferType = BufferType_t::Identity;
	id.Required = true;
	CreateRepo(id);	

	ColumnDef name;
	name.ColumnID = 1;
	name.Name = "Column.Name";
	name.ValueType = standardtypes::String;
	name.RowIDType = standardtypes::Long;
	name.BufferType = BufferType_t::Unique;
	name.Required = true;
	CreateRepo(name);

	ColumnDef vt;
	vt.ColumnID = 2;
	vt.Name = "Column.ValueType";
	vt.ValueType = standardtypes::String;
	vt.RowIDType = standardtypes::Long;
	vt.BufferType = BufferType_t::Multi;
	vt.Required = true;
	CreateRepo(vt);	

	ColumnDef idt;
	idt.ColumnID = 3;
	idt.Name = "Column.RowIDType";
	idt.ValueType = standardtypes::String;
	idt.RowIDType = standardtypes::Long;
	idt.BufferType = BufferType_t::Multi;
	idt.Required = true;
	CreateRepo(idt);	

	ColumnDef unique;
	unique.ColumnID = 4;
	unique.Name = "Column.BufferType";
	unique.ValueType = standardtypes::Int;
	unique.RowIDType = standardtypes::Long;
	unique.BufferType = BufferType_t::Multi;
	unique.Required = true;
	CreateRepo(unique);	

	ColumnDef required;
	required.ColumnID = 5;
	required.Name = "Column.Required";
	required.ValueType = standardtypes::Bool;
	required.RowIDType = standardtypes::Long;
	required.BufferType = BufferType_t::Multi;
	required.Required = true;
	CreateRepo(required);

	ColumnDef topo;
	topo.ColumnID = 100;
	topo.Name = "Topology.ID";
	topo.ValueType = standardtypes::Long;
	topo.RowIDType = standardtypes::Long;
	topo.BufferType = BufferType_t::Identity;
	topo.Required = true;
	CreateRepo(topo);

	ColumnDef hostId;
	hostId.ColumnID = 200;
	hostId.Name = "Host.ID";
	hostId.ValueType = standardtypes::Long;
	hostId.RowIDType = standardtypes::Long;
	hostId.BufferType = BufferType_t::Identity;
	hostId.Required = true;
	CreateRepo(hostId);	

	ColumnDef podId;
	podId.ColumnID = 300;
	podId.Name = "Pod.ID";
	podId.ValueType = standardtypes::Long;
	podId.RowIDType = standardtypes::Long;
	podId.BufferType = BufferType_t::Unique;
	podId.Required = true;
	CreateRepo(podId);	

	ColumnDef podHostId;
	podHostId.ColumnID = 301;
	podHostId.Name = "Pod.HostID";
	podHostId.ValueType = standardtypes::Long;
	podHostId.RowIDType = standardtypes::Long;
	podHostId.BufferType = BufferType_t::Multi;
	podHostId.Required = true;
	CreateRepo(podHostId);	

	ColumnDef podColPodId;
	podColPodId.ColumnID = 400;
	podColPodId.Name = "PodColumn.PodID";
	podColPodId.ValueType = standardtypes::Long;
	podColPodId.RowIDType = standardtypes::Long;
	podColPodId.BufferType = BufferType_t::Multi;
	podColPodId.Required = true;
	CreateRepo(podColPodId);	

	ColumnDef podColColId;
	podColColId.ColumnID = 401;
	podColColId.Name = "PodColumn.ColumnID";
	podColColId.ValueType = standardtypes::Long;
	podColColId.RowIDType = standardtypes::Long;
	podColColId.BufferType = BufferType_t::Multi;
	podColColId.Required = true;
	CreateRepo(podColColId);

	ColumnDef stashId;
	stashId.ColumnID = 500;
	stashId.Name = "Stash.ID";
	stashId.ValueType = standardtypes::Long;
	stashId.RowIDType = standardtypes::Long;
	stashId.BufferType = BufferType_t::Unique;
	stashId.Required = true;
	CreateRepo(stashId);	

	ColumnDef stashHostId;
	stashHostId.ColumnID = 501;
	stashHostId.Name = "Stash.HostID";
	stashHostId.ValueType = standardtypes::Long;
	stashHostId.RowIDType = standardtypes::Long;
	stashHostId.BufferType = BufferType_t::Multi;
	stashHostId.Required = true;
	CreateRepo(stashHostId);	

	ColumnDef stashColStashId;
	stashColStashId.ColumnID = 600;
	stashColStashId.Name = "StashColumn.StashID";
	stashColStashId.ValueType = standardtypes::Long;
	stashColStashId.RowIDType = standardtypes::Long;
	stashColStashId.BufferType = BufferType_t::Multi;
	stashColStashId.Required = true;
	CreateRepo(stashColStashId);	

	ColumnDef stashColColId;
	stashColColId.ColumnID = 601;
	stashColColId.Name = "StashColumn.ColumnID";
	stashColColId.ValueType = standardtypes::Long;
	stashColColId.RowIDType = standardtypes::Long;
	stashColColId.BufferType = BufferType_t::Multi;
	stashColColId.Required = true;
	CreateRepo(stashColColId);
	
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
	AddColumnToSchema(stashId);
	AddColumnToSchema(stashHostId);
	AddColumnToSchema(stashColStashId);
	AddColumnToSchema(stashColColId);}

void WorkerHandler::CreateRepo(ColumnDef def)
{
	boost::shared_ptr<Repository> repo(new Repository(def));
	_repositories[def.ColumnID] = repo;
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

		std::unordered_set<ColumnID> schemaIds;

		for (size_t i = 0; i < answer.rowIDValues.size(); i++)
		{
			fastore::communication::ColumnID id = *(ColumnID*)(answer.rowIDValues[i].value.data());
			schemaIds.insert(id);
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

	std::vector<Cell> includes;
	Cell include;
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

	//required column
	includes.clear();
	std::string required;
	required.assign((char*)&def.Required, sizeof(bool));

	include.__set_value(required);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[5]->apply(0, writes);
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
	def.Name = answer.rowIDValues[0].value;

	//ValueType
	answer = _repositories[2]->query(query);
	def.ValueType = standardtypes::GetTypeFromName(answer.rowIDValues.at(0).value);

	//RowType
	answer = _repositories[3]->query(query);
	def.RowIDType = standardtypes::GetTypeFromName(answer.rowIDValues[0].value);

	//Unique
	answer = _repositories[4]->query(query);
	def.BufferType = (BufferType_t)*(int*)(answer.rowIDValues[0].value.data());

	//Required
	answer = _repositories[5]->query(query);
	def.Required = *(bool*)(answer.rowIDValues[0].value.data());

	return def;
}

void WorkerHandler::CheckState()
{
	//if (_state == WorkerState.
}

void WorkerHandler::prepare(PrepareResults& _return, const TransactionID transactionID, const ColumnPrepares& columns) 
{
	CheckState();

	// Your implementation goes here
	printf("Prepare\n");
}

void WorkerHandler::apply(PrepareResults& _return, const TransactionID transactionID, const ColumnIDs& columnIDs) 
{
	CheckState();

	// Your implementation goes here
	printf("Apply\n");
}

void WorkerHandler::commit(const TransactionID transactionID, const Writes& writes) 
{
	bool syncSchema = false;

	for (auto write = writes.begin(); write != writes.end(); ++write)
	{
		fastore::communication::ColumnID id = write->first;

		// If pod or column table changes, check if we should update.
		if (id == 400 || id == 401)
			syncSchema = true;

		//If we've worked through all the system tables, we may have encountered includes
		//That would cause our current pod to instantiate a new repo. This creates the repo
		//before continuing on with the apply, since both the creation of a repo and the first
		//data may be in the same transaction. If we try to insert without creating it... bad things happen.
		auto repo = _repositories.find(id);
		if (repo != _repositories.end())
		{
			if (syncSchema && id > 401)
			{
				syncSchema = false;
				SyncToSchema();
			}

			const ColumnWrites& colwrites = write->second;
			repo->second->apply(transactionID, colwrites);
		}
	}

	if (syncSchema)
		SyncToSchema();
}

void WorkerHandler::rollback(const TransactionID transactionID) 
{
// Your implementation goes here
printf("Rollback\n");
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
	for (size_t i = 0; i < columnIDs.size(); i++)
	{		
		auto iter = _repositories.find(columnIDs[i]);

		if (iter != _repositories.end())
		{
			boost::shared_ptr<Repository> repo = _repositories[columnIDs[i]];
			Statistic stat = repo->getStatistic();
			_return.push_back(stat);
		}
		else
		{
			Statistic stat;
			_return.push_back(stat);
		}
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
	//_currentConnection->park();

	//SHUT DOWN EVERYTHING!! FAIL FAST FAIL HARD! KILL THE HIVE IF SOMETHING GOES WRONG!
	//(Not really, just testing things)
	//_currentConnection->getServer()->stop();
}

void WorkerHandler::shutdown()
{
	_currentConnection->getServer()->shutdown();
}

void WorkerHandler::loadBegin(const ColumnID columnID)
{
}

void WorkerHandler::loadBulkWrite(const ColumnID columnID, const ValueRowsList& values)
{
}

void WorkerHandler::loadWrite(const ColumnID columnID, const ColumnWrites& writes)
{
}

void WorkerHandler::loadEnd(const ColumnID columnID, const Revision revision)
{
}

void* WorkerHandler::getContext(const char* fn_name, void* serverContext)
{
	_currentConnection = (apache::thrift::server::TFastoreServer::TConnection*)serverContext;
	return NULL;
}
