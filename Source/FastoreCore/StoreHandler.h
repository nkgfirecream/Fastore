#pragma once
#include <Communication/Store.h>
#include <Communication/Comm_types.h>
#include <thrift/TProcessor.h>
#include "Scheduler.h"
#include "LogManager.h"

class StoreHandler : 
	virtual public fastore::communication::StoreIf, 
	virtual public apache::thrift::TProcessorEventHandler 
{
public:
	StoreHandler(std::string path);

	void checkpointBegin(const fastore::communication::ColumnID columnID);
	void checkpointWrite(const fastore::communication::ColumnID columnID, const fastore::communication::ValueRowsList& values);
	void checkpointEnd(const fastore::communication::ColumnID columnID);

	void getStatus(fastore::communication::StoreStatus& _return);

	void getWrites(fastore::communication::GetWritesResults& _return, const fastore::communication::Ranges& ranges);

	void commit(const fastore::communication::TransactionID transactionID, const fastore::communication::Writes& writes);

	void flush(const fastore::communication::TransactionID transactionID);

private:
	std::unique_ptr<LogManager> _logManager;
};
