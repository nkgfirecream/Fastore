#include "Scheduler.h"
#include <Communication/Store.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransport.h>
#include <thrift/transport/TBufferTransports.h>
#include <future>

Scheduler::Scheduler(fastore::communication::NetworkAddress serviceAddress, fastore::communication::HostID hostId) : 
	_serviceAddress(serviceAddress),
	_hostId(hostId),
	_status(idle),
	_services
	(
		[](boost::shared_ptr<TProtocol> proto) { return ServiceClient(proto); }, 
		[&](HostID i) { return this->getServiceAddress(i); }, 
		[](ServiceClient& client) { client.getInputProtocol()->getTransport()->close(); }, 
		[](ServiceClient& client) { return client.getInputProtocol()->getTransport()->isOpen();}
	),
	_stores
	(
		[](boost::shared_ptr<TProtocol> proto) { return StoreClient(proto); }, 
		[&](HostID i) { return this->getStoreAddress(i); }, 
		[](StoreClient& client) { client.getInputProtocol()->getTransport()->close(); }, 
		[](StoreClient& client) { return client.getInputProtocol()->getTransport()->isOpen();}
	),
	_workers
	(
		[](boost::shared_ptr<TProtocol> proto) { return WorkerClient(proto); }, 
		[&](PodID i) { return this->getWorkerAddress(i); }, 
		[](WorkerClient& client) { client.getInputProtocol()->getTransport()->close(); }, 
		[](WorkerClient& client) { return client.getInputProtocol()->getTransport()->isOpen();}
	)
	{ }

bool Scheduler::start()
{
	_run = true;
	try 
	{ 
		_pthread = boost::shared_ptr<boost::thread>(
			new boost::thread(std::mem_fun(&Scheduler::run), this) );
		_status = running;
	}
	catch( std::exception& oops ) 
	{
		_status = stopped;
	}
	catch(...) 
	{
		_status = stopped;
		perror("indeterminate error");
	}
	return _status == running;
}

void Scheduler::stop()
{
	_run = false;
	_pthread->join();	
}

void Scheduler::run()
{
	fastore::communication::ServiceState state;
	state.__set_address(_serviceAddress);

	//Add our local service first, just to bootstrap.
	_hiveState.services.insert(std::make_pair(_hostId, state));

	//Scheduler should never start running until hive is initialized.
	while(_run)
	{
	    //Scheduler -- Query service for current state.
		auto hiveState = queryHiveForState();
		updateHiveStateCache(hiveState);

		//Send a heartbeat -- TODO: Should this happen on even another thread to ensure consistency?
		sendHeartbeat();

		//Check our current service for state.
		auto service = hiveState.services[_hostId];
		
		//Check stuff on the service...

		//Check store status
		auto storeClient = _stores[_hostId];

		fastore::communication::StoreStatus sStatus;
		storeClient.getStatus(sStatus);

		_stores.Release(std::make_pair(_hostId, storeClient));

		//if the log is still loading, just wait.
		if(sStatus.LogStatus != fastore::communication::StoreLogStatus::Loading)
		{
			switch(sStatus.LogStatus)
			{
			case StoreLogStatus::Offline:
				//attempt to restart
				break;
			
			case StoreLogStatus::Ready:
				//fill workers
				populateWorkers(sStatus);
				break;

			case StoreLogStatus::Online:
				//check for possibility of checkpointing
				break;

			case StoreLogStatus::Unknown:
			default:
				break;
			}
		}

		sleep(INTERVAL);
	}

	_status = status_t::stopped;
}

