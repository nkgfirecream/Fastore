#include "ServiceHandler.h"
#include <thrift/transport/TSimpleFileTransport.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/transport/TSocket.h>
#include "WorkerHandler.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>

#include <ctime>

#include "../FastoreCommon/Log/Syslog.h"

using namespace boost::filesystem;
using boost::shared_ptr;
using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::fastore::communication;
using namespace ::fastore::server;

using fastore::Log;
using fastore::log_endl;
using fastore::log_info;
using fastore::log_err;

const char* const ServiceHandler::ConfigFileName = "config.dat";

ServiceHandler::ServiceHandler(const ServiceStartup& startup)
{
	bool configChanged = false;

	_configPath = path(startup.path) / path(ConfigFileName);
	if (exists(_configPath))
	{
		Log << log_info << __func__ 
			<< ": Existing instance: " << startup.path << log_endl;

		// Open and hold the configuration file
		boost::shared_ptr<TFDTransport>_configFile = boost::shared_ptr<TFDTransport>(new TFDTransport(open(_configPath.string().c_str(), O_RDONLY), TFDTransport::CLOSE_ON_DESTROY));

		// Read the configuration
		TBinaryProtocol reader(_configFile);
		_config = boost::shared_ptr<ServiceConfig>(new ServiceConfig());
		_config->read(&reader);

		_configFile->close();
	}
	else
	{
		configChanged = true;
		Log << log_info << __func__ 
			<< ": New instance: " << startup.path << log_endl;

		// Create new configuration
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
	if (startup.__isset.workerPaths)
	{
		// Should it be possible to set the list to be empty as an override? If so, remove false from next line. 
		if (false && startup.workerPaths.empty()) {
			throw logic_error("startup.workerPaths is empty but isset is true");
		}
		_config->workerPaths.resize(startup.workerPaths.size());
		copy( startup.workerPaths.begin(), startup.workerPaths.end(), _config->workerPaths.begin() );
		configChanged = true;
	}

	// Save the new configuration if needed
	//if (configChanged)
		//SaveConfiguration();

	if (_config->__isset.joinedHive)
	{
		_hiveState = boost::shared_ptr<HiveState>(new HiveState(_config->joinedHive));
		auto thisService = _config->joinedHive.services.find(_config->joinedHive.reportingHostID);
		auto numPods = thisService->second.workers.size();

		// Report the number of configured pods
		cout << "Number of Pods: " << numPods 
			 << "	Recommended workers: " << GetRecommendedWorkerCount() << " (these should be the same)\r\n";
	
		// Initialize store
		initializeStore(thisService->second.store);
		
		// Initialize the workers, if joined
		initializeWorkers(thisService->second.workers);
	}
	else
	{
		// Display worker recommendations
		cout << "Hardware cores/threads: " << (int)boost::thread::hardware_concurrency() 
			 << "	Recommended workers: " << GetRecommendedWorkerCount() << "\r\n";
	}
}

int ServiceHandler::GetRecommendedWorkerCount()
{
	// Use all processors but one, but ensure that at least worker is created
	return max(1, (int)boost::thread::hardware_concurrency() - 1);
}

void ServiceHandler::EnsureWorkerPaths(size_t numWorkers)
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
	#ifndef _WIN32
	  mode_t mode = S_IRUSR | S_IWUSR| S_IRGRP | S_IROTH;
	#else
	  int mode = _S_IREAD | _S_IWRITE;
	#endif
	 
	int fd = open(_configPath.string().c_str(), O_CREAT | O_TRUNC|  O_WRONLY, mode);
	if (fd < 0)
		throw "Failed to config for writing";

	boost::shared_ptr<TFDTransport>_configFile = boost::shared_ptr<TFDTransport>(new TFDTransport(fd, TFDTransport::CLOSE_ON_DESTROY));
	// Write the configuration for the new instance
	TBinaryProtocol writer(_configFile);
	_config->write(&writer);

	_configFile->flush();
	_configFile->close();
}

void ServiceHandler::initializeWorkers(const vector<WorkerState>& states)
{
	// Ensure that there enough configured paths for each worker
	EnsureWorkerPaths((int)states.size());
	///	const static char *func = __func__;

	// Constructor starts each Worker running.  
	// If Worker::endpoint::run throws an exception, 
	// it's caught and logged by Worker::run().  
	std::transform( states.begin(), states.end(), 
					_config->workerPaths.begin(), 
					std::back_inserter(_workers), 
					Worker::InitWith( boost::shared_ptr<Scheduler>() ) );
}

void ServiceHandler::initializeStore(const StoreState& store)
{
	_store = std::unique_ptr<Store>(new Store(_config->path, store.port, _scheduler));
}

void ServiceHandler::ping()
{
	// Do nothing
}

