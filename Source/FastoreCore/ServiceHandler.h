#pragma once
#include "FastoreHost.h"
#include "../FastoreCommunication/Service.h"
#include "../FastoreCommunication/Server_types.h"
#include "Endpoint.h"
#include <thrift/server/TServer.h>
#include <thrift/transport/TSimpleFileTransport.h>
#include <hash_map>
#include <string>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace ::fastore::communication;
using namespace ::fastore::server;

class ServiceHandler : virtual public ServiceIf 
{
private: 
	static const char* const ConfigFileName;

	shared_ptr<TSimpleFileTransport> _configFile;
	shared_ptr<ServiceConfig> _config;
	std::hash_map<WorkerNumber, shared_ptr<Endpoint>> _workers;
	shared_ptr<HiveState> _hiveState;	

	void InitializeJoined(const JoinedTopology& joined);
	void SaveConfiguration();
	std::string EnsureWorkerPath(int workerNumber);
	int GetRecommendedWorkerCount();
	void CheckInHive();
	void CheckNotInHive();
public:
	ServiceHandler(const ServiceStartup& startup);

	void init(ServiceState& _return, const Topology& topology, const HostAddresses& addresses, const HostID hostID);
	void join(ServiceState& _return, const HiveState& hiveState, const NetworkAddress& address, const HostID hostID);
	void leave();
	void getHiveState(HiveState& _return, const bool forceUpdate);
	void getState(ServiceState& _return);
	LockID acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout);
	void keepLock(const LockID lockID);
	void escalateLock(const LockID lockID, const LockTimeout timeout);
	void releaseLock(const LockID lockID);
};