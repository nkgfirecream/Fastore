namespace cpp fastore.communication
namespace csharp Alphora.Fastore

// State Reporting

typedef i32 TopologyID
typedef i32 ColumnID
typedef i32 HostID
typedef i32 PodID
struct NetworkAddress
{
	1: required string name,
	2: optional i32 port
}

enum RepositoryStatus
{
	/** Not in-sync; don't send read or write requests. */
	Loading = 1,
	/** In the process of unloading; don't send any requests. */
	Unloading = 2,
	/** Online and servicing requests */
	Online = 3,
	/** In the process of checkpointing; can send read and write requests, but performance may be sub-optimal. */
	Checkpointing = 4,
	/** The repository is offline due to error during startup or execution. */
	Offline = 5
}

enum ServiceStatus
{
	Unknown = 1,
	Offline = 2,
	Online = 3,
	Unreachable = 4
}

typedef i64 TimeStamp

struct WorkerState
{
	1: required PodID podID,
	2: required map<ColumnID, RepositoryStatus> repositoryStatus,
	3: required i32 port
}

struct ServiceState
{
	1: required ServiceStatus status,
	2: required TimeStamp timeStamp,
	3: required NetworkAddress address,
	4: required list<WorkerState> workers
}

struct HiveState
{
	1: required TopologyID topologyID,
	2: required map<HostID, ServiceState> services,
	/** the host ID of the service giving the report */
	3: required HostID reportingHostID
}

// Topology construction

typedef list<ColumnID> ColumnIDs

typedef map<PodID, ColumnIDs> Pods
	
struct Topology
{
	1: required TopologyID topologyID, 
	2: required map<HostID, Pods> hosts
}

typedef map<HostID, NetworkAddress> HostAddresses

// Service

typedef i64 LockID
typedef string LockName
enum LockMode { Read = 1, Write = 2 }
/** Timeout in ms; <= 0 means fail immediately. */
typedef i32 LockTimeout

/* Specifies the hive state, or if not joined how many potential workers */
struct OptionalHiveState
{
	1: optional HiveState hiveState,
	2: optional i32 potentialWorkers
}

/* Specifies the service state, or if not joined how many potential workers */
struct OptionalServiceState
{
	1: optional ServiceState serviceState,
	2: optional i32 potentialWorkers
}

exception LockExpired
{
	1: LockID lockID
}

exception LockTimedOut {}

exception AlreadyJoined
{
	1: HostID hostID
}

exception NotJoined 
{
	1: i32 potentialWorkers
}

service Service
{
	void ping(),

	/** Initialize a new hive including this service */
	ServiceState init(1:Topology topology, 2:HostAddresses addresses, 3:HostID hostID)
		throws (1:AlreadyJoined alreadyJoined),

	/** Associates the service with the given logical host ID within the given hive. */
	ServiceState join(1:HiveState hiveState, 3:NetworkAddress address, 4:HostID hostID)
		throws (1:AlreadyJoined alreadyJoined),

	/** Dissociates the service from the current hive. */
	void leave()
		throws (1:NotJoined notJoined),

	/** Returns the current status of all services in the hive as understood by this service. */
	OptionalHiveState getHiveState(1: bool forceUpdate = false),

	/** Returns the current status of this service. */
	OptionalServiceState getState(),


	/** Acquires a given named lock given a mode and timeout. */
	LockID acquireLock(1:LockName name, 2:LockMode mode, 3:LockTimeout timeout = 1000)
		throws (1:LockTimedOut timeout, 2:NotJoined notJoined),

	/** Keeps a given lock alive - locks automatically expire if not renewed. */
	void keepLock(1:LockID lockID)
		throws (1:LockExpired expired, 2:NotJoined notJoined),
	
	/** Attempts to escalate the given lock to write mode */
	void escalateLock(1:LockID lockID, 2:LockTimeout timeout = -1)
		throws (1:LockTimedOut timeout, 2:LockExpired expired, 3:NotJoined notJoined),

	/** Releases the given lock */
	void releaseLock(1:LockID lockID)
		throws (1:LockExpired expired, 2:NotJoined notJoined)
}

// Worker

typedef i64 Revision

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
	2: required bool inclusive
}

struct RangeRequest
{
	1: required bool ascending = true,
	2: required i32 limit = 500
	3: optional RangeBound first,
	4: optional RangeBound last,
	5: optional binary rowID

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
	2: required bool eof,
	3: required bool bof,
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

exception AlreadyPending
{
	1: TransactionID pendingTransactionID
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

service Worker
{
	/** Retreives current state of the worker and its repositories */
	WorkerState getState(),

	/** Validates that the transaction ID is updated to the latest and then Applies all changes - HIVE TRANSACTED. */
	Revision prepare(1:TransactionID transactionID, 2:Writes writes, 3:Reads reads) 
		throws (1:NotLatest notLatest, 2:AlreadyPending alreadyPending),
	
	/** Applies the given writes as of the latest revision (regardless of whether the transaction ID is out of date), 
		returns an updated Transaction ID - HIVE TRANSACTED. */
	TransactionID apply(1:TransactionID transactionID, 2:Writes writes)
		throws (1:AlreadyPending alreadyPending),

	/** Informs that the prepare was successful on the majority of workers, the changes should be committed. */
	oneway void commit(1:TransactionID transactionID),

	/** Informs that the prepare was unsuccessful on the majority of workers, the changes should be rolled back. */
	oneway void rollback(1:TransactionID transactionID),

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

	
	/** Retrieves data and the latest revision number corresponding to a given list of queries. */
	ReadResults query(1:Queries queries),
	
	/** Retrieves statistics for a given list of columns based on the latest committed revision. */
	list<Statistic> getStatistics(1:list<ColumnID> columnIDs)	
}

