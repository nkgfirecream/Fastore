#pragma once
#include "FastoreHost.h"
#include "../FastoreCommunication/Service.h"
#include "../FastoreCommunication/Server_types.h"
#include "Endpoint.h"
#include <thrift/server/TServer.h>
#include <thrift/transport/TFileTransport.h>
#include <hash_map>

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

	shared_ptr<TFileTransport> _configFile;
	shared_ptr<ServiceConfig> _config;
	std::hash_map<WorkerNumber, shared_ptr<Endpoint>> _workers;

	void InitializeJoined(const JoinedTopology& joined);
	void SaveConfiguration();
public:
	ServiceHandler(const ServiceStartup& startup);

	void init(HiveState& _return);
	void join(ServiceState& _return, const HostID hostID, const HiveState& hiveState);
	void leave();
	void getHiveState(HiveState& _return);
	void getState(ServiceState& _return);
	LockID acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout);
	void keepLock(const LockID lockID);
	void escalateLock(const LockID lockID, const LockTimeout timeout);
	void releaseLock(const LockID lockID);
};