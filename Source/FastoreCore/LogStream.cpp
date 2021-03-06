#include "LogStream.h"
#include <time.h>
#include <Communication/Comm_types.h>

//LogStream
LogStream::LogStream() : _lsn(0), _complete(0) { }
LogStream::LogStream(int64_t lsn) : _lsn(lsn), _complete(0) { }

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

RecordType LogStream::readNextRecordType()
{
	if (!_file.eof() && _file.tellg() <= _size)
	{
		int type;
		_file >> type;

		return (RecordType)type;
	}

	return RecordType::Null;
}

TransactionBeginRecord LogStream::readTransactionBegin()
{
	int64_t offset =  (int64_t)_file.tellg() - 4;

	//TODO: Transaction struct?
	int64_t transactionId;
	_file >> transactionId;

	int64_t timeStamp;
	_file >> timeStamp;

	int size;
	_file >> size;

	std::map<fastore::communication::ColumnID, fastore::communication::TransactionID> revisions;

	for(int i = 0; i < size; ++i)
	{
		int64_t columnID;
		_file >> columnID;
		
		int64_t revision;
		_file >> revision;

		revisions[columnID] = revision;
	}

	//TODO: MD4
	_file.seekg(16, std::ios_base::cur);

	TransactionBeginRecord record;
	record.header.timeStamp = timeStamp;
	record.header.lsn = _lsn;
	record.header.offset = offset;
	record.transactionId = transactionId;
	record.revisions = revisions;

	return record;
}

TransactionEndRecord LogStream::readTransactionEnd()
{
	int64_t offset =  (int64_t)_file.tellg() - 4;

	int64_t transactionId;
	_file >> transactionId;

	int64_t timeStamp;
	_file >> timeStamp;
	
	//TODO: MD4
	_file.seekg(16, std::ios_base::cur);

	TransactionEndRecord record;
	record.header.timeStamp = timeStamp;
	record.header.lsn = _lsn;
	record.header.offset = offset;
	record.transactionId = transactionId;

	return record;
}

RevisionRecord LogStream::readRevision()
{
	int64_t offset =  (int64_t)_file.tellg() - 4;

	int64_t columnId;
	_file >> columnId;
		
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

	RevisionRecord  record;
	record.header.timeStamp = timeStamp;
	record.header.lsn = _lsn;
	record.header.offset = offset;
	record.columnId = columnId;
	record.revision = revision;
	record.data = std::string(buffer);

	delete buffer;

	return record;
}

CheckpointRecord LogStream::readCheckpoint()
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

	CheckpointRecord record;
	record.header.timeStamp = timeStamp;
	record.header.lsn = _lsn;
	record.header.offset = offset;
	record.columnId = columnID;
	record.revision = revision;

	return record;
}

RollbackRecord LogStream::readRollBack()
{
	int64_t offset =  (int64_t)_file.tellg() - 4;

	int64_t transactionId;
	_file >> transactionId;

	int64_t timeStamp;
	_file >> timeStamp;

	//TODO: MD4
	_file.seekg(16, std::ios_base::cur);

	RollbackRecord record;
	record.header.timeStamp = timeStamp;
	record.header.lsn = _lsn;
	record.header.offset = offset;
	record.transactionId = transactionId;
	
	return record;
}
	
bool LogStream::isComplete()
{
	return _complete == 1;
}

int64_t  LogStream::lsn()
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

LogStream::~LogStream()
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

void LogReader::setSize(int64_t size)
{
	_size = size;
}

void LogReader::seekReadToOffset(int64_t offset)
{
	if (offset <= _size)
	{
		_file.seekg(offset);
	}
	else
	{
		throw std::exception("Attempt to seek log reader past file!");
	}
}

//LogWriter
LogWriter::LogWriter(std::string filename)
{
	_file.open(filename.c_str(), std::ios::binary | std::ios::in | std::ios::out);

	readLogHeader();
}

LogWriter::LogWriter(std::string filename, int64_t lsn) : LogStream(lsn)
{
	_file.open(filename.c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

	zeroFile();
	writeLogHeader();
	flush();
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
	//Need a better way to do this. This is probably going to be slow. Is it even neccessary?  (collocate entire file when first zeroed vs zeroing time)
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

void LogWriter::writeTransactionBegin(int64_t transactionId, const std::map<fastore::communication::ColumnID, fastore::communication::TransactionID>& revisions)
{
	_file.seekp(_size, std::ios_base::beg);
	_file << (int)RecordType::TransactionBegin;

	_file << transactionId;
	_file << (int64_t)time(0);

	_file << revisions.size();

	for (auto begin = revisions.begin(), end = revisions.end(); begin != end; ++begin)
	{
		_file << begin->first;
		_file << begin->second;
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
	_file << (int)RecordType::ColumnRevision;

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
	_file << (int)RecordType::TransactionRollback;

	_file << transactionId;
	_file << (int64_t)time(0);

	//TODO: Write Md4
	_file.seekp(16, std::ios_base::cur);

	_size = _file.tellp();
}

LogWriter::~LogWriter()
{
	updateLogHeader();
	flush();
}