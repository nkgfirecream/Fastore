#include "ServiceHandler.h"
#include <iostream>
#include <thrift/transport/TSimpleFileTransport.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/transport/TSocket.h>
#include "WorkerHandler.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <functional>
#include <ctime>

using namespace boost::filesystem;
using boost::shared_ptr;
using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::fastore::communication;
using namespace ::fastore::server;

const char* const ServiceHandler::ConfigFileName = "config.json";

ServiceHandler::ServiceHandler(const ServiceStartup& startup)
{
	bool configChanged = false;

	auto configFileName = path(startup.path) / path(ConfigFileName);
	if (exists(configFileName))
	{
		cout << "Existing instance:" << startup.path;

		// Open and hold the configuration file
		_configFile = boost::shared_ptr<TSimpleFileTransport>(new TSimpleFileTransport(configFileName.string(), true, true));

		// Read the configuration
		TJSONProtocol reader(_configFile);
		_config = boost::shared_ptr<ServiceConfig>(new ServiceConfig());
		_config->read(&reader);
	}
	else
	{
		configChanged = true;
		cout << "New instance:" << startup.path;

		// Create new configuration
		_configFile = boost::shared_ptr<TSimpleFileTransport>(new TSimpleFileTransport(configFileName.string(), true, true));
		_config = boost::shared_ptr<ServiceConfig>(new ServiceConfig());
	}

	// Apply overrides/initialization from startup
	if (startup.__isset.port)
	{
		_config->address.__set_port(startup.port);
		_config->__isset.address = true;
		configChanged = true;
	}
	if (startup.__isset.address)
	{
		_config->address.__set_name(startup.address);
		_config->__isset.address = true;
		configChanged = true;
	}
	if (startup.path != _config->path)
	{
		_config->__set_path(startup.path);
		configChanged = true;
	}
	if (startup.__isset.workerPaths && startup.workerPaths.size() > 0)
	{
		_config->workerPaths.clear();
		for (auto wp = startup.workerPaths.begin(); wp != startup.workerPaths.end(); ++wp)
			_config->workerPaths.push_back(*wp);
		configChanged = true;
	}

	// Save the new configuration if needed
	if (configChanged)
		SaveConfiguration();

	if (_config->__isset.joinedHive)
	{
		auto thisService = _config->joinedHive.services.find(_config->joinedHive.reportingHostID);
		auto numPods = thisService->second.workers.size();

		// Report the number of configured pods
		cout << "Number of Pods: " << numPods << "	Recommended workers: " << GetRecommendedWorkerCount() << " (these should be the same)\r\n";
	
		// Initialize the workers, if joined
		InitializeWorkers(thisService->second.workers);
	}
	else
	{
		// Display worker recommendations
		cout << "Hardware cores/threads: " << (int)boost::thread::hardware_concurrency() << "	Recommended workers: " << GetRecommendedWorkerCount() << "\r\n";
	}
}

int ServiceHandler::GetRecommendedWorkerCount()
{
	// Use all processors but one, but ensure that at least worker is created
	return max(1, (int)boost::thread::hardware_concurrency() - 1);
}

void ServiceHandler::EnsureWorkerPaths(int numWorkers)
{
	while (_config->workerPaths.size() < numWorkers)
	{
		stringstream folder;
		folder << "worker" << _config->workerPaths.size();
		auto workerPath = path(_config->path) / path(folder.str());
		auto result = workerPath.string();
		_config->workerPaths.push_back(result);
	}
}

void ServiceHandler::SaveConfiguration()
{
	// Write the configuration for the new instance
	TJSONProtocol writer(_configFile);
	_config->write(&writer);
}

void ServiceHandler::InitializeWorkers(const vector<WorkerState>& workers)
{
	// Ensure that there enough configured paths for each worker
	EnsureWorkerPaths((int)workers.size());

	for (int i = 0; i < workers.size(); ++i)
	{
		auto podID = workers[i].podID;

		// Create a handler for the worker
		auto handler = boost::shared_ptr<WorkerHandler>(new WorkerHandler(podID, _config->workerPaths[i]));
		auto processor = boost::shared_ptr<TProcessor>(new WorkerProcessor(handler));
		
		// Create an endpoint for the worker
		EndpointConfig endPointConfig;
		endPointConfig.port = workers[i].port;
		auto endpoint = boost::shared_ptr<Endpoint>(new Endpoint(endPointConfig, processor));
		
		// Track the endpoint by worker number
		_workers.push_front(endpoint);

		// Start the endpoint's thread
		boost::thread workerThread
		(
			[endpoint, i, podID]() -> void 
			{ 
				try
				{
					endpoint->Run();
				}
				catch (const exception& e)
				{
					cout << "ERROR: unhandled exception running the endpoint for worker " << i << " for pod " << podID << ": " << e.what();
				}
				catch (...)
				{
					cout << "ERROR: unhandled exception running the endpoint for worker " << i << " for pod " << podID << ".";
				} 
			}
		);
		// TODO: experiment with explicitly setting the worker thread's affinity 
	}
}

