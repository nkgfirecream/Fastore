#pragma once
#include <Communication/Comm_types.h>
#include <Connection/ConnectionPool.h>
#include <Communication/Service.h>
#include <Communication/Worker.h>
#include <Communication/Store.h>
#include <boost/thread.hpp>


// The purpose of the scheduler is to manage events that must happen
// on the service on intervals. Since the workers are single threaded,
// they are not able to wake at defined intervals. The scheduler can wake
// and send messages to the workers.

// Examples of potential actions the scheduler might do:
// Coordinating checkpointing
// Worker repair
// Log management/batching

class Scheduler
{
public:
	enum status_t { idle, running, stopped };
	Scheduler(fastore::communication::NetworkAddress serviceAddress, fastore::communication::HostID hostId);

	//Start and stop the scheduler running. 
	//Need to think about potential problems and 
	//race conditions since it needs to talk to the service,
	//via the thrift interface.
	//Furthermore, workers might start callbacks into the scheduler once they start running.

	//Once the scheduler is running, it should either ask the service about the state of the workers to find them,
	//or it should listen for callbacks from workers, or both...
	bool start();
	void stop();
	status_t status() const { return _status; }

private:
	fastore::communication::HiveState _hiveState;
	fastore::communication::HostID _hostId;


	// Connection pool of services by host ID
	fastore::client::ConnectionPool<HostID, ServiceClient> _services;

	// Connection pool of stores by host ID
	fastore::client::ConnectionPool<HostID, StoreClient> _stores;

	// Connected workers by pod ID
	fastore::client::ConnectionPool<PodID, WorkerClient> _workers;

	//Current state of workers
	std::map<PodID, std::pair<ServiceState, WorkerState>> _workerStates;

	void sendHeartbeat();
	void populateWorkers(fastore::communication::StoreStatus currentStatus);
	
	fastore::communication::HiveState queryHiveForState();
	void updateHiveStateCache(const HiveState &newState);

	const static int INTERVAL = 3000;

	fastore::communication::NetworkAddress _serviceAddress;
	boost::shared_ptr<boost::thread>  _pthread;
	bool _run;
	status_t _status;

	void run();

	NetworkAddress Scheduler::getWorkerAddress(PodID podID);
	NetworkAddress Scheduler::getServiceAddress(HostID hostID);
	NetworkAddress Scheduler::getStoreAddress(HostID hostID);

};