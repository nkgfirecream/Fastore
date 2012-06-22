#include "ServiceHandler.h"
#include <iostream>
#include <thrift/transport/TSimpleFileTransport.h>
#include <thrift/protocol/TJSONProtocol.h>
#include "WorkerHandler.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <functional>

using namespace boost::filesystem;
using namespace std;

const char* const ServiceHandler::ConfigFileName = "config.json";

ServiceHandler::ServiceHandler(const ServiceStartup& startup)
{
	bool newInstance = false;

	auto configFileName = path(startup.path) / path(ConfigFileName);
	if (exists(configFileName))
	{
		cout << "Existing instance:" << startup.path;

		// Open and hold the configuration file
		_configFile = boost::shared_ptr<TSimpleFileTransport>(new TSimpleFileTransport(configFileName.string(), false));

		// Read the configuration
		TJSONProtocol reader(_configFile);
		_config = boost::shared_ptr<ServiceConfig>(new ServiceConfig());
		_config->read(&reader);
	}
	else
	{
		newInstance = true;
		cout << "New instance:" << startup.path;

		// Create new configuration
		_configFile = boost::shared_ptr<TSimpleFileTransport>(new TSimpleFileTransport(configFileName.string(), false));
		_config = boost::shared_ptr<ServiceConfig>(new ServiceConfig());
	}

	// Apply overrides/initialization from startup
	if (startup.__isset.port)
		_config->address.__set_port(startup.port);
	if (startup.__isset.address)
		_config->address.__set_name(startup.address);
	_config->__set_path(startup.path);
	auto maxWorkerNumber = 0;
	if (startup.__isset.workerPaths)
		for (auto wp = startup.workerPaths.begin(); wp != startup.workerPaths.end(); ++wp)
		{
			if (maxWorkerNumber < wp->first)
				maxWorkerNumber = wp->first;
			_config->workerPaths[wp->first] = wp->second;
		}

	// Determine the number of hardware cores/threads
	auto cores = GetRecommendedWorkerCount();

	// Determine the worker count to use
	auto workerCount = cores > maxWorkerNumber ? cores : maxWorkerNumber;
	cout << "Hardware cores/threads: " << cores << "	Max configured worker: " << maxWorkerNumber << "	Initializing " << workerCount << " workers";
	 
	// Save the new configuration
	if (newInstance)
		SaveConfiguration();

	// Initialize the topology, if joined
	if (_config->__isset.joinedTopology)
		InitializeJoined(_config->joinedTopology);
}

int ServiceHandler::GetRecommendedWorkerCount()
{
	return (int)boost::thread::hardware_concurrency();
}

std::string ServiceHandler::EnsureWorkerPath(int workerNumber)
{
	auto found = _config->workerPaths.find(workerNumber);
	if (found == _config->workerPaths.end())
	{
		stringstream folder;
		folder << "worker" << workerNumber;
		auto workerPath = path(_config->path) / path(folder.str());
		auto result = workerPath.string();
		_config->workerPaths[workerNumber] = result;
		return result;
	}
	else
		return found->second;
}

void ServiceHandler::SaveConfiguration()
{
	// Write the configuration for the new instance
	TJSONProtocol writer(_configFile);
	_config->write(&writer);
}

void ServiceHandler::InitializeJoined(const JoinedTopology& joined)
{
	for (auto worker = joined.workerPodIDs.begin(); worker != joined.workerPodIDs.end(); ++worker)
	{
		// Create a handler for the worker
		auto handler = boost::shared_ptr<WorkerHandler>(new WorkerHandler(worker->second, _config->workerPaths[worker->first]));
		auto processor = boost::shared_ptr<TProcessor>(new WorkerProcessor(handler));
		
		// Create an endpoint for the worker
		EndpointConfig endPointConfig;
		endPointConfig.port = _config->address.port + worker->first;
		auto endpoint = boost::shared_ptr<Endpoint>(new Endpoint(endPointConfig, processor));
		
		// Track the endpoint by worker number
		_workers[worker->first] = endpoint;

		// Start the endpoint's thread
		boost::thread workerThread
		(
			[endpoint, worker]() -> void 
			{ 
				try
				{
					endpoint->Run();
				}
				catch (const exception& e)
				{
					cout << "ERROR: unhandled exception running the endpoint for worker " << worker->first << ": " << e.what();
				}
				catch (...)
				{
					cout << "ERROR: unhandled exception running the endpoint for worker " << worker->first << ".";
				} 
			}
		);
		// TODO: experiment with explicitly setting the worker thread's affinity 
	}
}

void ServiceHandler::init(ServiceState& _return, const Topology& topology, const HostAddresses& addresses, const HostID hostID) 
{
	CheckNotInHive();

	_hiveState = boost::shared_ptr<HiveState>(new HiveState());

	_return.__set_address(addresses.find(hostID)->second);
	if (!_return.address.__isset.port)
		_return.address.__set_port(_config->address.port);
	_return.__set_status(ServiceStatus::Online);
	// TODO: Implement state timestamping
	_return.__set_timeStamp(0);
	// TODO: Finish this
	// InitializedJoinedTopology(
	//_return.workers.insert ... each worker
	_hiveState->services.insert(pair<int, ServiceState>(1, _return));
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

void ServiceHandler::getState(ServiceState& _return) {
// Your implementation goes here
printf("GetState\n");
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