void Scheduler::populateWorkers(fastore::communication::StoreStatus currentStatus)
{
	auto sClient = _stores[_hostId];
	
	for (auto begin = currentStatus.LatestRevisions.begin(), end = currentStatus.LatestRevisions.end(); begin != end; ++begin)
	{
		auto columnId = begin->first;
		auto maxRevision = begin->second;

		//See if we have this worker in our server.. find the correct pod, etc.
		auto wClient = _workers[columnId];

		wClient.loadBegin(columnId);

		//TODO: Grab multiple revisions at a time. Don't have a way to determine size.
		for (fastore::communication::Revision currentRevision = 0; currentRevision <= maxRevision; ++currentRevision)
		{
			fastore::communication::ColumnRange range;
			range.__set_from(currentRevision);
			range.__set_to(currentRevision);

			fastore::communication::Ranges ranges;
			ranges.insert(std::make_pair(columnId, range));

			fastore::communication::GetWritesResults result;
			sClient.getWrites(result, ranges);

			auto columnResult = result[columnId];

			auto writes = columnResult.writes;

			for (auto revBegin = writes.begin(), revEnd = writes.end(); revBegin != revEnd; ++revBegin)
			{				
				wClient.loadWrite(columnId, revBegin->second);
			}
		}

		wClient.loadEnd(columnId, maxRevision);

		_workers.Release(std::make_pair(columnId, wClient));
	}

	//Workers are loaded, start processing requests.
	sClient.start();

	_stores.Release(std::make_pair(_hostId, sClient));
}

void Scheduler::sendHeartbeat()
{
	auto client = _stores[_hostId];
	client.heartbeat();
	_stores.Release(std::make_pair(_hostId, client));

	auto sclient = _services[_hostId];
	sclient.heartbeat();
	_services.Release(std::make_pair(_hostId, sclient));
}

NetworkAddress Scheduler::getWorkerAddress(PodID podID)
{		
	auto iter = _workerStates.find(podID);
	if (iter == _workerStates.end())
	{
		
		throw "no worker found";
	}
	else
	{
		NetworkAddress na;
		na.__set_name(iter->second.first.address.name);
		na.__set_port(iter->second.second.port);
		return na;
	}
}

NetworkAddress Scheduler::getServiceAddress(HostID hostID)
{
	try
	{
		auto result = _hiveState.services.find(hostID);
		if (result == _hiveState.services.end())
		{
			throw "no service found";
		}
		else
		{
			return result->second.address;
		}				
	}
	catch(const std::exception& e)
	{
		throw e;
	}
}

NetworkAddress Scheduler::getStoreAddress(HostID hostID)
{
	try
	{
		auto result = _hiveState.services.find(hostID);
		if (result == _hiveState.services.end())
		{
			throw "no service or store found";
		}
		else
		{
			NetworkAddress address;
			address.__set_name(result->second.address.name);
			address.__set_port(result->second.store.port);
			return address;
		}				
	}
	catch(const std::exception& e)
	{
		throw e;
	}
}

HiveState Scheduler::queryHiveForState()
{
	//TODO: Need to actually try to get new worker information, update topologyID, etc.
	//This just assumes that we won't add any workers once we are up and running
	HiveState newHive;
	newHive.__set_topologyID(0);
	newHive.__set_services(std::map<HostID, ServiceState>());

	std::vector<std::future<std::pair<HostID, ServiceState>>> tasks;
	for (auto service : _hiveState.services)
	{
		tasks.push_back
		(
			std::future<std::pair<HostID, ServiceState>>
			(
				std::async
				(
					std::launch::async,
					[&]()-> std::pair<HostID, ServiceState>
					{
						ServiceClient serviceClient = _services[service.first];
						OptionalServiceState state;
						serviceClient.getState(state);
						if (!state.__isset.serviceState)						
							throw "Host is unexpectedly not part of the topology.";
						_services.Release(std::make_pair(service.first, serviceClient));
						return std::make_pair(service.first, state.serviceState);
					}
				)
			)
		);


	}

	for (size_t i = 0; i < tasks.size(); ++i)
	{
		tasks[i].wait();
		auto statePair = tasks[i].get();
		newHive.services.insert(statePair);
	}

	//Don't care for now...
	newHive.__set_reportingHostID(newHive.services.begin()->first);

	return newHive;
}

void Scheduler::updateHiveStateCache(const HiveState &newState)
{
	_hiveState = newState;

	// Maintain some indexes for quick access
	_workerStates.clear();

	if (newState.services.size() > 0)
	{
		for (auto service = newState.services.begin(); service != newState.services.end(); ++service)
		{
			for (auto  worker = service->second.workers.begin(); worker != service->second.workers.end(); ++worker)
			{
				_workerStates.insert(std::pair<PodID, std::pair<ServiceState, WorkerState>>(worker->podID, std::make_pair(service->second, *worker)));
			}
		}
	}
}