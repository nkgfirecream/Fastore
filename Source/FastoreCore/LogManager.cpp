#include "LogManager.h"
#include <boost\filesystem.hpp>

LogManager::LogManager(std::string config) :
	_readService(MAXTHREADS),
	_readWork(_readService),
	_writeService(1),
	_writeWork(_writeService),
	_config(config),
	_lock(boost::shared_ptr<boost::mutex>(new boost::mutex())),
	_readerPool
	(
		[&](int64_t lsn) { return createReader(lsn); }, 
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
}

LogManager::~LogManager()
{
	_readService.stop();
	_readThreadPool.join_all();

	_writeService.stop();
	_writeThreadPool.join_all();
}

std::shared_ptr<LogReader> LogManager::createReader(int64_t lsn)
{
	//look up filename in index -- if not present throw exception

	_lock->lock();
	auto entry = _files.find(lsn);
	if (entry == _files.end())
	{
		_lock->unlock();
		std::stringstream ss;
		ss << "Can't file a log file that corresponds to LSN: " << lsn;
		throw std::exception(ss.str().c_str());
	}
	else
	{
		std::string fn = entry->second.filename;
		_lock->unlock();
		return std::shared_ptr<LogReader>(new LogReader(fn));
	}
}

bool LogManager::validateReader(std::shared_ptr<LogReader> reader)
{
	_lock->lock();
	//check that the lsn is still live (we haven't closed the file)
	auto entry = _files.find(reader->lsn());
	if (entry == _files.end())
	{
		_lock->unlock();
		return false;
	}

	//update reader size to match last flush
	reader->setSize(entry->second.size);
	_lock->unlock();
	return true;
}

void LogManager::destroyReader(std::shared_ptr<LogReader> reader)
{
	reader.reset();
}

void LogManager::initalizeLogManager()
{
	//Find all files in directory that are logs.
	std::vector<std::string> filenames;

	boost::filesystem::directory_iterator end;
	for ( boost::filesystem::directory_iterator item (_config); item != end; ++item)
	{
		if (!boost::filesystem::is_regular_file(item->status()))
			continue;

		if (!(item->path().extension() == LogExtension))
			continue;
		else
			filenames.push_back(item->path().string());
	}
	

	//Scan all files, creating index.
	for (auto start = filenames.begin(), end = filenames.end(); start != end; ++start)
	{
		indexLogFile(*start);
	}

	//Create a writer at the last LSN.
	if (_files.size() > 0)
	{
		auto filename = _files.rbegin()->second.filename;
		_writer = std::unique_ptr<LogWriter>(new LogWriter(filename));
	}
	else
	{
		//Create a new file
		LogFile file;
		file.lsn = 0;
		file.complete = false;
		file.size = 0;

		std::stringstream ss;
		ss << _config << file.lsn << "." << LogExtension;

		file.filename = ss.str();

		_files[file.lsn] = file;

		_writer = std::unique_ptr<LogWriter>(new LogWriter(file.filename, file.lsn));
	}
	
	//Any modifications to shared structures past this point need to be protected.

	//Start reader threads..
	for (int i = 0; i < MAXTHREADS; i++)
	{
		_readThreadPool.create_thread(boost::bind(&boost::asio::io_service::run, &_readService));
	}
}

void LogManager::indexLogFile(std::string filename)
{
	LogWriter writer(filename);
	
	LogFile file;
	file.filename = filename;
	file.lsn = writer.lsn();
	file.complete = writer.isComplete();
	file.size = writer.size();

	_files[file.lsn] = file;

	//TODO: Add record validation and recovery
	RecordType recordType = writer.readNextRecordType();
	while(recordType != RecordType::Null)
	{
		switch(recordType)
		{
		case RecordType::Revision:
			indexRevisionRecord(writer.readRevision());
			break;
		case RecordType::Checkpoint:
			indexCheckpointRecord(writer.readCheckpoint());
			break;
		case RecordType::TransactionBegin:
			indexTransactionBeginRecord(writer.readTransactionBegin());
			break;
		case RecordType::TransactionEnd:
			indexTransactionEndRecord(writer.readTransactionEnd());
			break;
		case RecordType::Rollback:
			indexRollbackRecord(writer.readRollBack());
			break;		
		default:
			throw new std::exception(("Unknown Record Type encountered while indexing log file: " + filename).c_str()); 
		}

		recordType = writer.readNextRecordType();
	}
}

