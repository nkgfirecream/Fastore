#include "StoreHandler.h"

using boost::shared_ptr;

using namespace::fastore::communication;

StoreHandler::StoreHandler(std::string path, uint64_t port)
	: _logManager(new LogManager(path)),
	_status(internalStoreStatus::stopped)
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
   //Need to combine our internal status with the log's status.
	auto logStatus = _logManager->getStatus();
	if (logStatus == LogManager::logStatus::initalizing && _status == internalStoreStatus::stopped)
	{
		_return.__set_LogStatus(StoreLogStatus::Loading);
	}
	else if (logStatus == LogManager::logStatus::online && _status == internalStoreStatus::stopped)
	{
		_return.__set_LogStatus(StoreLogStatus::Ready);
	}
	else if (logStatus == LogManager::logStatus::online && _status == internalStoreStatus::processing)
	{
		_return.__set_LogStatus(StoreLogStatus::Online);
	}
	else if (logStatus == LogManager::logStatus::offline || _status == internalStoreStatus::logerror)
	{
		_return.__set_LogStatus(StoreLogStatus::Offline);
	}
	//There will be various other states like log offline, log error, etc.
	else
	{
		_return.__set_LogStatus(StoreLogStatus::Unknown);
	}


	if (_return.LogStatus == StoreLogStatus::Online || _return.LogStatus == StoreLogStatus::Ready)
	{
		//TODO: Once we have checkpoints
		//_return.__set_beganCheckpoints();
		//_return.__set_LastCheckpoints();

		auto revisions = _logManager->getLatestRevisions();
		
	
		_return.__set_LatestRevisions(revisions);		
	}	
}

void StoreHandler::getWrites(fastore::communication::GetWritesResults& _return, const fastore::communication::Ranges& ranges) 
{
	_currentConnection->park();
	_logManager->getWrites(ranges, _port, _currentConnection->getId());
}

void StoreHandler::commit(const TransactionID transactionID, const std::map<ColumnID, Revision>& revisions, const Writes& writes)
{
	if(_status == internalStoreStatus::processing)
	{
		_logManager->commit(transactionID, revisions, writes);
	}
}

void StoreHandler::flush(const fastore::communication::TransactionID transactionID) 
{
	if(_status == internalStoreStatus::processing)
	{
		_currentConnection->park();
		_logManager->flush(transactionID, _port, _currentConnection->getId());
	}
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

//Signal store manager to do maintenance tasks
void StoreHandler::heartbeat()
{
	_logManager->heartbeat();
}

//Signal store to start processing writes.
void StoreHandler::start()
{
	_status = internalStoreStatus::processing;
}

//Signal store to stop processing writes
void StoreHandler::suspend()
{
	_status = internalStoreStatus::stopped;
}
