namespace cpp fastore
namespace csharp Alphora.Fastore

// Common

typedef i64 Revision

// Topology

typedef i32 TopologyID
typedef i32 ColumnID
typedef i32 HostID
typedef string HostAddress

struct Host
{
	1: required HostID id,

	/** Host name and optional port (e.g. "myserver:1234") */
	2: required HostAddress address,
}

struct Repository
{
	1: required ColumnID columnID,
	2: required HostID hostID,
}

struct Topology
{
	1: required TopologyID id,
	2: required set<Host> hosts,
	3: required set<Repository> repositories
}

struct TopologyResult
{
	1: required Topology topology,
	2: required Revision revision
}

// Topology Reporting

enum RepositoryStatus
{
	/** Not in-sync; don't send read or write requests. */
	Loading = 1,
	/** In the process of unloading; don't send any requests. */
	Unloading = 2,
	/** Online and servicing requests */
	Online = 3,
	/** In the process of checkpointing; can send read and write requests, but performance may be sub-optimal. */
	Checkpointing = 4
}

enum HostStatus
{
	Offline = 1,
	Online = 2,
	Unreachable = 3
}

struct HostReport
{
	1: required HostStatus status,
	2: required map<ColumnID, RepositoryStatus> repositoryStatus
}

struct TopologyReport
{
	1: required TopologyID topologyID,
	2: required map<HostID, HostReport> hosts,
	3: required Revision revision = 1
}

// Host

typedef i64 LockID
typedef string LockName
enum LockMode { Read = 1, Write = 2 }
/** Timeout in ms; <= 0 means fail immediately. */
typedef i32 LockTimeout

struct TransactionID
{
	1: required Revision revision,
	2: required i64 key
}

struct Include
{
	1: required binary rowID,
	2: required binary value
}

struct Exclude
{
	1: required binary rowID
}
 
struct ColumnWrites
{
	1: optional list<Include> includes,
	2: optional list<Exclude> excludes
}

typedef map<ColumnID, ColumnWrites> Writes

struct Statistic
{
	1: required i64 total,
	2: required i64 unique
}

struct RangeBound
{
	1: required binary value,
	2: required bool inclusive,
	3: optional binary rowID
}

struct RangeRequest
{
	1: required i32 limit = 500,
	2: required bool ascending = true,
	3: optional RangeBound first,
	4: optional RangeBound last
}

struct ValueRows
{
	1: required binary value,
	2: required list<binary> rowIDs
}

typedef list<ValueRows> ValueRowsList

struct RangeResult
{
	1: required ValueRowsList valueRowsList,
	2: required bool endOfFile,
	3: required bool beginOfFile,
	4: required bool limited
}

struct Query
{
	1: optional list<binary> rowIDs,
	2: optional list<RangeRequest> ranges
}

typedef map<ColumnID, Query> Queries

struct Answer
{
	1: optional list<binary> rowIDValues,
	2: optional list<RangeResult> rangeValues
}

struct ReadResult
{
	1: required Answer answer,
	2: required Revision revision
}

typedef map<ColumnID, ReadResult> ReadResults

typedef map<Query, Answer> Read

typedef map<ColumnID, Read> Reads

exception NotLatest
{
	1: Revision latest
}

exception Conflict
{
	1: string details,
	2: list<ColumnID> columnIDs
}

exception BeyondHistory
{
	1: Revision minHistory
}

exception LockExpired
{
	1: LockID lockID
}

exception LockTimedOut {}

service Service
{
	/** Returns the target topology as this host presently understands it. */
	TopologyResult getTopology(),

	/** Updates the topology and returns the new topology revision - GRID COORDINATED. */
	Revision prepareTopology(1:TransactionID transactionID, 2:Topology topology),

	/** Informs that the prepare was successful, the change should be committed. */
	void commitTopology(1:TransactionID transactionID),

	/** Informs that the prepare was unsuccessful, the change should be rolled back. */
	void rollbackTopology(1:TransactionID transactionID),

	/** Returns the current status of all hosts as this host understands it. */
	TopologyReport getTopologyReport(),

	/** Returns the current status of this host. */
	HostReport getReport(),


	/** Validates that the transaction ID is updated to the latest and then Applies all changes - GRID COORDINATED. */
	Revision prepare(1:TransactionID transactionID, 2:Writes writes, 3:Reads reads) 
		throws (1:NotLatest notLatest),
	
	/** Applies the given writes as of the latest revision (regardless of whether the transaction ID is out of date), 
		returns an updated Transaction ID - GRID COORDINATED. */
	TransactionID apply(1:TransactionID transactionID, 2:Writes writes),

	/** Informs that the prepare was successful, the changes should be committed. */
	void commit(1:TransactionID transactionID),

	/** Informs that the prepare was unsuccessful, the changes should be rolled back. */
	void rollback(1:TransactionID transactionID),

	/** Waits for the given transaction to be flushed to disk */
	void flush(1:TransactionID transactionID),


	/** Determines whether the given set of reads conflict with any intervening revisions. */
	bool doesConflict(1:Reads reads, 2:Revision source, 3:Revision target)
		throws (1:BeyondHistory beyondHistory),

	/** Updates the given transaction to the latest by validating reads and writes for conflicts, and returns a new TransactionID. */
	TransactionID update(1:TransactionID transactionID, 2:Writes writes, 3:Reads reads)
		throws (1:Conflict conflict),
	
	/** Upgrades or downgrades the given reads to match the data as of a given revision. */
	Reads transgrade(1:Reads reads, 2:Revision source, 3:Revision target)
		throws (1:BeyondHistory beyondHistory),

	
	/** Acquires a given named lock given a mode and timeout. */
	LockID acquireLock(1:LockName name, 2:LockMode mode, 3:LockTimeout timeout = 1000)
		throws (1:LockTimedOut timeout),

	/** Keeps a given lock alive - locks automatically expire if not renewed. */
	void keepLock(1:LockID lockID)
		throws (1:LockExpired expired),
	
	/** Attempts to escalate the given lock to write mode */
	void escalateLock(1:LockID lockID, 2:LockTimeout timeout = -1)
		throws (1:LockTimedOut timeout, 2:LockExpired expired),

	/** Releases the given lock */
	void releaseLock(1:LockID lockID)
		throws (1:LockExpired expired),

	/** Retrieves data and the latest revision number corresponding to a given list of queries. */
	ReadResults query(1:Queries queries),
	
	/** Retrieves statistics for a given list of columns based on the latest committed revision. */
	list<Statistic> getStatistics(1:list<ColumnID> columnIDs)
}

