#include "StoreHandler.h"

using boost::shared_ptr;

using namespace::fastore::communication;

StoreHandler::StoreHandler(std::string path)
	: _logManager(new LogManager(path))
{
// Your initialization goes here

}

void StoreHandler::checkpointBegin(const ColumnID columnID) 
{
printf("checkpointBegin\n");	
}

void StoreHandler::checkpointWrite(const ColumnID columnID, const ValueRowsList& values) 
{
// Your implementation goes here
printf("checkpointWrite\n");
}

void StoreHandler::checkpointEnd(const ColumnID columnID) 
{
// Your implementation goes here
printf("checkpointEnd\n");
}

void StoreHandler::getStatus(StoreStatus& _return) 
{
// Your implementation goes here
printf("getStatus\n");
}

void StoreHandler::getWrites(GetWritesResults& _return, const Ranges& ranges) 
{
// Your implementation goes here
printf("getWrites\n");
}

void StoreHandler::commit(const TransactionID transactionID, const Writes& writes) 
{
// Your implementation goes here
printf("commit\n");
}

void StoreHandler::flush(const TransactionID transactionID) 
{
// Your implementation goes here
printf("flush\n");
}
