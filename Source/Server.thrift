include "Comm.thrift"

namespace cpp fastore.server
namespace csharp Alphora.Fastore

typedef string Path

struct ServiceStartup
{
	/** Instance path. */
	1: required Path path,

	/** Override of network address. */
	2: optional string address,

	/** Override of network port. */
	3: optional i64 port,

	/** Override of worker paths */
	4: optional list<string> workerPaths
}

struct ServiceConfig
{
	1: required Path path,
	2: required list<Path> workerPaths,
	3: optional Comm.NetworkAddress address,
	4: optional Comm.HiveState joinedHive
}
