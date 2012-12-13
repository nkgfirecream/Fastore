namespace cpp fastore.communication
namespace csharp Alphora.Fastore

// State Reporting

typedef i64 TopologyID
typedef i64 ColumnID
typedef i64 HostID
typedef i64 StashID
typedef i64 PodID
struct NetworkAddress
{
	1: required string name,
	2: optional i64 port
}

enum RepositoryStatus
{
	/** Not in-sync; don't send read or write requests. */
	Loading = 1,
	/** In the process of unloading; don't send any requests. */
	Unloading = 2,
	/** Online and servicing requests */
	Online = 3,
	/** The repository is offline due to error during startup or execution. */
	Offline = 4
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
	3: required i64 port
}

struct StoreState
{
	2: required i64 port
}

struct ServiceState
{
	1: required ServiceStatus status,
	2: required TimeStamp timeStamp,
	3: required NetworkAddress address,
	4: required list<WorkerState> workers,
	5: required StoreState store
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
typedef list<PodID> PodIDs
typedef list<HostID> HostIDs
typedef list<StashID> StashIDs

typedef map<PodID, ColumnIDs> Pods
typedef map<StashID, ColumnIDs> Stashes
	
struct Host
{
	1: required Pods pods,
	2: required Stashes stashes
}

struct Topology
{
	1: required TopologyID topologyID, 
	2: required map<HostID, Host> hosts
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
    /* Tells the service to shutdown gracefully */
	void shutdown(),

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
	OptionalHiveState getHiveState(1:bool forceUpdate = false),

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
		throws (1:LockExpired expired, 2:NotJoined notJoined),


	/** Requests a checkpoint be immediately scheduled for the given column IDs. */
	oneway void checkpoint(1:ColumnIDs columnIDs)
}

// Worker

typedef i64 Revision

typedef i64 TransactionID

struct Cell
{
	1: required binary rowID,
	2: required binary value
}
 
struct ColumnWrites
{
	1: optional list<Cell> includes,
	2: optional list<Cell> excludes
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

struct OptionalValue
{
	1: optional binary value
}

struct Answer
{
	1: optional list<OptionalValue> rowIDValues,
	2: optional list<RangeResult> rangeValues
}

struct ReadResult
{
	1: required Answer answer,
	2: required Revision revision
}

typedef map<ColumnID, ReadResult> ReadResults

typedef i64 BloomFilter

struct RangePredicate
{
	1: optional RangeBound first,
	2: optional RangeBound last
}

struct PrepareInfo
{
	1: optional Revision fromRevision,
	2: list<RangePredicate> rangePredicates,
	3: BloomFilter filter
}

typedef map<ColumnID, PrepareInfo> ColumnPrepares

struct PrepareResult
{
	1: required Revision actualRevision,
	2: required bool validateRequired
}

typedef map<ColumnID, PrepareResult> PrepareResults

exception NotLatest
{
	1: Revision latest
}

exception AlreadyPending
{
	1: TransactionID pendingTransactionID
	2: ColumnID columnID
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
	/** Instructs the worker to shutdown gracefully. */
	void shutdown(),

	/** Starts the initial loading process; no transaction protocol. */
	void loadBegin(1:ColumnID columnID),

	/** Loads a bulk block of values. */
	void loadBulkWrite(1:ColumnID columnID, 2:ValueRowsList values),

	/** Loads a set of writes. */
	void loadWrite(1:ColumnID columnID, 2:ColumnWrites writes),

	/** Ends the initial loading process. */
	void loadEnd(1:ColumnID columnID, 2:Revision revision),

	
	/** Retrieves current state of the worker and its repositories. */
	WorkerState getState(),


	/** Attempt to acquire new revisions for all involved columns, and validates that no collisions are possible. - HIVE TRANSACTED. */
	PrepareResults prepare(1:TransactionID transactionID, 2:ColumnPrepares columns) 
		throws (1:AlreadyPending alreadyPending),
	
	/** Prepares changes at the current revision. - HIVE TRANSACTED. */
	PrepareResults apply(1:TransactionID transactionID, 2:ColumnIDs columns)
		throws (1:AlreadyPending alreadyPending),

	/** Informs that the prepare was successful on the majority of workers, the changes should be committed. */
	oneway void commit(1:TransactionID transactionID, 2:Writes writes),

	/** Informs that the prepare was unsuccessful on the majority of workers, the changes should be rolled back. */
	oneway void rollback(1:TransactionID transactionID),


	/** Retrieves data and the latest revision number corresponding to a given list of queries. */
	ReadResults query(1:Queries queries),

	/** Retrieves statistics for a given list of columns based on the latest committed revision. */
	list<Statistic> getStatistics(1:list<ColumnID> columnIDs),
}

struct ColumnRange
{
	1:required Revision from, 
	2:required Revision to
}

typedef map<ColumnID, ColumnRange> Ranges

struct GetWritesResult
{
	/** If writes is not populated the range is out of bounds, consult minFrom or MaxTo to determine the missing extent. */
	1:optional map<Revision, ColumnWrites> writes,
	2:required Revision minFrom,
	3:required Revision maxTo
}

typedef map<ColumnID, GetWritesResult> GetWritesResults

struct StoreStatus
{
	1:map<ColumnID, Revision> LastCheckpoints,
	2:set<ColumnID> beganCheckpoints,
	3:map<ColumnID, Revision> LatestRevisions
}

service Store
{
	/** Begins the process of checkpointing a given column */
	void checkpointBegin(1:ColumnID columnID),

	/** Writes a block of values to the checkpoint. */
	void checkpointWrite(1:ColumnID columnID, 2:ValueRowsList values),

	/** Completes the checkpoint for the given column. */
	void checkpointEnd(1:ColumnID columnID),


	/** Retrieves various details about that store's present status. */
	StoreStatus getStatus(),


	/** Retrieves changes for a given set of column revision ranges. May included changes not yet flushed. */
	GetWritesResults getWrites(1:Ranges ranges),


	/** Commits the given already prepared writes. */
	oneway void commit(1:TransactionID transactionID, 2:map<ColumnID,Revision> revisions, 3:Writes writes),


	/** Waits for the given transaction to be flushed to disk. */
	void flush(1:TransactionID transactionID),
}
