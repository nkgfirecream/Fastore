#include "LogManager.h"

LogManager::LogManager(std::string config) :
	_readService(MAXTHREADS),
	_readWork(_readService),
	_writeService(1),
	_writeWork(_writeService),
	_config(config),
	_lock(boost::shared_ptr<boost::mutex>(new boost::mutex())),
	_readerPool
	(
		[&](int lsn) { return createReader(lsn); }, 
		[&](std::shared_ptr<LogReader> reader) { return validateReader(reader); }, 
		[&](std::shared_ptr<LogReader> reader) { return destroyReader(reader);  }
	)
{
	//Start writer thread.
	_writeThreadPool.create_thread(boost::bind(&boost::asio::io_service::run, &_writeService));

	//Start initialization (just call initializeLog to do this on foreground thread)
	_writeService.post(boost::bind(&LogManager::initalizeLogManager, this));

	//Reader threads aren't started until initialization is complete.
	//In theory, you could begin queueing up read work immediately, but it's probably best to wait until 
	//the threads are started
	//TODO: add a state variable.
}

LogManager::~LogManager()
{
	_readService.stop();
	_readThreadPool.join_all();

	_writeService.stop();
	_writeThreadPool.join_all();
}

std::shared_ptr<LogReader> LogManager::createReader(int lsn)
{
	//look up filename in index -- if not present throw exception
	std::string fn;

	return std::shared_ptr<LogReader>(new LogReader(fn));
}

bool LogManager::validateReader(std::shared_ptr<LogReader> reader)
{
	//check that the lsn is still live (we haven't closed the file)

	//update reader size to match last flush

	return true;
}

void LogManager::destroyReader(std::shared_ptr<LogReader> reader)
{
	reader->close();
	reader.reset();
}

void LogManager::initalizeLogManager()
{
	//Find all files in directory that are logs.

	//Scan all files, creating index.

	//Create a writer at the last LSN.

	//Start reader threads..
	for (int i = 0; i < MAXTHREADS; i++)
	{
		_readThreadPool.create_thread(boost::bind(&boost::asio::io_service::run, &_readService));
	}
}

void LogManager::flush(int64_t transactionId, Connection* connection)
{
	//connection->park();
	_writeService.post(boost::bind(&LogManager::internalFlush, this, transactionId, connection));
}

void LogManager::commit(int64_t transactionId, std::vector<Change> changes)
{
	_writeService.post(boost::bind(&LogManager::internalCommit, this, transactionId, changes));
}

void LogManager::saveThrough(std::vector<Change> changes, int64_t revision, int64_t columnId)
{
	_writeService.post(boost::bind(&LogManager::internalSaveThrough, this, changes, revision, columnId));
}

void LogManager::getThrough(int64_t columnId, int64_t revision,  Connection* connection)
{
	//connection->park();
	_readService.post(boost::bind(&LogManager::internalGetThrough, this, columnId, revision, connection));
}

void LogManager::getChanges(int64_t fromRevision, int64_t toRevision, std::vector<Read> reads,  Connection* connection)
{
	//connection->park();
	_readService.post(boost::bind(&LogManager::internalGetChanges, this, fromRevision, toRevision, reads, connection));
}

void LogManager::internalFlush(int64_t transactionId, Connection* connection)
{



	//Post result to connection buffer...
	//connection->postToBuffer();
	//Unpark connection
	//connection->unpark();
}

void LogManager::internalCommit(int64_t transactionId, std::vector<Change> changes)
{

}

void LogManager::internalSaveThrough(std::vector<Change>, int64_t revision, int64_t columnId)
{

}


void LogManager::internalGetThrough(int64_t columnId, int64_t revision,  Connection* connection)
{


	//Post result to connection buffer...
	//connection->postToBuffer();
	//Unpark connection
	//connection->unpark();
}

void LogManager::internalGetChanges(int64_t fromRevision, int64_t toRevision, std::vector<Read> reads,  Connection* connection)
{


	//Post result to connection buffer...
	//connection->postToBuffer();
	//Unpark connection
	//connection->unpark();
}

