#include "StoreHandler.h"

using boost::shared_ptr;

using namespace::fastore::communication;

StoreHandler::StoreHandler(std::string path)
	: _logManager(new LogManager(path))
{
// Your initialization goes here

}

void StoreHandler::checkpointBegin(const fastore::communication::ColumnID columnID) 
{
	//TODO: Create checkpoint file, write header, prepare for more data
printf("checkpointBegin\n");	
}

void StoreHandler::checkpointWrite(const fastore::communication::ColumnID columnID, const fastore::communication::ValueRowsList& values) 
{

	//TODO: Write data to file.
printf("checkpointWrite\n");
}

void StoreHandler::checkpointEnd(const fastore::communication::ColumnID columnID) 
{
	//TODO: Close checkpoint file, writer marker to log.
printf("checkpointEnd\n");
}

void StoreHandler::getStatus(fastore::communication::StoreStatus& _return) 
{
// Your implementation goes here
printf("getStatus\n");
}

void StoreHandler::getWrites(fastore::communication::GetWritesResults& _return, const fastore::communication::Ranges& ranges) 
{
// Your implementation goes here
printf("getWrites\n");
}

void StoreHandler::commit(const fastore::communication::TransactionID transactionID, const fastore::communication::Writes& writes) 
{
	//_logManager->commit(transactionID, writes);
}

void StoreHandler::flush(const fastore::communication::TransactionID transactionID) 
{
	//_logManager->flush(transactionID);
}
