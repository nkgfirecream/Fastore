#include "LogManager.h"
#include "LogConstants.h"
#include <boost\filesystem.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <Communication/Store.h>

LogManager::LogManager(std::string path) :
	_readService(MAXTHREADS),
	_readWork(_readService),
	_writeService(1),
	_writeWork(_writeService),
	_path(path),
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
	for ( boost::filesystem::directory_iterator item (_path); item != end; ++item)
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
		createLogFile();
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
		case RecordType::ColumnRevision:
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
		case RecordType::TransactionRollback:
			indexRollbackRecord(writer.readRollBack());
			break;		
		default:
			//TODO: This should be a case where we mark the log file as corrupted or incomplete past this point.
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

void LogManager::offsetByRevision(ColumnInfo& info, int64_t revision, int64_t& outLsn, int64_t& outOffset)
{
	for (size_t i = 0; i < info.revisions.size(); i++)
	{
		int64_t starting = info.revisions[i].startingRevision;
		int64_t diff = revision - starting;
		if (diff < int64_t(info.revisions[i].offsets.size()) && diff >=0)
		{
			outOffset = info.revisions[i].offsets[diff];
			outLsn = info.revisions[i].lsn;
			return;
		}
		else
			continue;
	}

	outLsn = -1;
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

	if(index != -1)
	{
		info.revisions[index].offsets.push_back(record.header.offset);
	}
	else
	{
		RevisionFileInfo fileInfo;
		fileInfo.lsn = record.header.lsn;
		fileInfo.startingRevision = record.revision;
		fileInfo.offsets.push_back(record.header.offset);		
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
	if (entry != _transactions.end() && entry->second.revisions.size() == 0)
	{
		//Add revision infomation if it's not present in the index, but don't update the state.
		//The state must be >= begin if it's present at all.
		entry->second.revisions = record.revisions;
	}
	else
	{
		TransactionInfo info;
		info.state = TransactionState::Begin;
		info.revisions = record.revisions;
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

//Public methods
void LogManager::flush(const fastore::communication::TransactionID transactionID, int64_t connectionID)
{
	_writeService.post(boost::bind(&LogManager::internalFlush, this, transactionID, connectionID));
}
void LogManager::commit(const fastore::communication::TransactionID transactionID, const std::map<fastore::communication::ColumnID, fastore::communication::Revision> & revisions, const fastore::communication::Writes& writes)
{
	_writeService.post(boost::bind(&LogManager::internalCommit, this, transactionID, revisions, writes));
}

void LogManager::getWrites(const fastore::communication::Ranges& ranges, int64_t connectionID)
{
	_readService.post(boost::bind(&LogManager::internalGetWrites, this, ranges, connectionID));
}

//Internal methods
void LogManager::internalFlush(const fastore::communication::TransactionID transactionID, int64_t connectionID)
{
	_lock->lock();
	_writer->flush();

	auto file = _files[_writer->lsn()];
	file.size = _writer->size();

	_pendingTransactions.clear();
	_lock->unlock();


}

void LogManager::internalCommit(const fastore::communication::TransactionID transactionID, const std::map<fastore::communication::ColumnID, fastore::communication::Revision> revisions, const fastore::communication::Writes writes)
{
	writeTransactionBegin(transactionID, revisions);

	for(auto write : writes)
	{
		auto entry = revisions.find(write.first);
		if (entry == revisions.end())
		{
			throw std::exception("No revision associated with columns writes.");
		}

		writeRevision(write.first, entry->second, write.second);
	}
	writeTransactionEnd(transactionID);
}

void LogManager::internalGetWrites(const fastore::communication::Ranges ranges, int64_t connectionID)
{
	fastore::communication::GetWritesResults results;

	//Get ranges and post to result.
	for (auto range : ranges)
	{
		auto columnId = range.first;
		auto revFrom = range.second.from;
		auto revTo = range.second.to;

		//Adjust revision to what we actually have.		
		try
		{
			_lock->lock();
			auto colInfo = _columns[columnId];
			
			auto haveMin = colInfo.revisions[0].startingRevision;
			auto haveMax = colInfo.revisions[colInfo.revisions.size() - 1].startingRevision + int64_t(colInfo.revisions[colInfo.revisions.size() - 1].offsets.size()) - 1;

			if (haveMin > revFrom)
				revFrom = haveMin;

			if (haveMax < revTo)
				revTo = haveMax;

			if (revTo < revFrom)
				throw std::exception("End revision is after start revision");

			_lock->unlock();

			auto columnWrites = getRange(columnId, revFrom, revTo);
			results[columnId] = columnWrites;
		}
		catch(...)
		{
			_lock->unlock();
			//Set entry to null
			continue;
		}
	}

	boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> buffer(new apache::thrift::transport::TMemoryBuffer);

	apache::thrift::protocol::TBinaryProtocol protocol(buffer);

	fastore::communication::Store_getWrites_result wireResult;
	wireResult.__set_success(results);

	wireResult.write(&protocol);

	std::string result = buffer->getBufferAsString();

}

fastore::communication::GetWritesResult LogManager::getRange(fastore::communication::ColumnID columnId, fastore::communication::Revision from, fastore::communication::Revision to)
{
	fastore::communication::GetWritesResult result;

	result.__set_minFrom(from);
	result.__set_maxTo(to);

	std::map<fastore::communication::Revision, fastore::communication::ColumnWrites> revisions;

	for (fastore::communication::Revision rev = from; rev <= to; ++to)
	{
		int64_t lsn;
		int64_t offset;
		_lock->lock();
		offsetByRevision(_columns[columnId], rev, lsn, offset);
		_lock->unlock();

		if (lsn != -1)
		{
			auto reader = _readerPool[lsn];
			reader->seekReadToOffset(offset + 4);
			auto record = reader->readRevision();
			//Convert buffer to structure
			fastore::communication::ColumnWrites writes;
			
			//Constructing a buffer like this is read-only
			boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> readBuffer(new apache::thrift::transport::TMemoryBuffer((uint8_t*)record.data.data(), record.data.size()));
			apache::thrift::protocol::TBinaryProtocol readProtocol(readBuffer);
			writes.read(&readProtocol);
			revisions[rev] = writes;
		}
		else
		{
			throw std::exception("Missing column revision!");
		}
	}

	if (revisions.size() > 0)
		result.__set_writes(revisions);

	return result;
}

void LogManager::writeTransactionBegin(const fastore::communication::TransactionID transactionID, const std::map<fastore::communication::ColumnID, fastore::communication::Revision> revisions)
{
	TransactionInfo info;
	info.revisions = revisions;
	info.state = TransactionState::Begin;
	info.transactionId = transactionID;

	//Calculate total size of this record (to see if it will fit in a log file)
	//This the size of the header (constant size) plus two int64s for every entry in revisions
	int64_t size = TransactionBeginSize + ((sizeof(fastore::communication::ColumnID) + sizeof(fastore::communication::Revision)) * revisions.size());

	prepLog(size);
	_writer->writeTransactionBegin(transactionID, revisions);

	_lock->lock();
	_transactions[transactionID] = info;
	_pendingTransactions.insert(transactionID);	
	_lock->unlock();	
}

void LogManager::writeRevision(const fastore::communication::ColumnID columnId, const fastore::communication::Revision revision, const fastore::communication::ColumnWrites& writes)
{
	//Write structure to buffer
	boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> writeBuffer(new apache::thrift::transport::TMemoryBuffer());
	apache::thrift::protocol::TBinaryProtocol writeProtocol(writeBuffer);

	writes.write(&writeProtocol);

	uint8_t* buf;
	uint32_t size;
	writeBuffer->getBuffer(&buf, &size);

	auto totalSize = size + RevisionSize;

	prepLog(totalSize);

	int64_t offset = _writer->size();
	int64_t lsn = _writer->lsn();
	_writer->writeRevision(columnId, revision, (char *)buf, size);

	_lock->lock();
	auto column = _columns[columnId];
	if (column.revisions.size() > 0 && column.revisions[column.revisions.size() - 1].lsn == lsn)
	{
		column.revisions[column.revisions.size() - 1].offsets.push_back(offset);
	}
	else
	{
		RevisionFileInfo info;
		info.lsn = lsn;
		info.offsets.push_back(offset);
		info.startingRevision = revision;
		column.revisions.push_back(info);
	}
	_lock->unlock();
}

void LogManager::writeTransactionEnd(const fastore::communication::TransactionID transactionID)
{
	prepLog(TransactionEndSize);

	_writer->writeTransactionEnd(transactionID);

	_lock->lock();
	_transactions[transactionID].state = TransactionState::End;
	_lock->unlock();	
}

void LogManager::prepLog(int64_t size)
{
	if (_writer == NULL)
		createLogFile();

	if (_writer->size() + size > MaxLogSize)
	{
		completeCurrentLog();
		createLogFile();
	}
}

void LogManager::completeCurrentLog()
{
	_writer->setComplete();

	_lock->lock();
	auto file = _files[_writer->lsn()];
	file.complete = true;
	file.size = _writer->size();
	_lock->unlock();

	_writer.reset();
}

void LogManager::createLogFile()
{
	_lock->lock();

	int64_t newLsn;
	size_t oldLsn = reuseFileOldLsn(newLsn);

	LogFile file;
	file.lsn = newLsn;
	file.complete = false;
	file.size = 0;

	if (oldLsn >= 0)
	{
		auto oldFile = _files[oldLsn];
		file.filename = oldFile.filename;
		_files.erase(oldLsn);

	}
	else
	{
		std::stringstream ss;
		ss << _path << file.lsn << "." << LogExtension;
		file.filename = ss.str();
	}

	
	_files[newLsn] = file;
	_writer = std::unique_ptr<LogWriter>(new LogWriter(file.filename, file.lsn));

	_lock->unlock();
}

//Must be called while shared structures are proctected.
int64_t LogManager::reuseFileOldLsn(int64_t& outNewLsn)
{
	//TODO: Determine which logfile if any can be reused.
	//If none can, create a new file.

	//A file can be reused if every column it contains has a checkpoint record in a subsequent file.
	
	//For now always just use a new log file.
	auto entry = _files.rbegin();
	if (entry != _files.rend())
		outNewLsn = entry->first + 1;
	else
		outNewLsn = 0;

	return -1;
}