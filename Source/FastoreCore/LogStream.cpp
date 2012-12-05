#include "LogStream.h"
#include <time.h>

//LogStream
LogStream::LogStream() : _lsn(0), _complete(0) { }
LogStream::LogStream(int lsn) : _lsn(lsn), _complete(0) { }

void LogStream::readLogHeader()
{
	_file.seekg(0, std::ios_base::beg);
	//TODO: Test for incomplete header, checksum.
	std::string sig;
	_file >> sig;
	if (strcmp(sig.c_str(), Signature) != 0)
		throw "Signature mismatch. File isn't fastore log!";

	int version;
	_file >> version;
	if (version != Version)
		throw "Version mistmatch: Upgrade log file before using";

	_file >> _lsn;		
	_file >> _timeStamp;

	int salt;
	_file >> salt;

	int headerSize;
	_file >> headerSize;


	_file >> _complete;
	_file >> _size;

	//Padding...

	_file.seekg(headerSize - 16, std::ios_base::beg);

	//Read checksum and test it
	//md4 checksum
	//_file >> checksum;

	// if checksum != checksum of read so far, bad header.

	//Remove this once checksum is actually read, this just positions
	//us where we would be if we had read the MD4
	_file.seekg(16, std::ios_base::cur);

	//Now positioned at first entry
}

Record* LogStream::readRecord(int offset, bool headerOnly)
{
	_file.seekg(offset, std::ios_base::beg);
	return readNextRecord(headerOnly);
}

//TODO: exceptions/error messages.
Record* LogStream::readNextRecord(bool headerOnly)
{
	if (!_file.eof() && _file.tellg() <= _size)
	{
		int type;
		_file >> type;

		switch((RecordType)type)
		{
			case RecordType::TransactionBegin:
				return readTransactionBegin();
				break;
			case RecordType::TransactionEnd:
				return readTransactionEnd();
				break;
			case RecordType::Revision:
				return readRevision(headerOnly);
				break;
			case RecordType::Checkpoint:
				return readCheckpoint();
				break;
			case RecordType::Rollback:
				return readRollBack();
				break;
			default:
				//NULL return indicates invalid record.
				return NULL;
				break;
		}
	}

	return NULL;
}

TransactionBeginRecord* LogStream::readTransactionBegin()
{
	int64_t offset =  (int64_t)_file.tellg() - 4;

	//TODO: Transaction struct?
	int64_t transactionId;
	_file >> transactionId;

	int64_t timeStamp;
	_file >> timeStamp;

	int size;
	_file >> size;

	std::vector<std::pair<int64_t, int64_t>> revisions(size);

	for(int i = 0; i < size; ++i)
	{
		int64_t columnID;
		_file >> columnID;
		
		int64_t revision;
		_file >> revision;

		revisions[i] = std::make_pair(columnID, revision);
	}

	//TODO: MD4
	_file.seekg(16, std::ios_base::cur);

	TransactionBeginRecord* record = new TransactionBeginRecord(timeStamp, _lsn, offset, transactionId, revisions);

	return record;
}

TransactionEndRecord* LogStream::readTransactionEnd()
{
	int64_t offset =  (int64_t)_file.tellg() - 4;

	int64_t transactionId;
	_file >> transactionId;

	int64_t timeStamp;
	_file >> timeStamp;
	
	//TODO: MD4
	_file.seekg(16, std::ios_base::cur);

	TransactionEndRecord* record = new TransactionEndRecord(timeStamp, _lsn, offset, transactionId);

	return record;
}

RevisionRecord* LogStream::readRevision(bool headerOnly)
{
	int64_t offset =  (int64_t)_file.tellg() - 4;

	int64_t columnID;
	_file >> columnID;
		
	int64_t revision;
	_file >> revision;

	int64_t timeStamp;
	_file >> timeStamp;

	int size;
	_file >> size;

	char* buffer = new char[size];

	_file.read(buffer, size);

	//TODO: MD4
	_file.seekg(16, std::ios_base::cur);

	RevisionRecord*  record = new RevisionRecord(timeStamp, _lsn, offset, headerOnly ? 0 : size, headerOnly ? NULL : buffer);

	return record;
}

CheckpointRecord* LogStream::readCheckpoint()
{
	int64_t offset =  (int64_t)_file.tellg() - 4;

	int64_t columnID;
	_file >> columnID;
		
	int64_t revision;
	_file >> revision;

	int64_t timeStamp;
	_file >> timeStamp;

	//TODO: MD4
	_file.seekg(16, std::ios_base::cur);

	CheckpointRecord* record = new CheckpointRecord(timeStamp, _lsn, offset, columnID, revision);

	return record;
}

