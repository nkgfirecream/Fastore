#pragma once
#include "LogConstants.h"
#include "LogStream.h"
#include "LogReaderPool.h"
#include <map>
#include <hash_set>
#include <string>
#include <boost/asio/io_service.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread.hpp>
#include <boost/circular_buffer.hpp>
#include <Communication/Comm_types.h>
#include "TFastoreServer.h"

struct LogFile
{
	std::string filename;
	int64_t lsn;
	bool complete;
	//bool closed; Need some way to signal the physical file can be resused.
	int64_t size;
};

//Values are important for comparisons here.
enum TransactionState
{
	Begin = 10,
	End = 20,
	Rollback = 30
};

struct TransactionInfo
{
	int64_t transactionId;
	std::map<fastore::communication::ColumnID, fastore::communication::TransactionID> revisions; //Column ID and Revision
	TransactionState state;
};

struct RevisionFileInfo
{
	int64_t lsn;
	int64_t startingRevision;
	std::vector<int64_t> offsets;
};

struct ColumnInfo
{
	int64_t columnId;
	std::vector<RevisionFileInfo> revisions;
};

//Dummy structs for fleshing out API
struct Change {};
struct Read {};
struct Connection;

//enum LogManagerState
//{
//	Stopped,
//	Initializing,
//	Running
//};

class LogManager
{
private:
	const static int MAXTHREADS = 3;

	//protects structures shared by worker threads
	//file index, trasaction and revision index, cache
	boost::shared_ptr<boost::mutex> _lock;

	//Index of physical on-disk log files and their properties
	std::map<int64_t, LogFile> _files;

	//Index of transaction info (by transaction id)
	std::map<int64_t, TransactionInfo> _transactions;

	//Transactions that have started, but have not yet been flushed to disk.
	std::hash_set<fastore::communication::TransactionID> _pendingTransactions;

	//Index of revision info (by column id)
	std::map<int64_t, ColumnInfo> _columns;

	//Thread pool that does reads only.
	boost::asio::io_service _readService;
	boost::thread_group _readThreadPool;
	boost::asio::io_service::work _readWork;

	//We could potentially have multiple writer threads, though right now code assumes just one.
	//We'll have to revisit writer synchronization if we do.
	boost::asio::io_service _writeService;
	boost::thread_group _writeThreadPool;
	boost::asio::io_service::work _writeWork;

	LogReaderPool _readerPool;
	std::unique_ptr<LogWriter> _writer;
	
	std::string _path;

	//functions for creating, destroying, and validating readers (Used by reader pool)
	std::shared_ptr<LogReader> createReader(int64_t lsn);
	void destroyReader(std::shared_ptr<LogReader> reader);
	bool validateReader(std::shared_ptr<LogReader> reader);

	//The following are internal functions that actually do work. They are called by the various worker threads.

	//These are write functions that should be accessed only from the write thread.
	//They either modify shared state or write to disk, and their operations should be serialized.
	void initalizeLogManager();

	void indexLogFile(std::string filename);
	void indexRevisionRecord(RevisionRecord& record);
	void indexCheckpointRecord(CheckpointRecord& record);
	void indexTransactionBeginRecord(TransactionBeginRecord& record);
	void indexTransactionEndRecord(TransactionEndRecord&  record);
	void indexRollbackRecord(RollbackRecord& record);

	//Returns -1 if Lsn is not present, otherwise returns index of RevisionFileInfo
	size_t revisionFileInfoByLsn(ColumnInfo& info, int64_t lsn);

	//outLSN set to -1 if revision is not present, otherwise sets lsn and offset
	void offsetByRevision(ColumnInfo& info, int64_t revision, int64_t& outLsn, int64_t& outOffset);

	//Write functions 
	void internalFlush(const fastore::communication::TransactionID transactionID, int64_t connectionID);
	void internalCommit(const fastore::communication::TransactionID transactionID, const std::map<fastore::communication::ColumnID, fastore::communication::Revision> revisions, const fastore::communication::Writes writes);
	
	void writeTransactionBegin(const fastore::communication::TransactionID transactionID, const std::map<fastore::communication::ColumnID, fastore::communication::Revision> revisions);
	void writeRevision(const fastore::communication::ColumnID columnId, const fastore::communication::Revision revision, const fastore::communication::ColumnWrites& writes);
	void writeTransactionEnd(const fastore::communication::TransactionID transactionID);

	//Checks and makes sure there's enough room available in the log file
	//to write size bytes. If not, completes it and opens a new file.
	void prepLog(int64_t size);
	void completeCurrentLog();
	void createLogFile();
	//return value is >= 0 if a file can be reused. Lsn is always set to next Lsn.
	int64_t reuseFileOldLsn(int64_t& outNewLsn);

	//These are read-only functions that can be accessed from any thread.
	void internalGetWrites(const fastore::communication::Ranges ranges, int64_t connectionID);
	
	fastore::communication::GetWritesResult getRange(fastore::communication::ColumnID columnId, fastore::communication::Revision from, fastore::communication::Revision to);

public:

	LogManager(std::string path);
	~LogManager();

	//Public functions used to schedule work.
	//TODO: Consider shared pointers. It's possible the server will dispose of an
	//unused connection.
	void flush(const fastore::communication::TransactionID transactionID, int64_t connectionID);
	void commit(const fastore::communication::TransactionID transactionID, const std::map<fastore::communication::ColumnID, fastore::communication::Revision> & revisions, const fastore::communication::Writes& writes);
	void getWrites(const fastore::communication::Ranges& ranges, int64_t connectionID);
	//void saveThrough(std::vector<Change>, int64_t revision, int64_t columnId);

	//void getThrough(int64_t columnId, int64_t revision,  Connection* connection);
	//void getChanges(int64_t fromRevision, int64_t toRevision, std::vector<Read> reads,  Connection* connection);
};
