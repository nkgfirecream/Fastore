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
	_currentConnection->park();
	_logManager->getWrites(ranges, _currentConnection->getId());
}

void StoreHandler::commit(const TransactionID transactionID, const std::map<ColumnID, Revision>& revisions, const Writes& writes)
{
	_logManager->commit(transactionID, revisions, writes);
}

void StoreHandler::flush(const fastore::communication::TransactionID transactionID) 
{
	_currentConnection->park();
	_logManager->flush(transactionID, _currentConnection->getId());
}

void StoreHandler::unpark(const int64_t connectionId, const std::string&  data)
{
	//Post data to the parked connection:
	//Going through the current connection, find the old connection and post data to it
	auto conn = _currentConnection->getServer()->getConnectionById(connectionId);
	if (conn != NULL)
	{
		conn->getOutputTransport()->resetBuffer();
		conn->getOutputTransport()->write((const uint8_t*)data.data(), (uint32_t)data.size());
		conn->transition();
	}
	else
	{
		//Operation attempted to unpark a connection that no longer existed. How should we handle this?
	}
}

void* StoreHandler::getContext(const char* fn_name, void* serverContext)
{
	_currentConnection = (apache::thrift::server::TFastoreServer::TConnection*)serverContext;
	return NULL;
}
