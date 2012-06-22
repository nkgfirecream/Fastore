#pragma once
#include "FastoreHost.h"
#include "../FastoreCommunication/Worker.h"
#include "../FastoreCommunication/Comm_types.h"
#include <thrift/server/TServer.h>
#include "Repository.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using boost::shared_ptr;
using namespace ::fastore::communication;
using namespace std;

class WorkerHandler : virtual public WorkerIf 
{
private: 
	PodID _podId;
	string _path;
	hash_map<ColumnID, Repository*> _repositories;
	void CheckState();
	void Bootstrap();
	void SyncToSchema();

	void CreateRepo(ColumnDef def);
	void DestroyRepo(ColumnID id);

	void AddColumnToSchema(ColumnDef def);
	
public:
	WorkerHandler(const PodID podId, const string path);

	Revision prepare(const TransactionID& transactionID, const Writes& writes, const Reads& reads);
	void apply(TransactionID& _return, const TransactionID& transactionID, const Writes& writes);
	void commit(const TransactionID& transactionID);
	void rollback(const TransactionID& transactionID);
	void flush(const TransactionID& transactionID);
	bool doesConflict(const Reads& reads, const Revision source, const Revision target);
	void update(TransactionID& _return, const TransactionID& transactionID, const Writes& writes, const Reads& reads);
	void transgrade(Reads& _return, const Reads& reads, const Revision source, const Revision target);
	void query(ReadResults& _return, const Queries& queries);
	void getStatistics(std::vector<Statistic> & _return, const std::vector<ColumnID> & columnIDs);
};