void ServiceHandler::init(ServiceState& _return, const Topology& topology, const HostAddresses& addresses, const HostID hostID) 
{
	CheckNotInHive();

	// Setup the hive state 
	_hiveState = boost::shared_ptr<HiveState>(new HiveState());
	_hiveState->__set_topologyID(topology.topologyID);
	_hiveState->__set_reportingHostID(hostID);

	// Set up service states
	for (auto host = topology.hosts.cbegin(); host != topology.hosts.cend(); ++host)
	{
		// Determine the address for the host
		auto hostAddress = addresses.find(host->first);
		if (hostAddress == addresses.end())
		{
			ostringstream error;
			error << "An address has not been given for host (" << host->first << ").";
			throw std::exception(error.str().c_str());
		}

		ServiceState state;
		state.__set_address(hostAddress->second);

		// If the host is this one, fill in details
		if (host->first == hostID)
		{
			state.__set_status(ServiceStatus::Online);
			state.__set_timeStamp((TimeStamp)time(nullptr));

			// Set the state for each of our workers
			int workerIndex = 0;
			for (auto pod = host->second.cbegin(); pod != host->second.cend(); ++pod)
			{
				WorkerState workerState;
				workerState.__set_podID(pod->first);
				workerState.__set_port(_config->address.port + workerIndex + 1);
				for (auto column = pod->second.cbegin(); column != pod->second.cend(); ++column)
					workerState.repositoryStatus.insert(pair<ColumnID, RepositoryStatus::type>(*column, RepositoryStatus::Online));
				state.workers.push_back(workerState);
				++workerIndex;
			}

			// Set the configuration's address if it wasn't given
			if (!_config->__isset.address)
				_config->__set_address(hostAddress->second);

			_return = state;
		}
		else
		{
			state.__set_status(ServiceStatus::Unknown);
			state.__set_timeStamp(0);
		}

		_hiveState->services.insert(pair<int, ServiceState>(host->first, state));
	}

	// Update the configuration
	_config->__set_joinedHive(*_hiveState);
	SaveConfiguration();

	// Initialize workers
	InitializeWorkers(_return.workers);
}

void ServiceHandler::join(ServiceState& _return, const HiveState& hiveState, const NetworkAddress& address, const HostID hostID) 
{
	CheckNotInHive();

	// Your implementation goes here
	printf("join\n");
}

void ServiceHandler::leave() 
{
	CheckInHive();

	// Your implementation goes here
	printf("leave\n");
}

void ServiceHandler::CheckNotInHive()
{
	if (_hiveState)
		throw AlreadyJoined();
}

void ServiceHandler::CheckInHive()
{
	if (!_hiveState)
	{
		NotJoined notJoined;
		notJoined.__set_potentialWorkers(GetRecommendedWorkerCount());
		throw notJoined;
	}
}

void ServiceHandler::getHiveState(HiveState& _return, const bool forceUpdate) 
{
	CheckInHive();
	_return = *_hiveState;
	// TODO: implement force
}

void ServiceHandler::getState(ServiceState& _return)
{
	auto currentState = _hiveState->services.find(_hiveState->reportingHostID);

	_return.__set_status(ServiceStatus::Online);
	_return.__set_timeStamp((TimeStamp)time(nullptr));
	_return.__set_address(currentState->second.address);

	std::vector<WorkerState> workerStates;

	auto workers = currentState->second.workers;

	for (auto iter = workers.begin(); iter != workers.end(); ++iter)
	{
		boost::shared_ptr<TSocket> socket(new TSocket(_config->address.name, iter->port));
		boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

		WorkerClient client(protocol);
		try
		{
			transport->open();
			WorkerState state;
			client.getState(state);
			state.__set_port(iter->port);
			transport->close();
			workerStates.push_back(state);
		}
		catch (...)
		{
			WorkerState state;
			state.__set_podID(iter->podID);
			state.__set_port(iter->port);
			workerStates.push_back(state);			
		}		
	}

	_return.__set_workers(workerStates);
}

LockID ServiceHandler::acquireLock(const LockName& name, const LockMode::type mode, const LockTimeout timeout) {
// Your implementation goes here
printf("AcquireLock\n");

return 0;
}

void ServiceHandler::keepLock(const LockID lockID) {
// Your implementation goes here
printf("KeepLock\n");
}

void ServiceHandler::escalateLock(const LockID lockID, const LockTimeout timeout) {
// Your implementation goes here
printf("EscalateLock\n");
}

void ServiceHandler::releaseLock(const LockID lockID) {
// Your implementation goes here
printf("ReleaseLock\n");
}
