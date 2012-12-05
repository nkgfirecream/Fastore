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
	int _lsn;
	int _complete;
	int64_t _timeStamp;
	int64_t _size;

	LogStream();
	LogStream(int lsn);

	TransactionBeginRecord* readTransactionBegin();
	TransactionEndRecord* readTransactionEnd();
	CheckpointRecord* readCheckpoint();
	RollbackRecord* readRollBack();
	RevisionRecord* readRevision(bool headerOnly);
	void readLogHeader();

public:

	Record* readRecord(int offset, bool headerOnly);
	Record* readNextRecord(bool headerOnly);
	
	bool isComplete();
	int  lsn();
	int64_t size();
	int64_t timeStamp();
	void close();

};

class LogWriter : public LogStream
{

public:
	//Open file for reads and writes.
	LogWriter(std::string filename);	

	//Zero file and write new header (create file if it doesn't exist)
	LogWriter(std::string filename, int lsn);	


	void writeTransactionBegin(int64_t transactionId, std::vector<std::pair<int64_t, int64_t>>& revisions);
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
};


