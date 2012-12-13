#pragma once
#include <cstdint>
#include <map>
#include <Communication/Comm_types.h>

enum RecordType
{
	Null = 0,
	TransactionBegin = 100,
	TransactionEnd = 101,
	ColumnRevision = 200,
	TransactionRollback = 300,
	Checkpoint = 400
};

struct RecordHeader
{
	int64_t timeStamp;
	int64_t lsn;
	int64_t offset;
};

struct TransactionBeginRecord
{
	RecordHeader header;
	int64_t transactionId;
	std::map<fastore::communication::ColumnID, fastore::communication::TransactionID> revisions;
};

struct TransactionEndRecord
{
	RecordHeader header;
	int64_t transactionId;
};

struct RollbackRecord
{
	RecordHeader header;
	int64_t transactionId;
};

struct RevisionRecord
{
	RecordHeader header;
	int64_t columnId;
	int64_t revision;
	std::string data;
};

struct CheckpointRecord
{
	RecordHeader header;
	int64_t columnId;
	int64_t revision;
};

