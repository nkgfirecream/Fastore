include "Comm.thrift"

namespace cpp fastore.server
namespace csharp Alphora.Fastore

typedef string Path
typedef i32 WorkerNumber

struct ServiceStartup
{
	/** Instance path. */
	1: required Path path,

	/** Override of network address. */
	2: optional string address,

	/** Override of network port. */
	3: optional i32 port,

	/** Override of worker paths */
	4: optional map<WorkerNumber, string> workerPaths
}

struct JoinedTopology
{
	1: required Comm.TopologyID topologyID,
	2: required Comm.HostID hostID,
	3: required map<WorkerNumber, Comm.PodID> workerPodIDs
}

struct ServiceConfig
{
	1: required Path path,
	2: required map<WorkerNumber, Path> workerPaths,
	3: required Comm.NetworkAddress address,
	4: optional JoinedTopology joinedTopology
}
