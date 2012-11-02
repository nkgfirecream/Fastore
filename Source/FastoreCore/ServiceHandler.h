#pragma once
#include "../FastoreCommon/Service.h"
#include "../FastoreCommon/Server_types.h"
#include "Endpoint.h"
#include <thrift/server/TServer.h>
#include <thrift/transport/TFDTransport.h>
#include <boost/thread.hpp>
#include <unordered_map>
#include <string>
#include "Scheduler.h"
#include "TFastoreServer.h"
#include "Worker.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

class ServiceHandler : virtual public fastore::communication::ServiceIf, virtual public apache::thrift::TProcessorEventHandler
{
private: 
	static const char* const ConfigFileName;

	boost::filesystem::path _configPath;
	boost::shared_ptr<fastore::server::ServiceConfig> _config;
	std::list<boost::shared_ptr<Endpoint>> _endpoints;
	std::list<boost::thread> _workerThreads;
	std::list<Worker> _workers;
	boost::shared_ptr<fastore::communication::HiveState> _hiveState;	
	boost::shared_ptr<Scheduler> _scheduler;

	//Not a shared pointer because we don't own the connection.
	apache::thrift::server::TFastoreServer::TConnection* _currentConnection;


	void InitializeWorkers(const std::vector<fastore::communication::WorkerState>& workers);
	void SaveConfiguration();
	void EnsureWorkerPaths(size_t numWorkers);
	int GetRecommendedWorkerCount();
	void CheckInHive();
	void CheckNotInHive();
public:
	ServiceHandler(const fastore::server::ServiceStartup& startup);

	void ping();
	void init(fastore::communication::ServiceState& _return, const fastore::communication::Topology& topology, const fastore::communication::HostAddresses& addresses, const fastore::communication::HostID hostID);
	void join(fastore::communication::ServiceState& _return, const fastore::communication::HiveState& hiveState, const fastore::communication::NetworkAddress& address, const fastore::communication::HostID hostID);
	void leave();
	void getHiveState(fastore::communication::OptionalHiveState& _return, const bool forceUpdate);
	void getState(fastore::communication::OptionalServiceState& _return);
	fastore::communication::LockID acquireLock(const fastore::communication::LockName& name, const fastore::communication::LockMode::type mode, const fastore::communication::LockTimeout timeout);
	void keepLock(const fastore::communication::LockID lockID);
	void escalateLock(const fastore::communication::LockID lockID, const fastore::communication::LockTimeout timeout);
	void releaseLock(const fastore::communication::LockID lockID);

	//Admin
	void shutdown();

	//TProcessorEventHandler
	void handlerError(void* ctx, const char* fn_name);
	void* getContext(const char* fn_name, void* serverContext);
};
