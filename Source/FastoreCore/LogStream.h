#pragma once
#include <string>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include "LogConstants.h"
#include "Record.h"

class LogStream
{
protected:
	std::fstream _file;
	std::string _filename;
	int64_t _lsn;
	int _complete;
	int64_t _timeStamp;
	int64_t _size;

	LogStream();
	LogStream(int64_t lsn);
	~LogStream();

	void readLogHeader();

public:

	TransactionBeginRecord readTransactionBegin();
	TransactionEndRecord readTransactionEnd();
	CheckpointRecord readCheckpoint();
	RollbackRecord readRollBack();
	RevisionRecord readRevision();
	RecordType readNextRecordType();

	bool isComplete();
	int64_t  lsn();
	int64_t size();
	int64_t timeStamp();
};

class LogWriter : public LogStream
{

public:
	//Open file for reads and writes.
	LogWriter(std::string filename);	

	//Zero file and write new header (create file if it doesn't exist)
	LogWriter(std::string filename, int64_t lsn);

	~LogWriter();


	void writeTransactionBegin(int64_t transactionId, const std::map<fastore::communication::ColumnID, fastore::communication::TransactionID>& revisions);
	void writeTransactionEnd(int64_t transactionId);

	void writeRevision(int64_t columnId, int64_t revision, char* buffer, int bufferlength);
	void writeCheckpoint(int64_t columnId, int64_t revision);
	void writeRollback(int64_t transactionId);

	//Mark log as full
	void setComplete();

	void flush();
	void close();

private:
	void zeroFile();
	void writeLogHeader();
	void updateLogHeader();

};

//This class represents a logfile on disk, which will be managed by a log manager
class LogReader : public LogStream
{

public:
	//Open existing log file.
	LogReader(std::string filename);

	//Update the reader size limit. This happens after a log file has been flushed
	//Notify readers that there is more to read.
	void setSize(int64_t size);
	void seekReadToOffset(int64_t offset);
};


