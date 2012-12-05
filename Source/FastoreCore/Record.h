#include <cstdint>
#include <vector>

enum RecordType
{
	TransactionBegin = 100,
	TransactionEnd = 101,
	Revision = 200,
	Rollback = 300,
	Checkpoint = 400
};

struct Record
{
protected:
	Record(RecordType type, int64_t timeStamp, int64_t lsn, int64_t offset)
		: Type(type), TimeStamp(timeStamp), LSN(lsn), Offset(offset) { }
public:
	RecordType Type;
	int64_t TimeStamp;
	int64_t LSN;
	int64_t Offset;
};

struct TransactionBeginRecord : Record
{
	TransactionBeginRecord(int64_t timeStamp, int64_t lsn, int64_t offset, int64_t transactionId, std::vector<std::pair<int64_t,int64_t>> revisions)
		: Record(RecordType::TransactionBegin, timeStamp, lsn, offset), TransactionId(transactionId), Revisions(revisions) { }

	int64_t TransactionId;
	std::vector<std::pair<int64_t,int64_t>> Revisions;
};

struct TransactionEndRecord : Record
{
	TransactionEndRecord(int64_t timeStamp, int64_t lsn, int64_t offset, int64_t transactionId)
		: Record(RecordType::TransactionEnd, timeStamp, lsn, offset), TransactionId(transactionId) { }

	int64_t TransactionId;
};

struct RollbackRecord : Record
{
	RollbackRecord(int64_t timeStamp, int64_t lsn, int64_t offset, int64_t transactionId)
		: Record(RecordType::Rollback, timeStamp, lsn, offset), TransactionId(transactionId) { }

	int64_t TransactionId;
};

//Takes ownership of data buffer. Will delete it when disposed.
struct RevisionRecord : Record
{
	RevisionRecord(int64_t timeStamp, int64_t lsn, int64_t offset, int64_t size, char* data)
		: Record(RecordType::Revision, timeStamp, lsn, offset), Size(size), Data(data) { }

	//Size == 0 and Data == NULL means this is a header-only entry. Have to revisit to actually grab the data.
	int64_t Size;
	char* Data;

	~RevisionRecord()
	{
		if (Data != NULL)
			delete Data;
	}
};

struct CheckpointRecord : Record
{
	CheckpointRecord(int64_t timeStamp, int64_t lsn, int64_t offset, int64_t columnId, int64_t revision)
		: Record(RecordType::Revision, timeStamp, lsn, offset), ColumnId(columnId), Revision(revision) { }

	int64_t ColumnId;
	int64_t Revision;
};

