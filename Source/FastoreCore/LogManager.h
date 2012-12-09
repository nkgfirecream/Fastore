#pragma once
#include "LogConstants.h"
#include "LogStream.h"
#include "LogReaderPool.h"
#include <map>
#include <string>
#include <boost\asio\io_service.hpp>
#include <boost\bind\bind.hpp>
#include <boost\thread.hpp>

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
	std::vector<std::pair<int64_t,int64_t>> columns; //Column ID and Revision
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
	//lsn -> file map, index, cache
	boost::shared_ptr<boost::mutex> _lock;

	//This structure keeps track of current log files
	std::map<int64_t, LogFile> _files;

	//Index of transaction info (by transaction id)
	std::map<int64_t, TransactionInfo> _transactions;

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
	
	std::string _config;

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

	//Returns -1 if revision is not present, otherwise returns offset
	int64_t offsetByRevision(ColumnInfo& info, int64_t revision);

	void internalFlush(int64_t transactionId, Connection* connection);
	void internalCommit(int64_t transactionId, std::vector<Change> changes);
	void internalSaveThrough(std::vector<Change>, int64_t revision, int64_t columnId);

	//These are read-only functions that can be accessed from any thread.
	void internalGetThrough(int64_t columnId, int64_t revision,  Connection* connection);
	void internalGetChanges(int64_t fromRevision, int64_t toRevision, std::vector<Read> reads,  Connection* connection);


public:

	LogManager(std::string config);
	~LogManager();

	//Public functions used to schedule work.
	//TODO: Consider shared pointers. It's possible the server will dispose of an
	//unused connection.
	void flush(int64_t transactionId, Connection* connection);
	void commit(int64_t transactionId, std::vector<Change> changes);
	void saveThrough(std::vector<Change>, int64_t revision, int64_t columnId);

	void getThrough(int64_t columnId, int64_t revision,  Connection* connection);
	void getChanges(int64_t fromRevision, int64_t toRevision, std::vector<Read> reads,  Connection* connection);
};