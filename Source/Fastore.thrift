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
	1: HostID ID,

	/** Host name and optional port (e.g. "myserver:1234") */
	2: HostAddress Address,
}

struct Repository
{
	1: ColumnID ColumnID,
	2: HostID HostID,
}

struct Topology
{
	1: TopologyID ID,
	2: set<Host> Hosts,
	3: set<Repository> Repositories
}

struct TopologyResult
{
	1: Topology Topology,
	2: Revision Revision
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
	1: HostStatus Status,
	2: map<ColumnID, RepositoryStatus> RepositoryStatus
}

struct TopologyReport
{
	1: TopologyID TopologyID,
	2: map<HostID, HostReport> Hosts,
	3: Revision Revision = 1
}

// Host

typedef i64 LockID
typedef string LockName
enum LockMode { Read = 1, Write = 2 }
/** Timeout in ms; <= 0 means fail immediately. */
typedef i32 LockTimeout
const LockTimeout DefaultLockTimeout = 1000;

struct TransactionID
{
	1: Revision Revision,
	2: i64 Key
}

struct Include
{
	1: binary RowID,
	2: binary Value
}

struct Exclude
{
	1: binary RowID
}
 
struct ColumnWrites
{
	1: set<Include> Includes,
	2: set<Exclude> Excludes
}

typedef map<ColumnID, ColumnWrites> Writes

struct Statistic
{
	1: i64 Total,
	2: i64 Unique
}

struct RangeBound
{
	1: binary Value,
	2: bool Inclusive,
	3: optional binary RowID
}

struct RangeRequest
{
	1: i32 Limit = 500,
	2: bool Ascending = true,
	3: optional RangeBound Start,
	4: optional RangeBound End
}

struct ValueRows
{
	1: binary Value,
	2: list<binary> RowIDs
}

typedef list<ValueRows> ValueRowsList

struct Query
{
	1: list<binary> RowIDs,
	2: list<RangeRequest> Ranges
}

typedef map<ColumnID, Query> Queries

struct Answer
{
	1: list<binary> RowIDValues,
	2: list<ValueRowsList> RangeValues
}

struct ReadResults
{
	1:map<ColumnID, Answer> Answers,
	2:Revision Revision
}

typedef map<Query, Answer> Read

typedef map<ColumnID, Read> Reads
	

exception NotLatest
{
	1: Revision Latest
}

exception Conflict
{
	1: string Details
}

exception BeyondHistory
{
	1: Revision MinHistory
}

exception LockExpired
{
	1: LockID LockID
}

exception LockTimedOut {}

service Service
{
	/** Returns the target topology as this host presently understands it. */
	TopologyResult GetTopology(),

	/** Updates the topology and returns the new topology revision - GRID COORDINATED. */
	Revision PrepareTopology(1:TransactionID transactionID, 2:Topology topology),

	/** Informs that the prepare was successful, the change should be committed. */
	void CommitTopology(1:TransactionID transactionID),

	/** Informs that the prepare was unsuccessful, the change should be rolled back. */
	void RollbackTopology(1:TransactionID transactionID),

	/** Returns the current status of all hosts as this host understands it. */
	TopologyReport GetTopologyReport(),

	/** Returns the current status of this host. */
	HostReport GetReport(),


	/** Validates that the transaction ID is updated to the latest and then Applies all changes - GRID COORDINATED. */
	Revision Prepare(1:TransactionID transactionID, 2:Writes writes, 3:Reads reads) 
		throws (1:NotLatest notLatest),
	
	/** Applies the given writes as of the latest revision (regardless of whether the transaction ID is out of date), 
		returns an updated Transaction ID - GRID COORDINATED. */
	TransactionID Apply(1:TransactionID transactionID, 2:Writes writes),

	/** Informs that the prepare was successful, the changes should be committed. */
	void Commit(1:TransactionID transactionID),

	/** Informs that the prepare was unsuccessful, the changes should be rolled back. */
	void Rollback(1:TransactionID transactionID),

	/** Waits for the given transaction to be flushed to disk */
	void Flush(1:TransactionID transactionID),


	/** Determines whether the given set of reads conflict with any intervening revisions. */
	bool DoesConflict(1:Reads reads, 2:Revision source, 3:Revision target)
		throws (1:BeyondHistory beyondHistory),

	/** Updates the given transaction to the latest by validating reads and writes for conflicts, and returns a new TransactionID. */
	TransactionID Update(1:TransactionID transactionID, 2:Writes writes, 3:Reads reads)
		throws (1:Conflict conflict),
	
	/** Upgrades or downgrades the given reads to match the data as of a given revision. */
	Reads Transgrade(1:Reads reads, 2:Revision source, 3:Revision target)
		throws (1:BeyondHistory beyondHistory),

	
	/** Acquires a given named lock given a mode and timeout. */
	LockID AcquireLock(1:LockName name, 2:LockMode mode, 3:LockTimeout timeout = DefaultLockTimeout)
		throws (1:LockTimedOut timeout),

	/** Keeps a given lock alive - locks automatically expire if not renewed. */
	void KeepLock(1:LockID lockID)
		throws (1:LockExpired expired),
	
	/** Attempts to escalate the given lock to write mode */
	void EscalateLock(1:LockID lockID, 2:LockTimeout timeout = -1)
		throws (1:LockTimedOut timeout, 2:LockExpired expired),

	/** Releases the given lock */
	void ReleaseLock(1:LockID lockID)
		throws (1:LockExpired expired),

	/** Retrieves data and the latest revision number corresponding to a given list of queries. */
	ReadResults Query(1:Queries queries),
	
	/** Retrieves statistics for a given list of columns based on the latest committed revision. */
	list<Statistic> GetStatistics(1:list<ColumnID> columnIDs)
}

