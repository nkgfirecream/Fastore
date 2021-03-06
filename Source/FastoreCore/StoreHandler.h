#pragma once
#include <Communication/Store.h>
#include <Communication/Comm_types.h>
#include <thrift/TProcessor.h>
#include "Scheduler.h"
#include "LogManager.h"
#include "TFastoreServer.h"

class StoreHandler : 
	virtual public fastore::communication::StoreIf, 
	virtual public apache::thrift::TProcessorEventHandler 
{
public:
	StoreHandler(std::string path, uint64_t port);

	void checkpointBegin(const fastore::communication::ColumnID columnID);
	void checkpointWrite(const fastore::communication::ColumnID columnID, const fastore::communication::ValueRowsList& values);
	void checkpointEnd(const fastore::communication::ColumnID columnID);

	void getStatus(fastore::communication::StoreStatus& _return);

	void getWrites(fastore::communication::GetWritesResults& _return, const fastore::communication::Ranges& ranges);

	void commit(const fastore::communication::TransactionID transactionID, const std::map<fastore::communication::ColumnID, fastore::communication::Revision> & revisions, const fastore::communication::Writes& writes);

	void flush(const fastore::communication::TransactionID transactionID);

	void unpark(const int64_t connectionId, const std::string&  data); 

	void* getContext(const char* fn_name, void* serverContext);

	void heartbeat();
	void start();
    void suspend();

private:
	std::unique_ptr<LogManager> _logManager;
	uint64_t _port;

	enum internalStoreStatus { stopped, processing, logerror };

	internalStoreStatus _status;

	//Not a shared pointer because we don't own the connection.
	apache::thrift::server::TFastoreServer::TConnection* _currentConnection;

};
