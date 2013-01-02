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

#include <Schema/Dictionary.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using boost::shared_ptr;
using namespace ::fastore::communication;
using namespace ::fastore::common;
using namespace std;

const int MaxSystemColumnID = 9999;

WorkerHandler::WorkerHandler(const PodID podId) 
  : _podId(podId)
{
	Bootstrap();
}

WorkerHandler::~WorkerHandler()
{
	_repositories.clear();
}

void WorkerHandler::Bootstrap()
{
	static const ColumnDef defaults[] =  
	{ 
		{ Dictionary::ColumnID, "Column.ID", standardtypes::Long, standardtypes::Long, BufferType_t::Identity, true }, 
		{ Dictionary::ColumnName, "Column.Name", standardtypes::String, standardtypes::Long, BufferType_t::Unique, true },
		{ Dictionary::ColumnValueType, "Column.ValueType", standardtypes::String, standardtypes::Long, BufferType_t::Multi, true },
		{ Dictionary::ColumnRowIDType, "Column.RowIDType", standardtypes::String, standardtypes::Long, BufferType_t::Multi, true },
		{ Dictionary::ColumnBufferType, "Column.BufferType", standardtypes::Int, standardtypes::Long, BufferType_t::Multi, true },
		{ Dictionary::ColumnRequired, "Column.Required", standardtypes::Bool, standardtypes::Long, BufferType_t::Multi, true },
		{ Dictionary::TopologyID, "Topology.ID", standardtypes::Long, standardtypes::Long, BufferType_t::Identity, true },
		{ Dictionary::HostID, "Host.ID", standardtypes::Long, standardtypes::Long, BufferType_t::Identity, true },
		{ Dictionary::PodID, "Pod.ID", standardtypes::Long, standardtypes::Long, BufferType_t::Unique, true },
		{ Dictionary::PodHostID, "Pod.HostID", standardtypes::Long, standardtypes::Long, BufferType_t::Multi, true },
		{ Dictionary::PodColumnPodID, "PodColumn.PodID", standardtypes::Long, standardtypes::Long, BufferType_t::Multi, true },
		{ Dictionary::PodColumnColumnID, "PodColumn.ColumnID", standardtypes::Long, standardtypes::Long, BufferType_t::Multi, true },
		{ Dictionary::StashID, "Stash.ID", standardtypes::Long, standardtypes::Long, BufferType_t::Unique, true },
		{ Dictionary::StashHostID, "Stash.HostID", standardtypes::Long, standardtypes::Long, BufferType_t::Multi, true },
		{ Dictionary::StashColumnStashID, "StashColumn.StashID", standardtypes::Long, standardtypes::Long, BufferType_t::Multi, true },
		{ Dictionary::StashColumnColumnID, "StashColumn.ColumnID", standardtypes::Long, standardtypes::Long, BufferType_t::Multi, true }
	};	

	//This creates the in memory repo
	for_each( defaults, defaults + sizeof(defaults)/sizeof(defaults[0]), 
		[&](const ColumnDef& def) 
		{
			CreateRepo(def);
		} );
	
	//This must come after the repos are initialized. Can't add a column to the schema if the schema doesn't exist
	for_each( defaults, defaults + sizeof(defaults)/sizeof(defaults[0]), 
		[&](const ColumnDef& def) 
		{
			AddColumnToSchema(def);
		} );
}

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

	_repositories[fastore::common::Dictionary::ColumnID]->apply(0, writes);


	//name column
	includes.clear();

	include.__set_value(def.Name);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[fastore::common::Dictionary::ColumnName]->apply(0, writes);


	//valueType column
	includes.clear();

	include.__set_value(def.ValueType.Name);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[fastore::common::Dictionary::ColumnValueType]->apply(0, writes);


	//rowType column
	includes.clear();

	include.__set_value(def.RowIDType.Name);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[fastore::common::Dictionary::ColumnRowIDType]->apply(0, writes);


	//unique column
	includes.clear();

	std::string unique;
	unique.assign((char*)&def.BufferType, sizeof(int));

	include.__set_value(unique);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[fastore::common::Dictionary::ColumnBufferType]->apply(0, writes);	

	//required column
	includes.clear();
	std::string required;
	required.assign((char*)&def.Required, sizeof(bool));

	include.__set_value(required);
	include.__set_rowID(columnId);

	includes.push_back(include);
	writes.__set_includes(includes);

	_repositories[fastore::common::Dictionary::ColumnRequired]->apply(0, writes);
}

ColumnDef WorkerHandler::GetDefFromSchema(ColumnID id)
{
	//for the columns table, the rowId is the columnId
	std::string rowId;
	rowId.assign((char*)&id, sizeof(ColumnID));

	std::vector<std::string> rowIds;
	rowIds.push_back(rowId);

	Query query;
	query.__set_rowIDs(rowIds);	

	//Name column
	Answer answer = _repositories[fastore::common::Dictionary::ColumnName]->query(query);
	string name = answer.rowIDValues[0].value;

	//ValueType
	answer = _repositories[fastore::common::Dictionary::ColumnValueType]->query(query);
	string valueType = answer.rowIDValues.at(0).value;

	//RowType
	answer = _repositories[fastore::common::Dictionary::ColumnRowIDType]->query(query);
	string rowType = answer.rowIDValues[0].value;

	//Unique
	answer = _repositories[fastore::common::Dictionary::ColumnBufferType]->query(query);
	BufferType_t bType = (BufferType_t)*(int*)(answer.rowIDValues[0].value.data());

	//Required
	answer = _repositories[fastore::common::Dictionary::ColumnRequired]->query(query);
	bool req = *(bool*)(answer.rowIDValues[0].value.data());

	ColumnDef c = { id, name, standardtypes::GetTypeFromName(valueType), standardtypes::GetTypeFromName(rowType), bType, req };
	return c;
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

	for (auto column = columnIDs.begin(), end = columnIDs.end(); column != end; ++column)
	{
		auto repo = _repositories.find(*column);
		if (repo != _repositories.end())
		{
			PrepareResult result;
			result.__set_actualRevision(repo->second->getRevision());
			//What determines needs validation?
			_return.insert(std::make_pair(*column, result));
		}
	}

}

void WorkerHandler::commit(const TransactionID transactionID, const Writes& writes) 
{
	bool syncSchema = false;

	for (auto write = writes.begin(); write != writes.end(); ++write)
	{
		fastore::communication::ColumnID id = write->first;

		// If pod or column table changes, check if we should update.
		if (id == fastore::common::Dictionary::PodColumnPodID || id == fastore::common::Dictionary::PodColumnColumnID)
			syncSchema = true;

		//If we've worked through all the system tables, we may have encountered includes
		//That would cause our current pod to instantiate a new repo. This creates the repo
		//before continuing on with the apply, since both the creation of a repo and the first
		//data may be in the same transaction. If we try to insert without creating it... bad things happen.
		auto repo = _repositories.find(id);
		if (repo != _repositories.end())
		{
			if (syncSchema && id >  fastore::common::Dictionary::PodColumnColumnID)
			{
				syncSchema = false;
				SyncToSchema();
			}

			const ColumnWrites& colwrites = write->second;
			repo->second->apply(repo->second->getRevision() + 1, colwrites);
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
