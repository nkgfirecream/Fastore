#pragma once
#include "../FastoreCommunication/Worker.h"
#include "../FastoreCommunication/Comm_types.h"
#include <thrift/TProcessor.h>
#include <hash_map>
#include "Repository.h"
#include "Scheduler.h"
#include "TFastoreServer.h"

class WorkerHandler : virtual public fastore::communication::WorkerIf , virtual public apache::thrift::TProcessorEventHandler
{
private: 
	fastore::communication::PodID _podId;
	std::string _path;
	std::hash_map<fastore::communication::ColumnID, boost::shared_ptr<Repository>> _repositories;
	boost::shared_ptr<Scheduler> _scheduler;

	//Not a shared pointer because we don't own the connection.
	apache::thrift::server::TFastoreServer::TConnection* _currentConnection;

	void CheckState();
	void Bootstrap();
	void SyncToSchema();

	void CreateRepo(ColumnDef def);

	void AddColumnToSchema(ColumnDef def);
	ColumnDef GetDefFromSchema(ColumnID id);

	ScalarType GetTypeFromName(std::string typeName);
	
public:
	WorkerHandler(const PodID podId, const string path, const boost::shared_ptr<Scheduler>);

	Revision prepare(const fastore::communication::TransactionID& transactionID, const fastore::communication::Writes& writes, const fastore::communication::Reads& reads);
	void apply(fastore::communication::TransactionID& _return, const fastore::communication::TransactionID& transactionID, const fastore::communication::Writes& writes);
	void commit(const fastore::communication::TransactionID& transactionID);
	void rollback(const fastore::communication::TransactionID& transactionID);
	void flush(const fastore::communication::TransactionID& transactionID);
	bool doesConflict(const fastore::communication::Reads& reads, const fastore::communication::Revision source, const fastore::communication::Revision target);
	void update(fastore::communication::TransactionID& _return, const fastore::communication::TransactionID& transactionID, const fastore::communication::Writes& writes, const fastore::communication::Reads& reads);
	void transgrade(fastore::communication::Reads& _return, const fastore::communication::Reads& reads, const fastore::communication::Revision source, const fastore::communication::Revision target);
	void query(fastore::communication::ReadResults& _return, const fastore::communication::Queries& queries);
	void getStatistics(std::vector<Statistic> & _return, const std::vector<ColumnID> & columnIDs);
	void getState(WorkerState& _return);	

	//Admin
	void shutdown();

	//TProcessorEventHandler
	void handlerError(void* ctx, const char* fn_name);
	void* getContext(const char* fn_name, void* serverContext);
};