#pragma once
#include "..\FastoreCore\FastoreHost.h"
#include "..\FastoreCommunication\Worker.h"
#include "ServiceConfig.h"
#include <thrift/server/TServer.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::fastore;

class WorkerHandler : virtual public WorkerIf 
{
private: 
	FastoreHost _host;

public:
	WorkerHandler(const CoreConfig& config);

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