RollbackRecord* LogStream::readRollBack()
{
	int64_t offset =  (int64_t)_file.tellg() - 4;

	int64_t transactionId;
	_file >> transactionId;

	int64_t timeStamp;
	_file >> timeStamp;

	//TODO: MD4
	_file.seekg(16, std::ios_base::cur);

	RollbackRecord* record = new RollbackRecord(timeStamp, _lsn, offset, transactionId);

	return record;
}
	
bool LogStream::isComplete()
{
	return _complete == 1;
}

int  LogStream::lsn()
{
	return _lsn;
}

int64_t LogStream::size()
{
	return _size;
}

int64_t LogStream::timeStamp()
{
	return _timeStamp;
}

void LogStream::close()
{
	_file.close();
}

//LogReader
LogReader::LogReader(std::string filename)
{
	//Open existing file - binary mode, input, start at beginning of file.
	_file.open(filename.c_str(), std::ios::binary | std::ios::in);

	readLogHeader();
}


//LogWriter
LogWriter::LogWriter(std::string filename)
{
	_file.open(filename.c_str(), std::ios::binary | std::ios::in | std::ios::out);

	readLogHeader();
}

LogWriter::LogWriter(std::string filename, int lsn) : LogStream(lsn)
{
	_file.open(filename.c_str(), std::ios::binary | std::ios::in | std::ios::out);

	zeroFile();
	writeLogHeader();
}

void LogWriter::setComplete()
{
	_complete = 1;
}

//Need some sort of confirmation that we actually made it to disk.
void LogWriter::flush()
{
	_file.flush();
}

void LogWriter::zeroFile()
{
	_file.seekp(0, std::ios_base::beg);
	for (int i = 0; i < MaxLogSize; ++i)
	{
		_file << char(0);
	}
}

void LogWriter::writeLogHeader()
{
	_file.seekp(0, std::ios_base::beg);
	_file << Signature;
	_file << Version;
	_file << _lsn;
	_file << _complete;
	_file << (int64_t)time(NULL);

	//TODO: Seed only once...
	srand((unsigned int)time(NULL));

	_file << rand();
	_file << LogHeaderSize;

	_file << _complete;
	_file << _size;

	//TODO: Write Md4

	//Set position at first record
	_file.seekp(LogHeaderSize, std::ios_base::beg);
	_size = _file.tellp();
}

void LogWriter::updateLogHeader()
{
	//skip rewriting first values 
	_file.seekp(32, std::ios_base::beg);
	
	_file << _complete;
	_file << _size;

	//TODO: Recompute MD4 hash.

	//Skip writing timestamp, unless we want to it to indicate last update instead of creation?
	_file.seekp(_size, std::ios_base::cur);
}

void LogWriter::writeTransactionBegin(int64_t transactionId, std::vector<std::pair<int64_t, int64_t>>& revisions)
{
	_file.seekp(_size, std::ios_base::beg);
	_file << (int)RecordType::TransactionBegin;

	_file << transactionId;
	_file << (int64_t)time(0);

	_file << revisions.size();

	for (int i = 0; i < revisions.size(); ++i)
	{
		_file << revisions[i].first;
		_file << revisions[i].second;
	}

	//TODO: Write Md4
	_file.seekp(16, std::ios_base::cur);

	_size = _file.tellp();
}

void LogWriter::writeTransactionEnd(int64_t transactionId)
{
	_file.seekp(_size, std::ios_base::beg);
	_file << (int)RecordType::TransactionEnd;

	_file << transactionId;
	_file << (int64_t)time(0);

	//TODO: Write Md4
	_file.seekp(16, std::ios_base::cur);

	_size = _file.tellp();
}

void LogWriter::writeRevision(int64_t columnId, int64_t revision, char* buffer, int bufferlength)
{
	_file.seekp(_size, std::ios_base::beg);
	_file << (int)RecordType::Revision;

	_file << columnId;
	_file << revision;

	_file << (int64_t)time(0);

	_file << bufferlength;
	_file.write(buffer, bufferlength);

	//TODO: Write Md4
	_file.seekp(16, std::ios_base::cur);

	_size = _file.tellp();
}

void LogWriter::writeCheckpoint(int64_t columnId, int64_t revision)
{
	_file.seekp(_size, std::ios_base::beg);
	_file << (int)RecordType::Checkpoint;

	_file << columnId;
	_file << revision;

	_file << (int64_t)time(0);

	//TODO: Write Md4
	_file.seekp(16, std::ios_base::cur);

	_size = _file.tellp();
}

void LogWriter::writeRollback(int64_t transactionId)
{
	_file.seekp(_size, std::ios_base::beg);
	_file << (int)RecordType::Rollback;

	_file << transactionId;
	_file << (int64_t)time(0);

	//TODO: Write Md4
	_file.seekp(16, std::ios_base::cur);

	_size = _file.tellp();
}

void LogWriter::close()
{
	updateLogHeader();
	flush();
	LogStream::close();
}