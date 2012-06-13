#include "ServiceHandler.h"
#include <iostream>
#include <thrift/transport/TFileTransport.h>
#include <thrift/protocol/TJSONProtocol.h>
#include "WorkerHandler.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

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
		_configFile = boost::shared_ptr<TFileTransport>(new TFileTransport(configFileName.string(), false));

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
		_configFile = boost::shared_ptr<TFileTransport>(new TFileTransport(configFileName.string(), false));
		_config = boost::shared_ptr<ServiceConfig>(new ServiceConfig());
	}

	// Apply overrides/initialization from startup
	if (startup.__isset.port)
		_config->__set_port(startup.port);
	if (startup.__isset.address)
		_config->__set_address(startup.address);
	_config->__set_path(startup.path);
	if (startup.__isset.workerPaths)
		for (auto wp = startup.workerPaths.begin(); wp != startup.workerPaths.end(); ++wp)
			_config->workerPaths[(*wp).first] = (*wp).second;

	// TODO: Initialize the worker paths - at least one to cover each core

	// Save the new configuration
	if (newInstance)
		SaveConfiguration();

	// Initialize the topology, if joined
	if (_config->__isset.joinedTopology)
		InitializeJoined(_config->joinedTopology);
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
		EndpointConfig endPointConfig;
		endPointConfig.port = _config->port + worker->first;
		auto handler = boost::shared_ptr<WorkerHandler>(new WorkerHandler(worker->second, _config->workerPaths[worker->first]));
		auto processor = boost::shared_ptr<TProcessor>(new WorkerProcessor(handler));
		auto endpoint = boost::shared_ptr<Endpoint>(new Endpoint(endPointConfig, processor));
		_workers[worker->first] = endpoint;
		// TODO: put this in a thread:
		endpoint->Run();
	}
}

void ServiceHandler::init(HiveState& _return) 
{
// Your implementation goes here
printf("init\n");
}

void ServiceHandler::join(ServiceState& _return, const HostID hostID, const HiveState& hiveState) {
// Your implementation goes here
printf("join\n");
}

void ServiceHandler::leave() {
// Your implementation goes here
printf("leave\n");
}

void ServiceHandler::getHiveState(HiveState& _return) {
// Your implementation goes here
printf("GetHiveState\n");
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