size_t LogManager::revisionFileInfoByLsn(ColumnInfo& info, int64_t lsn)
{
	//Simple linear search. Theorectically, there will only be
	//be a few log files open at any given time, so a linear
	//search should be ok.
	for (size_t i = 0; i < info.revisions.size(); i++)
	{
		if (info.revisions[i].lsn == lsn)
			return i;
	}

	return -1;
}

int64_t LogManager::offsetByRevision(ColumnInfo& info, int64_t revision)
{
	for (size_t i = 0; i < info.revisions.size(); i++)
	{
		int64_t starting = info.revisions[i].startingRevision;
		int64_t diff = revision - starting;
			if (diff < int64_t(info.revisions[i].offsets.size()) && diff >=0)
				return info.revisions[i].offsets[diff];
			else
				continue;
	}

	return -1;
}

void LogManager::indexRevisionRecord(RevisionRecord& record)
{
	//This code makes the assumption that all revisions for a column will be present in the log,
	//and in monotonically ascending order.
	ColumnInfo info;
	int64_t columnId = record.columnId;
	auto iter = _columns.find(columnId);
	if (iter == _columns.end())
		info = _columns.insert(std::make_pair(columnId, ColumnInfo())).first->second;
	else
		info = iter->second;

	info.columnId = columnId;
	
	//find current file in files list and update it. If not present, create it.
	//TODO: Consider some sort of cache or alternative search algorithm if perfomance is bad.
	int64_t index = revisionFileInfoByLsn(info, record.header.lsn);

	if(index == -1)
	{
		RevisionFileInfo fileInfo;
		fileInfo.lsn = record.header.lsn;
		fileInfo.startingRevision = record.revision;
		fileInfo.offsets.push_back(record.header.offset);
	}
	else
	{
		info.revisions[index].offsets.push_back(record.header.offset);
	}
}


void LogManager::indexCheckpointRecord(CheckpointRecord& record)
{
	//TODO: Checkpoint indexing. This will be important to determine
	//which log files can be reused or disposed.
}

void LogManager::indexTransactionBeginRecord(TransactionBeginRecord& record)
{	
	auto entry = _transactions.find(record.transactionId);
	if (entry != _transactions.end() && entry->second.columns.size() == 0)
	{
		//Add revision infomation if it's not present in the index, but don't update the state.
		//The state must be >= begin if it's present at all.
		entry->second.columns = record.revisions;
	}
	else
	{
		TransactionInfo info;
		info.state = TransactionState::Begin;
		info.columns = record.revisions;
		info.transactionId = record.transactionId;
		_transactions.insert(std::make_pair(info.transactionId, info));
	}
}

void LogManager::indexTransactionEndRecord(TransactionEndRecord& record)
{
	auto entry = _transactions.find(record.transactionId);
	if (entry != _transactions.end() && entry->second.state < TransactionState::End)
	{
		//Update state infomation it's < End (i.e. Begin)
		entry->second.state = TransactionState::End;
	}
	else
	{
		//Add a new transaction entry if it's not present. Will be missing revision infomation
		//until its indexed by scanning further log files.
		//If no further entries are present, then that log file was probably
		//disposed of at some point. 
		TransactionInfo info;
		info.state = TransactionState::End;
		info.transactionId = record.transactionId;
		_transactions.insert(std::make_pair(info.transactionId, info));
	}
}

void LogManager::indexRollbackRecord(RollbackRecord& record)
{
	auto entry = _transactions.find(record.transactionId);
	if (entry != _transactions.end() && entry->second.state < TransactionState::Rollback)
	{
		entry->second.state = TransactionState::Rollback;
	}
	else
	{
		//Add a new transaction entry if it's not present. Will be missing revision infomation
		//until its indexed by scanning further log files.
		//If no further entries are present, then that log file was probably
		//disposed of at some point. 
		TransactionInfo info;
		info.state = TransactionState::Rollback;
		info.transactionId = record.transactionId;
		_transactions.insert(std::make_pair(info.transactionId, info));
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

