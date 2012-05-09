namespace cpp fastore;
namepsace cs Alphora.Fastore;

// Common

typedef i64 Revision;

// Topology

typedef i32 TopologyID;
typedef i32 ColumnID;
typedef i32 HostID;
typedef string HostAddress;

struct Column
{
	1: ColumnID ID,
	2: string Name
	3: bool IsUnique,
	4: string Type,
	5: string IDType
}

struct Host
{
	1: HostID ID,

	/** Host name and optional port (e.g. "myserver:1234") */
	2: HostAddress Address
}

struct ColumnHost
{
	1: ColumnID ColumnID,
	2: HostID HostID
}

struct Topology
{
	1: TopologyID ID,
	2: set<Column> Columns,
	4: set<Host> Hosts,
	5: set<ColumnHost> ColumnHosts,
	6: Revision Revision = 1
}

// Host

typedef i64 LockID;
typedef string LockName;
enum LockMode { Read = 1, Write = 2 };
/** Timeout in ms; -1 = infinite; 0 = fail immediately */
typedef i32 LockTimeout;

typedef i64 TransactionID;

struct Include
{
	1: ColumnID ColumnID,
	2: binary RowID,
	3: binary Value
}

struct Exclude
{
	1: ColumnID ColumnID,
	2: binary RowID
}
 
struct Changes
{
	1: TransactionID TransactionID,
	2: set<Include> Includes,
	3: set<Exclude> Excludes
}

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
	1: ColumnID ColumnID, 
	2: i32 Limit = 500,
	3: bool Ascending,
	4: optional RangeBound Start,
	5: optional RangeBound End
}

struct RangeResponse
{
	1: list<ValueRows> ValueRows,
	2: Revision Revision
}

struct ValueRequest
{
	1: ColumnID ColumnID,
	2: list<binary> RowIDs
}

struct ValueResponse
{
	1: list<binary> Values,
	2: Revision Revision
}

struct OrderRequest
{
	1: ColumnID ColumnID,
	2: list<list<binary>> RowIDLists,
	3: bool Ascending
}

struct ValueRows
{
	1: binary Value,
	2: list<binary> RowIDs
}

struct OrderResponse
{
	1: list<ValueRows> ValueRowsList,
	2: Revision Revision
}

service Host
{
	1: Topology GetTopology(),

	2: TransactionID StartTransaction(),
	3: void PrepareTransaction(1:TransactionID transactionID),
	4: Revision CommitTransaction(1:TransactionID transactionID),
	5: void WaitTransaction(1:TransactionID transactionID),

	6: LockID GetLock(1:LockName name, 2:LockMode mode, 3:LockTimeout timeout = -1),
	7: void EscelateLock(1:LockID lockID, 2:LockTimeout timeout = -1),
	8: void ReleaseLock(1:LockID lockID),

	9: list<RangeResponse> GetRanges(1:list<RangeRequest> requests),
	10: list<ValueResponse> GetValues(1:list<ValueRequest> requests),
	11: list<OrderResponse> GetOrdered(1:list<OrderRequest> requests),

	12: void Apply(1:Changes changes),
	
	13: list<Statistic> GetStatistics(1:list<ColumnID> columnIDs); 

	// TODO: Revisor logic 
}

