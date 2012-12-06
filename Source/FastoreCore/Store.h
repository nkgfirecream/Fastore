#pragma once

#include <Communication/Store.h>
#include <Communication/Comm_types.h>

class StoreHandler : virtual public fastore::communication::StoreIf, virtual public apache::thrift::TProcessorEventHandler 
{
public:
	StoreHandler();

	void checkpointBegin(const ColumnID columnID);
	void checkpointWrite(const ColumnID columnID, const ValueRowsList& values);
	void checkpointEnd(const ColumnID columnID);

	void getStatus(StoreStatus& _return);

	void getWrites(GetWritesResults& _return, const Ranges& ranges);

	void commit(const TransactionID transactionID, const Writes& writes);

	void flush(const TransactionID transactionID);
};