void ServiceHandler::init(ServiceState& _return, const Topology& topology, const HostAddresses& addresses, const HostID hostID) 
{
	Log << log_info << "ServiceHandler::init for host " << hostID << ", "
		<< topology.hosts.size() << " hosts " 
		<< " with " << addresses.size() << " addresses" << log_endl;
	
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
			throw std::runtime_error(error.str());
		}

		ServiceState state;
		state.__set_address(hostAddress->second);


		// Start store at service port + 1
		StoreState storeState;	
		storeState.__set_port(hostAddress->second.port + 1);

		state.__set_store(storeState);

		//Log << log_info << __func__ << ": host " << host->first 
		//	<< " has address " << hostAddress->second << log_endl;

		// If the host is this one, fill in details
		if (host->first == hostID)
		{
			state.__set_status(ServiceStatus::Online);
			state.__set_timeStamp((TimeStamp)time(nullptr));

			// Set the state for each of our workers
			int workerIndex = 1;  // Reserve port address + 1 for store
			Log << log_info << __func__ << ": " << host->second.pods.size() 
				<< " pods " << log_endl;
			for (auto pod = host->second.pods.cbegin(); pod != host->second.pods.cend(); ++pod)
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

		_hiveState->services.insert(pair<HostID, ServiceState>(host->first, state));
	}

	// Update the configuration
	_config->__set_joinedHive(*_hiveState);	

	_scheduler = boost::shared_ptr<Scheduler>(new Scheduler(_config->address));

	// Initialize store
	initializeStore(_return.store);

	// Initialize workers
	initializeWorkers(_return.workers);	

	// Start scheduler running... Or should it start on the first callback?
	_scheduler->start();

	//SaveConfiguration();
}

void ServiceHandler::join(ServiceState& _return, const HiveState& hiveState, const NetworkAddress& address, const HostID hostID) 
{
	CheckNotInHive();

	// Your implementation goes here
	printf("join\n");
}

void ServiceHandler::checkpoint(const ColumnIDs& columnIDs)
{
	SaveConfiguration();

	//TODO: Request stores checkpoint
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
		try 
		{
			notJoined.__set_potentialWorkers(GetRecommendedWorkerCount());
		}
		catch( const TTransportException& err ) {
			Log << log_err << __func__ << ": " << err.what() << log_endl;
			throw;
		}
		Log << log_err << __func__ << ": not joined"  << log_endl;
		throw notJoined;
	}
}

void ServiceHandler::getHiveState(OptionalHiveState& _return, const bool forceUpdate) 
{
	if (!_hiveState)
		_return.__set_potentialWorkers(GetRecommendedWorkerCount());
	else
		_return.__set_hiveState(*_hiveState);
	// TODO: implement force
}

void ServiceHandler::getState(OptionalServiceState& _return)
{
	if (!_hiveState)
		_return.__set_potentialWorkers(GetRecommendedWorkerCount());
	else
	{
		auto currentState = _hiveState->services.find(_hiveState->reportingHostID);

		_return.__isset.serviceState = true;
		_return.serviceState.__set_status(ServiceStatus::Online);
		_return.serviceState.__set_timeStamp((TimeStamp)time(nullptr));
		_return.serviceState.__set_address(currentState->second.address);

		std::vector<WorkerState> workerStates;

		auto workers = currentState->second.workers;

		for (auto iter = workers.begin(); iter != workers.end(); ++iter)
		{
			const string& name( _return.serviceState.address.name );
			Log << log_info << __func__ 
				<< ": creating worker on pod" << iter->podID 
				<< " at '" << name << ":" << iter->port << "'" << log_endl;
			boost::shared_ptr<TSocket> socket(new TSocket(name, 
														  int(iter->port)));
			socket->setConnTimeout(2000);
			socket->setRecvTimeout(2000);
			socket->setSendTimeout(2000);
			socket->setMaxRecvRetries(3);

			boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
			boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

			WorkerClient client(protocol);
			try
			{
				Log << log_info << __func__ << ": opening transport" << log_endl;
				transport->open();
				WorkerState state;
				client.getState(state);
				state.__set_port(iter->port);
				transport->close();
				workerStates.push_back(state);
				Log << log_info << __func__ << ": opened  transport" << log_endl;
			}
			catch (...)
			{
				WorkerState state;
				state.__set_podID(iter->podID);
				state.__set_port(iter->port);
				workerStates.push_back(state);			
				Log << log_info << __func__ << ": failed" << log_endl;
			}		
		}

		_return.serviceState.__set_workers(workerStates);
	}
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

void ServiceHandler::handlerError(void* ctx, const char* fn_name)
{
	//_currentConnection->park();

	//SHUT DOWN EVERYTHING!! FAIL FAST FAIL HARD! KILL THE HIVE IF SOMETHING GOES WRONG!
	//(Not really, just testing things)
	//_currentConnection->getServer()->stop();
}

void ServiceHandler::shutdown()
{
	if (_hiveState != NULL)
	{
		auto currentState = _hiveState->services.find(_hiveState->reportingHostID);

		auto workers = currentState->second.workers;

		for (auto iter = workers.begin(); iter != workers.end(); ++iter)
		{
			try
			{
				boost::shared_ptr<TSocket> socket(new TSocket(_config->address.name, int(iter->port)));
				boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
				boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

				WorkerClient client(protocol);
				transport->open();
				client.shutdown();
				transport->close();
			}
			catch(...)
			{
				//For now we expect a transport exception upon shutdown, since the server will immediately terminate.
				//We will want something more graceful in the future.
				continue;
			}
		}


		//Shutdown store thead
		try
		{
			boost::shared_ptr<TSocket> socket(new TSocket(_config->address.name, int(currentState->second.store.port)));
			boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
			boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

			StoreClient client(protocol);
			transport->open();
			//client.shutdown();
			transport->close();
		}
		catch(...)
		{
	
		}

		//for (auto iter = _workers.begin(); iter != _workers.end(); ++iter)
		//{

		//}
	}

	_currentConnection->getServer()->shutdown();
}

void* ServiceHandler::getContext(const char* fn_name, void* serverContext)
{
	_currentConnection = (apache::thrift::server::TFastoreServer::TConnection*)serverContext;

	return NULL;
}
