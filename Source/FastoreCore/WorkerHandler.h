#pragma once
#include <Communication/Worker.h>
#include <Communication/Comm_types.h>
#include "Wal/Wal.h"
#include <thrift/TProcessor.h>
#include <unordered_map>
#include "Repository.h"
#include "Scheduler.h"
#include "TFastoreServer.h"

class WorkerHandler :
	virtual public fastore::communication::WorkerIf,
	virtual public apache::thrift::TProcessorEventHandler
{
private: 
	fastore::communication::PodID _podId;
	std::unordered_map<fastore::communication::ColumnID, boost::shared_ptr<Repository>> _repositories;

	//Not a shared pointer because we don't own the connection.
	apache::thrift::server::TFastoreServer::TConnection* _currentConnection;

	void CheckState();
	void Bootstrap();
	void SyncToSchema();

	void CreateRepo(ColumnDef def);

	void AddColumnToSchema(ColumnDef def);
	ColumnDef GetDefFromSchema(ColumnID id);
	
public:
	WorkerHandler(const PodID podId);

	~WorkerHandler();

	//Admin
	void shutdown();
	void loadBegin(const ColumnID columnID);
	void loadBulkWrite(const ColumnID columnID, const ValueRowsList& values);
	void loadWrite(const ColumnID columnID, const ColumnWrites& writes);
	void loadEnd(const ColumnID columnID, const Revision revision);

	void getState(WorkerState& _return) override;	

	void prepare
	(	
		fastore::communication::PrepareResults& _return, 
		const fastore::communication::TransactionID transactionID, 
		const fastore::communication::ColumnPrepares& columns
	);

	void apply
	(
		fastore::communication::PrepareResults& _return, 
		const fastore::communication::TransactionID transactionID, 
		const ColumnIDs& columnIDs
	);

	void commit(const fastore::communication::TransactionID transactionID, const fastore::communication::Writes& writes);

	void rollback(const fastore::communication::TransactionID transactionID);

	void query(fastore::communication::ReadResults& _return, const fastore::communication::Queries& queries);

	void getStatistics(std::vector<Statistic> & _return, const std::vector<ColumnID> & columnIDs);

	//TProcessorEventHandler
	void handlerError(void* ctx, const char* fn_name);
	void* getContext(const char* fn_name, void* serverContext);
};
