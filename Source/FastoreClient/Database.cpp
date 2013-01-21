#include "Database.h"
#include "Dictionary.h"
#include <Schema/Dictionary.h>
#include <boost/format.hpp>
#include "Encoder.h"
#include <future>

//#include "../FastoreCore/safe_cast.h"
#include <Log/Syslog.h>
#include <Type/Standardtypes.h>

using namespace std;
using namespace fastore::client;
using fastore::Log;
using fastore::log_endl;
using fastore::log_info;
using fastore::log_err;

const int &Database::getWriteTimeout() const
{
	return _writeTimeout;
}

void Database::setWriteTimeout(const int &value)
{
	if (value < -1)
		throw std::runtime_error("WriteTimeout must be -1 or greater.");
	_writeTimeout = value;
}

Database::Database(std::vector<ServiceAddress> addresses)
	:	
	_nextSystemWorker(0) , 
	_writeTimeout(DefaultWriteTimeout),
	_services
	(
		[](boost::shared_ptr<TProtocol> proto) { return ServiceClient(proto); }, 
		[&](HostID i) { return this->GetServiceAddress(i); }, 
		[](ServiceClient& client) { client.getInputProtocol()->getTransport()->close(); }, 
		[](ServiceClient& client) { return client.getInputProtocol()->getTransport()->isOpen();}
	),
	_stores
	(
		[](boost::shared_ptr<TProtocol> proto) { return StoreClient(proto); }, 
		[&](HostID i) { return this->GetStoreAddress(i); }, 
		[](StoreClient& client) { client.getInputProtocol()->getTransport()->close(); }, 
		[](StoreClient& client) { return client.getInputProtocol()->getTransport()->isOpen();}
	),
	_workers
	(
		[](boost::shared_ptr<TProtocol> proto) { return WorkerClient(proto); }, 
		[&](PodID i) { return this->GetWorkerAddress(i); }, 
		[](WorkerClient& client) { client.getInputProtocol()->getTransport()->close(); }, 
		[](WorkerClient& client) { return client.getInputProtocol()->getTransport()->isOpen();}
	),
	_lock(boost::shared_ptr<boost::mutex>(new boost::mutex()))
{
	// Convert from service addresses to network addresses
	std::vector<NetworkAddress> networkAddresses;
	for (auto a : addresses)
	{
		NetworkAddress n;
		n.__set_name(a.Name);
		n.__set_port(a.Port);
		networkAddresses.push_back(n);
	}

	// Number of potential workers for each service (in case we need to create the hive)
	// If we need to init a hive, we have to assign the various services host ids
	// For now, their position in the network address vector is their Id
	std::map<HostID, int> numWorkersByHost;
	std::map<HostID, NetworkAddress> addressesByHost;
	for (size_t i = 0; i < networkAddresses.size(); i++)
	{
		NetworkAddress address = networkAddresses[i];
		auto service = _services.Connect(address);
		try
		{
			// Discover the state of the entire hive from the given service
			OptionalHiveState hiveStateResult;
			service.getHiveState(hiveStateResult, false);
			if (!hiveStateResult.__isset.hiveState)
			{  	/* 
				 * If no hive state is given, the service is not joined. 
				 * We should proceed to discover the number of potential 
				 * workers for the rest of the services, ensuring that they 
				 * are all not joined.
				 */
				numWorkersByHost[i] = hiveStateResult.potentialWorkers;
				addressesByHost[i] = address;
				_services.Destroy(service);
				//Log << log_err << __func__ << ": failed to join "
				//	<< networkAddresses[i] << log_endl;
				continue;
			}

			// If we have passed the first host, we are in "discovery" mode for a new topology so we find any services that are joined.
			if (i > 0)
				throw ClientException(boost::str(boost::format("Service '{0}' is joined to topology {1}, while at least one other specified service is not part of any topology.") % networkAddresses[i].name % hiveStateResult.hiveState.topologyID));
			
			UpdateHiveStateCache(hiveStateResult.hiveState);

			//Everything worked... exit function
				//Log << log_err << __func__ << ": joined "
				//	<< networkAddresses[i] << log_endl;
			return;
		}
		// If anything goes wrong, be sure to release the service client
		catch(const std::exception& e)
		{
			//Log << log_err << __func__ << ": could not connect to hive on "
			//	<< networkAddresses[i] << ": " << e.what() << log_endl;
			_services.Destroy(service);
			throw;
		}
		catch (...)
		{
			Log << log_err << __func__ << ": dive! dive!" << log_endl;
			_services.Destroy(service);
			throw;	// perhaps just exit()? 
		}
	}

	//Couldn't locate a hive state. The hive is (presumably) uninitialized.

	//Create a new topology instead. Base the topology to be created on number of possible workers reported.
	auto newTopology = CreateTopology(numWorkersByHost);

	HiveState newHive;
	newHive.__set_topologyID(newTopology.topologyID);
	newHive.__set_services(std::map<HostID, ServiceState>());

	for (auto begin = addressesByHost.begin(), end = addressesByHost.end(); begin != end; ++begin)
	{
		auto service = _services.Connect(begin->second);
		try
		{
			ServiceState serviceState;
			service.init(serviceState, newTopology, addressesByHost, begin->first);
			newHive.services[begin->first] = serviceState;
		}
		catch(const std::exception& e)
		{
			Log << log_err << __func__ << ": " << e.what() << log_endl;
			_services.Destroy(service);
		}
	}

	if (newHive.services.empty()) {
		const static char msg[] = "no services created for new hive";
		Log << log_err << __func__ << ": " << msg << log_endl;
		throw runtime_error(msg);
	}

	UpdateHiveStateCache(newHive);
	UpdateTopologySchema(newTopology);
}

void Database::UpdateTopologySchema(const Topology &newTopology)
{
	std::map<ColumnID, ColumnWrites> writes;

	// Insert the topology
	std::vector<fastore::communication::Cell> tempVector;

	fastore::communication::Cell inc;
	inc.__set_rowID(Encoder<TopologyID>::Encode(newTopology.topologyID));
	inc.__set_value(Encoder<TopologyID>::Encode(newTopology.topologyID));

	tempVector.push_back(inc);

	ColumnWrites topoWrites;
	topoWrites.__set_includes(tempVector);
	writes.insert(std::make_pair(fastore::common::Dictionary::TopologyID, topoWrites));

	// Insert the hosts and pods
	ColumnWrites hostWrites;
	hostWrites.__set_includes(std::vector<fastore::communication::Cell>());

	ColumnWrites podWrites;
	podWrites.__set_includes(std::vector<fastore::communication::Cell>());

	ColumnWrites podHostWrites;
	podHostWrites.__set_includes(std::vector<fastore::communication::Cell>());

	ColumnWrites stashWrites;


	for (auto h = newTopology.hosts.begin(); h != newTopology.hosts.end(); ++h)
	{
		fastore::communication::Cell inc;		
		inc.__set_rowID(Encoder<HostID>::Encode(h->first));
		inc.__set_value(Encoder<HostID>::Encode(h->first));
		hostWrites.includes.push_back(inc);

		for (auto p = h->second.pods.begin(); p != h->second.pods.end(); ++p)
		{
			fastore::communication::Cell pwinc;
			pwinc.__set_rowID(Encoder<PodID>::Encode(p->first));
			pwinc.__set_value(Encoder<PodID>::Encode(p->first));
			podWrites.includes.push_back(pwinc);

			fastore::communication::Cell phinc;
			phinc.__set_rowID(Encoder<PodID>::Encode(p->first));
			phinc.__set_value(Encoder<HostID>::Encode(h->first));
			podHostWrites.includes.push_back(phinc);
		}
	}

	writes.insert(std::make_pair(fastore::common::Dictionary::HostID, hostWrites));
	writes.insert(std::make_pair(fastore::common::Dictionary::PodID, podWrites));
	writes.insert(std::make_pair(fastore::common::Dictionary::PodHostID, podHostWrites));

	apply(writes, false);
}

Topology Database::CreateTopology(const std::map<HostID, int>& serviceWorkers)
{
	Topology newTopology;
	newTopology.__set_topologyID(generateTransactionID());
	newTopology.__set_hosts(std::map<HostID, Host>());
	PodID podID = 0;
	for (auto begin = serviceWorkers.begin(), end = serviceWorkers.end(); begin != end; ++begin)
	{
		Host host;
		host.__set_pods(std::map<PodID, std::vector<ColumnID>>());
		auto &pods = host.pods;

		for (int i = 0; i < begin->second; ++i)
		{
			pods.insert(std::make_pair(podID, std::vector<ColumnID>())); // No columns initially
			podID++;
		}

		newTopology.hosts.insert(std::make_pair(begin->first, host));
	}

	return newTopology;
}

Database::~Database()
{
	std::vector<std::function<void()>> cleanupActions;
	//Disposing of connections is now handled in in the connection pool destructors.
	//We need to call them explicitly to make sure connections are cleaned in spite of any errors.
	//TODO: Make sure multiple calls to destructor of connection pool doesn't cause problems.
	//TODO: Perhaps make this a pointer again and pass null to show we are sending an empty state.
	cleanupActions.push_back([&](){ UpdateHiveStateCache(HiveState()); });

	ClientException::ForceCleanup(cleanupActions);
}

//TODO: consider rewriting these in terms of a scoped_lock so that it's always released when it goes out of scope.
// This requires including more of the boost library. (Or just write some type of autolock).
NetworkAddress Database::GetServiceAddress(HostID hostID)
{
	_lock->lock();
	try
	{
		auto result = _hiveState.services.find(hostID);
		if (result == _hiveState.services.end())
		{
			throw ClientException(boost::str(boost::format("No service is currently associated with host ID ({0}).") % hostID));
		}
		else
		{
			_lock->unlock();
			return result->second.address;
		}				
	}
	catch(const std::exception& e)
	{
		_lock->unlock();
		throw e;
	}
}

NetworkAddress Database::GetStoreAddress(HostID hostID)
{
	_lock->lock();
	try
	{
		auto result = _hiveState.services.find(hostID);
		if (result == _hiveState.services.end())
		{
			throw ClientException(boost::str(boost::format("No service is currently associated with host ID ({0}) so store is not available.") % hostID));
		}
		else
		{
			NetworkAddress address;
			address.__set_name(result->second.address.name);
			address.__set_port(result->second.store.port);
			_lock->unlock();
			return address;
		}				
	}
	catch(const std::exception& e)
	{
		_lock->unlock();
		throw e;
	}
}

HiveState Database::QueryHiveForState()
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
							throw ClientException(boost::str(boost::format("Host ({0}) is unexpectedly not part of the topology.") % service.first));
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

void Database::RefreshHiveStateCache()
{
	HiveState newState =QueryHiveForState();
	UpdateHiveStateCache(newState);
}

void Database::UpdateHiveStateCache(const HiveState &newState)
{
	_lock->lock();
	_hiveState = newState;

	// Maintain some indexes for quick access
	_workerStates.clear();
	_columnWorkers.clear();

	if (newState.services.size() > 0)
	{
		for (auto service = newState.services.begin(); service != newState.services.end(); ++service)
		{
			for (auto  worker = service->second.workers.begin(); worker != service->second.workers.end(); ++worker)
			{
				_workerStates.insert(std::pair<PodID, std::pair<ServiceState, WorkerState>>(worker->podID, std::make_pair(service->second, *worker)));
				//TODO: Don't assume all repos are online.
				for (auto repo = worker->repositoryStatus.begin(); repo != worker->repositoryStatus.end(); ++repo) //.Where(r => r.Value == RepositoryStatus.Online || r.Value == RepositoryStatus.Checkpointing)
				{
					auto currentMap = _columnWorkers.find(repo->first);
					if (currentMap == _columnWorkers.end())
					{
						_columnWorkers.insert(std::make_pair(repo->first, PodMap()));
						currentMap = _columnWorkers.find(repo->first);
					}

					currentMap->second.Pods.push_back(worker->podID);
				}
			}
		}
	}

	_lock->unlock();
}

NetworkAddress Database::GetWorkerAddress(PodID podID)
{
	_lock->lock();		
	auto iter = _workerStates.find(podID);
	if (iter == _workerStates.end())
	{
		_lock->unlock();
		throw ClientException(boost::str(boost::format("No Worker is currently associated with pod ID ({0}).") % podID), ClientException::Codes::NoWorkerForColumn);
	}
	else
	{
		NetworkAddress na;
		na.__set_name(iter->second.first.address.name);
		na.__set_port(iter->second.second.port);
		_lock->unlock();
		return na;
	}
}

std::pair<PodID, WorkerClient> Database::GetWorker(ColumnID columnID)
{
	if (columnID <= fastore::common::Dictionary::MaxSystemColumnID)
		return GetWorkerForSystemColumn();
	else
		return GetWorkerForColumn(columnID);
}

std::pair<PodID, WorkerClient> Database::GetWorkerForColumn(ColumnID columnID)
{
	auto podId = GetWorkerIDForColumn(columnID);
	return std::pair<PodID, WorkerClient>(podId, _workers[podId]);
}

PodID Database::GetWorkerIDForColumn(ColumnID columnID)
{
	_lock->lock();
	
	auto iter = _columnWorkers.find(columnID);
	if (iter == _columnWorkers.end())
	{
		ClientException error(boost::str(boost::format("No worker is currently available for column ID (%1%).") % columnID), ClientException::Codes::NoWorkerForColumn);
		
		//TODO: Possibly add data to client exception
		// (or build an exception base class analogous to the C# one)
		//error->getData()->Add("ColumnID", columnID);
		_lock->unlock();
		throw error;
	}
	else
	{
		iter->second.Next = (iter->second.Next + 1) 
			                % SAFE_CAST(int, iter->second.Pods.size());

		if (iter->second.Next < 0)
			iter->second.Next = 0;

		auto podId = iter->second.Pods[iter->second.Next];
		_lock->unlock();
		return podId;
	}	
}

std::pair<PodID, WorkerClient> Database::GetWorkerForSystemColumn()
{
	// For a system column, any worker will do, so just use an already connected worker
	PodID podID;
	_lock->lock();
	podID = std::next(_workerStates.begin(), _nextSystemWorker)->first;
	_nextSystemWorker = (_nextSystemWorker + 1) % SAFE_CAST(int, _workerStates.size());
	_lock->unlock();

	return std::make_pair(podID, _workers[podID]);
}

fastore::client::Database::WorkerColumnBimap Database::DetermineWorkers(const std::map<ColumnID, ColumnWrites>& writes)
{
	auto schema = getSchema();
	_lock->lock();
	WorkerColumnBimap results;
	try
	{
		// Find all the system columns. Each worker has a copy, but they don't list them
		// in their state. (TODO: They probably should, it's just a bootstrapping issue.
		// we have to assume they have them so we can pull schema infomation)
		std::vector<ColumnID> systemColumns;
		for (auto iter = schema.begin(); iter != schema.end(); ++iter)
		{
			if (iter->first <= fastore::common::Dictionary::MaxSystemColumnID)
				systemColumns.push_back(iter->first);
		}

		for (auto ws = _workerStates.begin(); ws != _workerStates.end(); ++ws)
		{
			std::vector<ColumnID> ids;

			//Union with system columns.
			ids.insert(ids.begin(), systemColumns.begin(), systemColumns.end());	

			for (auto repo = ws->second.second.repositoryStatus.begin(); repo != ws->second.second.repositoryStatus.end(); ++repo)
			{
				if (repo->second == RepositoryStatus::Online && repo->first > fastore::common::Dictionary::MaxSystemColumnID)  //The > Dictionary::MaxSystemColumnID ensures we don't accidentally add the system columns twice
					ids.push_back(repo->first);
			}		

			//Sort the columns in workerInfo so we can do a merge.
			std::sort(ids.begin(), ids.end());

			auto writesBegin = writes.begin();
			auto writesEnd = writes.end();

			auto colBegin = ids.begin();
			auto colEnd = ids.end();

			//Add all the pairs to the bimap
			while(true)
			{
				if (writesBegin == writesEnd || colBegin == colEnd)
					break;

				auto infoCol = *colBegin;
				auto writeCol = writesBegin->first;

				if (infoCol == writeCol)
				{
					results.insert(WorkerColumnPair(ws->first, writeCol));
					++colBegin;
				}
				else if (infoCol < writeCol)
					++colBegin;
				else
					++writesBegin;
			}
		}

		// Release lock during ensures
		_lock->unlock();
		return results;
	}
	catch(const std::exception& e)
	{
		_lock->unlock();
		throw e;
	}
}

fastore::client::Database::StoreWorkerBimap Database::DetermineStores(const WorkerColumnBimap& workers)
{
	//TODO: instead of simply returning all the stores, map stores to workers so that only the stores
	//actually serving workers involved in the transactions get the full data set. All other stores
	//should see only the transaction record.
	_lock->lock();
	StoreWorkerBimap results;
	try
	{
		for (auto begin = _hiveState.services.begin(), end = _hiveState.services.end(); begin != end; ++begin)
		{

			for (auto wb = begin->second.workers.begin(), we = begin->second.workers.end(); wb != we; ++wb)
			{
				results.insert(StoreWorkerPair(begin->first, wb->podID));
			}
		}

		// Release lock during ensures
		_lock->unlock();
		return results;
	}
	catch(const std::exception& e)
	{
		_lock->unlock();
		throw e;
	}
}

void Database::AttemptRead(ColumnID columnId, std::function<void(WorkerClient)> work)
{
	std::map<PodID, std::exception> errors;
	clock_t begin;
	clock_t end;
	while (true)
	{
		// Determine the (next) worker to use
		auto worker = GetWorker(columnId);
		try
		{
			// If we've already failed with this worker, aggregate the exception.
			if (errors.size() > 0 && errors.find(worker.first) != errors.end())
			{
				TrackErrors(errors);
				//TODO: aggregate exception for c++
				//throw errors.begin()->second;
			}

			try
			{
				begin = clock();
				work(worker.second);
				end = clock();
			}
			catch (std::exception &e)
			{
				// If the exception is an entity (exception coming from the remote), rethrow
				//TODO: Figure what this will be since TBase doesn't exist in c++ (only c#)
				//if (dynamic_cast<apache::thrift::protocol::t*>(e) != nullptr)
				//	throw;

				Log << log_err << __func__ << ": error: " << e << log_endl;
				errors.insert(std::make_pair(worker.first, e));

				//TODO: Decide criteria for quiting.				
				_workers.Release(worker);
				continue;
			}

			// Succeeded, track any errors we received
			if (errors.size() > 0)
				TrackErrors(errors);

			// Succeeded, track the elapsed time
			TrackTime(worker.first, end - begin);

			_workers.Release(worker);

			break;
		}
		catch(const std::exception& e)
		{
			_workers.Release(worker);
			throw e;
		}

		_workers.Release(worker);
	}
}

void Database::AttemptWrite(PodID podId, std::function<void(WorkerClient)> work)
{
	clock_t begin;
	clock_t end;
	try
	{
		begin = clock();
		WorkerInvoke(podId, work);
		end = clock();
	}
	catch (const std::exception& e)
	{
		// If the exception is an entity (exception coming from the remote), rethrow
		//TODO: Catch only thrift exceptions.
		std::map<PodID, std::exception> errors;
		errors.insert(make_pair(podId, e));
		TrackErrors(errors);

		throw e;
	}

	// Succeeded, track the elapsed time
	TrackTime(podId, end - begin);
}

void Database::TrackTime(PodID podId, long long p)
{
	// TODO: track the time taken by the worker for better routing
}

void Database::TrackErrors(std::map<PodID, std::exception> &errors)
{
	// TODO: stop trying to reach workers that keep giving errors, ask for a state update too
	if( errors.size() ) {
		/*Log << log_info << "Database::" << __func__  << ": "
			<< errors.size() << " errors ... " << log_endl;
		for_each( errors.begin(), errors.end(), 
				  [&]( const std::map<PodID, std::exception>::value_type& e ) {
					  Log << log_info << "    {pod " 
						  << e.first << ", " << e.second.what() 
						  << "}" << log_endl;
				  } );*/
	}
}

boost::shared_ptr<Transaction> Database::begin(bool readsConflict)
{
	return boost::shared_ptr<Transaction>(new Transaction(*this, readsConflict));
}

RangeSet Database::getRange(const ColumnIDs& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId)
{
	// Create the range query
	auto query = CreateQuery(range, limit, startId);

	// Make the range request
	ReadResults results;
	ColumnID col = range.ColumnID;
	AttemptRead
	(
		col, 
		[&, col](WorkerClient client)
		{
			Queries queries;
			queries.insert(std::make_pair(col, query));
			client.query(results, queries);
		}
	);

	auto rangeResult = results[range.ColumnID].answer.rangeValues.at(0);

	// Create the row ID query
	Query rowIdQuery = GetRowsQuery(rangeResult);

	// Get the values for all but the range
	auto result = InternalGetValues(columnIds, range.ColumnID, rowIdQuery);

	// Add the range values into the result
	return ResultsToRangeSet(result, std::find(columnIds.begin(), columnIds.end(), range.ColumnID) - columnIds.begin(), rangeResult);
}

DataSet Database::InternalGetValues(const ColumnIDs& columnIds, const ColumnID exclusionColumnId, const Query& rowIdQuery)
{
	std::vector<boost::shared_ptr<std::future<ReadResults>>> tasks;
	for (size_t i = 0; i < columnIds.size(); ++i)
	{
		auto columnId = columnIds[i];
		if (columnId != exclusionColumnId)
		{
			auto task = boost::shared_ptr<std::future<ReadResults>>
			(
				new std::future<ReadResults>
				(
					std::async
					(
						std::launch::async,
						[&, columnId]()-> ReadResults
						{
							ReadResults result;
							Queries queries;
							queries.insert(std::make_pair(columnId, rowIdQuery));
							AttemptRead
							(
								columnId,
								[&](WorkerClient worker)
								{
									worker.query(result, queries);
								}
							);

							return result;
						}
					)
				)
			);

			tasks.push_back(task);
		}		
	}

	// Combine all results into a single dictionary by column
	ReadResults resultsByColumn;
	for (auto task = tasks.begin(); task != tasks.end(); ++task)
	{
		(*task)->wait();
		auto taskresult = (*task)->get();
		for (auto result = taskresult.begin(); result != taskresult.end(); ++result)
		{
			resultsByColumn.insert(*result);
		}

	}

	return ResultsToDataSet(columnIds, rowIdQuery.rowIDs, resultsByColumn);
}

DataSet Database::getValues(const ColumnIDs& columnIds, const std::vector<std::string>& rowIds)
{
	Query rowIdQuery;
	rowIdQuery.__set_rowIDs(rowIds);

	// Make the query
	return InternalGetValues(columnIds, -1, rowIdQuery);
}

DataSet Database::ResultsToDataSet(const ColumnIDs& columnIds, const std::vector<std::string>& rowIDs, const ReadResults& rowResults)
{
	DataSet result (rowIDs.size(), columnIds.size());

	// Populate by column then by row
	for (size_t y = 0; y < rowIDs.size(); ++y)
	{
		result[y].ID = rowIDs[y];
	}

	for (size_t x = 0; x < columnIds.size(); ++x)
	{
		auto columnId = columnIds[x];
		auto iter = rowResults.find(columnId);
		if (iter != rowResults.end())
		{
			for (size_t y = 0; y < rowIDs.size(); y++)
			{				
				result[y][x] = iter->second.answer.rowIDValues[y];
			}
		}
	}

	return result;
}

RangeSet Database::ResultsToRangeSet(DataSet& set, size_t rangeColumnIndex, const RangeResult& rangeResult)
{
	RangeSet result;

	result.Bof = rangeResult.bof;
	result.Eof = rangeResult.eof;
	result.Limited = rangeResult.limited;

	size_t valueRowValue = 0, valueRowRow = 0;
	auto newGroup = true;
	for (size_t y = 0; y < set.size(); y++)
	{
		set[y].Values[rangeColumnIndex].__set_value(rangeResult.valueRowsList[valueRowValue].value);
		valueRowRow++;
		set[y].newGroup = newGroup;
		newGroup = valueRowRow >= rangeResult.valueRowsList[valueRowValue].rowIDs.size();
		if (newGroup)
		{
			valueRowValue++;
			valueRowRow = 0;
		}
	}

	result.Data = set;

	return result;
}

Query Database::GetRowsQuery(const RangeResult &rangeResult)
{
	std::vector<std::string> rowIds;
	for (auto valuerow = rangeResult.valueRowsList.begin(); valuerow != rangeResult.valueRowsList.end(); ++valuerow)
	{
		for (auto rowid = valuerow->rowIDs.begin(); rowid != valuerow->rowIDs.end(); ++rowid)
			rowIds.push_back(*rowid);
	}

	Query query;
	query.__set_rowIDs(rowIds);
	return query;
}

Query Database::CreateQuery(const Range range, const int limit, const boost::optional<std::string>& startId)
{
	auto rangeRequest = rangeToRangeRequest(range, limit);

	Query rangeQuery;
	rangeQuery.__set_ranges(std::vector<RangeRequest>());
	rangeQuery.ranges.push_back(rangeRequest);

	return rangeQuery;
}

void Database::apply(const std::map<ColumnID, ColumnWrites>& writes, const bool flush)
{
	TransactionID transactionID = generateTransactionID();
	return apply(transactionID, writes, flush);
}

ColumnIDs getColumnIDs(const std::map<ColumnID, ColumnWrites>& writes)
{
	ColumnIDs result;
	for (auto w : writes)
		result.push_back(w.first);
	return result;
}

void Database::apply(TransactionID transactionID, const std::map<ColumnID, ColumnWrites>& writes, const bool flush)
{
	if (writes.size() > 0)
	{
		auto retries = MaxWriteRetries;
		while (true)
		{
			auto workers = DetermineWorkers(writes);
			auto stores = DetermineStores(workers);

			auto tasks = Apply(getColumnIDs(writes), transactionID, workers);

			auto revisionsByColumn = ProcessPrepareResults(workers, tasks);

			// Commit if threshold reached, else rollback and retry
			std::map<ColumnID,Revision> revisions;
			if (CanFinalizeTransaction(revisionsByColumn, revisions))
			{
				//TODO: Handle failed commit. Should be rare since we just prepared.
				Commit(transactionID, writes, revisions, workers, stores);

				bool shouldRefreshSchema = writes.begin()->first <= fastore::common::Dictionary::MaxSystemColumnID;

				if (flush || shouldRefreshSchema)
					Flush(transactionID, stores);
			
				if (shouldRefreshSchema)
				{
					RefreshSchemaCache();
					RefreshHiveStateCache();
				}
		
				break;
			}
			else
			{
				//TODO: Attempt resolution and update.
				//Rollback();
				continue;
			}

			// Fail if exceeded number of retries
			if (--retries <= 0)
				throw ClientException("Maximum number of retries reached attempting to apply writes.");
		}
	}
}

void Database::Commit(const TransactionID transactionID, const std::map<ColumnID, ColumnWrites>& writes, std::map<ColumnID,Revision>& revisions, WorkerColumnBimap& workers, StoreWorkerBimap& stores)
{
	std::vector<std::future<void>> tasks;
	for (auto worker = workers.left.begin(); worker != workers.left.end(); ++worker)
	{
		tasks.push_back
		(
			std::future<void>
			(
				std::async
				(
					std::launch::async,
					[&, worker]()-> void
					{
						AttemptWrite
						(
							worker->first,
							[&](WorkerClient client)
							{
								return client.commit(transactionID, writes);
							}
						);
					}
				)
			)
		);
	}

	for (auto store = stores.left.begin(); store != stores.left.end(); ++store)
	{
		tasks.push_back
		(
			std::future<void>
			(
				std::async
				(
					std::launch::async,
					[&, store]()-> void
					{
						auto client = _stores[store->first];
						bool flushed = false;
						try
						{
							//TODO: convert writes to revisions... Column should take the same.
							client.commit(transactionID, revisions, writes);
							_stores.Release(std::make_pair(store->first, client));
						}
						catch(const std::exception&)
						{
							//TODO: track exception errors. Failed flush is bad.
							if(!flushed)
								_stores.Release(std::make_pair(store->first, client));
						}
					}
				)
			)
		);
	}

	for (size_t i = 0; i < tasks.size(); ++i)
	{
		tasks[i].wait();
	}
}

//std::vector<std::future<PrepareResults>> Database::Prepare(const ColumnIDs &columnIDs, const TransactionID &transactionID, const fastore::client::Database::WorkerColumnBimap& workers)
//{
//	std::vector<std::future<PrepareResults>> tasks;
//	// Apply the modification to every worker, even if no work
//	for (auto worker = workers.left.begin(); worker != workers.left.end(); ++worker)
//	{
//		tasks.push_back
//		(
//			std::future<PrepareResults>
//			(
//				std::async
//				(
//					std::launch::async,
//					[&, worker]()-> PrepareResults
//					{
//						PrepareResults result;
//						AttemptWrite
//						(
//							worker->first,
//							[&](WorkerClient client)
//							{
//								return client.prepare(result, transactionID, columnIDs);
//							}
//						);
//
//						return result;
//					}
//				)
//			)
//		);
//	}
//
//	return tasks;
//}

std::map<PodID, std::future<PrepareResults>> Database::Apply(const ColumnIDs &columnIDs, const TransactionID &transactionID, const fastore::client::Database::WorkerColumnBimap& workers)
{
	std::map<PodID, std::future<PrepareResults>> tasks;
	// Apply the modification to every worker, even if no work
	for (auto worker =  workers.left.begin(); worker != workers.left.end(); ++worker)
	{
		auto podId = worker->first;
		tasks.insert
		(
			std::make_pair
			(
				podId,
				std::future<PrepareResults>
				(
					std::async
					(
						std::launch::async,
						[&, podId, transactionID, columnIDs]()-> PrepareResults
						{
							PrepareResults result;
							AttemptWrite
							(
								podId,
								[&](WorkerClient client)
								{
									return client.apply(result, transactionID, columnIDs);
								}
							);

							return result;
						}
					)
				)
			)
		);
	}

	return tasks;
}

void Database::Flush(const TransactionID& transactionID, const fastore::client::Database::StoreWorkerBimap& stores)
{
	std::vector<std::future<void>> flushTasks;
	for (auto s = stores.left.begin(); s != stores.left.end(); ++s)
	{
		flushTasks.push_back
		(
			std::future<void>
			(
				std::async
				(
					std::launch::async,
					[&,s]()
					{
						auto store = _stores[s->first];
						bool flushed = false;
						try
						{
							//store.flush(transactionID);
							flushed = true;
							_stores.Release(std::make_pair(s->first, store));
						}
						catch(const std::exception&)
						{
							//TODO: track exception errors. Failed flush is bad.
							if(!flushed)
								_stores.Release(std::make_pair(s->first, store));
						}
					}
				)
			)
		);
	}

	// Wait for critical number of stores to flush
	// This is a bit more complicated. If the stores aren't receiving every write we need to make
	// sure that at least two stores for every column has flushed.
	auto neededCount =  stores.size() / 2 > 1 ? stores.size() / 2 : 1;
	for (size_t i = 0; i < flushTasks.size() && i < neededCount; i++)
		flushTasks[i].wait();
}

std::unordered_map<ColumnID, Database::ColumnWriteResult> Database::ProcessPrepareResults(const WorkerColumnBimap& workers, std::map<PodID, std::future<PrepareResults>>& tasks)
{
	clock_t start = clock();

	// Group workers by column, then revision
	std::unordered_map<ColumnID, ColumnWriteResult> columnWriteResults;

	for (auto task = tasks.begin(); task != tasks.end(); ++task)
	{
		// Attempt to fetch the result for each task
		PrepareResults prepareResults;
		try
		{
			clock_t timeToWait = getWriteTimeout() - (clock() - start);
			auto taskStatus = task->second.wait_for(std::chrono::milliseconds(timeToWait > 0 ? timeToWait : 0));
#if __GNUC_MINOR__ == 6
			// ignore what wait_for returns until GNU and Microsoft agree
			if (taskStatus)
#else
			if (taskStatus == std::future_status::ready)
#endif
				prepareResults = task->second.get();
			else
			{
				//Should be caught immediately below
				throw std::exception("Worker not ready yet and timeout has expired");
			}
		}
		catch (std::exception&)
		{
			//Find all the columns associated with this worker and add the worker to their failed list.
			auto workerColumns = workers.left.find(task->first);
			while (workerColumns->first == task->first)
			{
				auto &writeResult = columnWriteResults[workerColumns->second];
				writeResult.failedWorkers.push_back(task->first);
				++workerColumns;
			}
			continue;
		}

		// Successful.  Track workers by column then revision number, and track validation needs for each column
		for (auto prepareResult : prepareResults)
		{
			auto &running = columnWriteResults[prepareResult.first];
			running.validateRequired |= prepareResult.second.validateRequired;
			running.workersByRevision[prepareResult.second.actualRevision].push_back(task->first);
		}
	}

	return columnWriteResults;
}

bool Database::CanFinalizeTransaction(const std::unordered_map<ColumnID, ColumnWriteResult>& columnWriteResultsByColumn, std::map<ColumnID,Revision>& outRevisions)
{
	for (auto begin = columnWriteResultsByColumn.begin(), end = columnWriteResultsByColumn.end(); begin != end; ++begin)
	{
		//TODO: Correct validation.
		auto writeResult = begin->second;

		//Total workers involved for this column:
		size_t totalWorkers = writeResult.failedWorkers.size();
		for (auto beginWorkerByRevision = writeResult.workersByRevision.begin(); beginWorkerByRevision != writeResult.workersByRevision.end(); ++beginWorkerByRevision)
		{
			totalWorkers += beginWorkerByRevision->second.size();
		}

		if (writeResult.validateRequired || 
			writeResult.workersByRevision.size() == 0 ||
			(writeResult.failedWorkers.size() >= totalWorkers / 2) ||
			(writeResult.workersByRevision.rbegin()->second.size() < totalWorkers / 2))
			return false;
		else
			outRevisions[begin->first] = writeResult.workersByRevision.rbegin()->first + 1;
	}

	return true;
}

void Database::WorkerInvoke(PodID podID, std::function<void(WorkerClient)> work)
{
	auto client = _workers[podID];
	bool released = false;
	try
	{
		work(client);
		released = true;
		_workers.Release(std::make_pair(podID, client));
	}
	catch(const std::exception& e)
	{
		if (!released)
			_workers.Release(std::make_pair(podID, client));
		throw e;
	}
}

void Database::ServiceInvoke(HostID podID, std::function<void(ServiceClient)> work)
{
	auto client = _services[podID];
	bool released = false;
	try
	{
		work(client);
		released = true;
		_services.Release(std::make_pair(podID, client));
	}
	catch(const std::exception& e)
	{
		if (!released)
			_services.Release(std::make_pair(podID, client));
		throw e;
	}
}

void Database::include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row)
{
	auto writes = CreateIncludes(columnIds, rowId, row);
	apply(writes, false);
}

std::map<ColumnID, ColumnWrites> Database::CreateIncludes(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row)
{
	std::map<ColumnID, ColumnWrites> writes;

	for (size_t i = 0; i < columnIds.size(); ++i)
	{
		fastore::communication::Cell inc;
		inc.__set_rowID(rowId);
		inc.__set_value(row[i]);

		ColumnWrites wt;
		wt.__set_includes(std::vector<fastore::communication::Cell>());
		wt.includes.push_back(inc);
		writes.insert(std::make_pair(columnIds[i], wt));
	}

	return writes;
}

void Database::exclude(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row)
{
	apply(CreateExcludes(columnIds, rowId, row), false);
}

void Database::exclude(const ColumnIDs& columnIds, const std::string& rowId)
{

	//Pull full row information, and then exclude it.
	//apply(CreateExcludes(columnIds, rowId, row), false);
}

std::map<ColumnID, ColumnWrites> Database::CreateExcludes(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>&  row)
{
	std::map<ColumnID, ColumnWrites> writes;

	for (size_t i = 0; i < columnIds.size(); ++i)
	{
		fastore::communication::Cell ex;
		ex.__set_rowID(rowId);
		ex.__set_value(row[i]);

		ColumnWrites wt;
		wt.__set_excludes(std::vector<fastore::communication::Cell>());
		wt.excludes.push_back(ex);
		writes.insert(std::make_pair(columnIds[i], wt));
	}
	return writes;
}

std::vector<Statistic> Database::getStatistics(const ColumnIDs& columnIds)
{
	// Make the request against each column
	//TODO: Instead of requesting each column individually, combine columns on a single worker.
	std::vector<std::future<Statistic>> tasks;
	for (size_t i = 0; i < columnIds.size(); ++i)
	{
		auto columnId = columnIds[i];
		tasks.push_back
		(
			std::future<Statistic>
			(
				std::async
				(
					std::launch::async,
					[&, columnId]() -> Statistic
					{
						Statistic result;
						AttemptRead
						(
							columnId, 
							[&, columnId](WorkerClient worker)
							{			
								std::vector<ColumnID> ids;
								std::vector<Statistic> stats;
								ids.push_back(columnId);
								worker.getStatistics(stats, ids);
								result = stats[0];
							}
						);
						return result;
					}
				)
			)
		);		
	}

	std::vector<Statistic> stats;
	for (auto iter = tasks.begin(); iter != tasks.end(); ++iter)
	{
		iter->wait();
		stats.push_back(iter->get());
	}

	return stats; 
}

Schema Database::getSchema()
{
	if (_schema.empty())
		_schema = LoadSchema();
	return _schema;
}

Schema Database::LoadSchema()
{
	Schema schema;
	bool finished = false;
	std::string startId = Encoder<ColumnID>::Encode(fastore::common::Dictionary::ColumnID);
	do
	{
		Range range;
		range.Ascending = true;
		range.ColumnID = fastore::common::Dictionary::ColumnID;
		RangeSet columns = getRange(fastore::common::Dictionary::ColumnColumns, range, MaxFetchLimit, startId);

		for (auto column : columns.Data)
		{
			ColumnDef def =
				{
					Encoder<ColumnID>::Decode(column.ID),
					Encoder<std::string>::Decode(column.Values[1].value),
					Encoder<std::string>::Decode(column.Values[2].value),
					Encoder<std::string>::Decode(column.Values[3].value),
					Encoder<BufferType_t>::Decode(column.Values[4].value),
					Encoder<bool>::Decode(column.Values[5].value)
				};

			schema.insert(std::make_pair(def.ColumnID, def));
		}

		if (columns.Data.size() > 0)
			startId =  columns.Data.rbegin()->ID;

		finished = !columns.Limited;
	}
	while(!finished);

	return schema;
}

void Database::RefreshSchemaCache()
{
	_schema.clear();
	_schema = LoadSchema();
}

//Must be called AFTER the HiveState is updated to the current version
//void Database::BootStrapSchema()
//{
//	static const ColumnDef defaults[] =  
//	{ 
//		{ fastore::common::Dictionary::ColumnID, "Column.ID", standardtypes::Long.Name, standardtypes::Long.Name, BufferType_t::Identity, true }, 
//		{ fastore::common::Dictionary::ColumnName, "Column.Name", standardtypes::String.Name, standardtypes::Long.Name, BufferType_t::Unique, true },
//		{ fastore::common::Dictionary::ColumnValueType, "Column.ValueType", standardtypes::String.Name, standardtypes::Long.Name, BufferType_t::Multi, true },
//		{ fastore::common::Dictionary::ColumnRowIDType, "Column.RowIDType", standardtypes::String.Name, standardtypes::Long.Name, BufferType_t::Multi, true },
//		{ fastore::common::Dictionary::ColumnBufferType, "Column.BufferType", standardtypes::Int.Name, standardtypes::Long.Name, BufferType_t::Multi, true },
//		{ fastore::common::Dictionary::ColumnRequired, "Column.Required", standardtypes::Bool.Name, standardtypes::Long.Name, BufferType_t::Multi, true }
//	};	
//
//	for_each( defaults, defaults + sizeof(defaults)/sizeof(defaults[0]), 
//		[&](const ColumnDef& def) 
//		{
//			_schema[def.ColumnID] = def;
//		} );
//
//	_schema = LoadSchema();
//}

std::map<HostID, long long> Database::ping()
{
	HostIDs hostIds;

	_lock->lock();
	for (auto iter = _hiveState.services.begin(); iter != _hiveState.services.end(); ++iter)
	{
		hostIds.push_back(iter->first);
	}
	_lock->unlock();

	std::vector<boost::shared_ptr<std::future<std::pair<HostID, long long>>>> tasks;
	for (size_t i = 0; i < hostIds.size(); ++i)
	{		
		HostID hostId = hostIds[i];
		auto task = boost::shared_ptr<std::future<std::pair<HostID, long long>>>
		(
			new std::future<std::pair<HostID, long long>>
			(
				std::async
				(
					std::launch::async,
					[&]() -> std::pair<HostID, long long>
					{
						auto service = _services[hostId];
						bool released = false;
						clock_t start = 0;
						clock_t stop = 0;
						try
						{
							start = clock();
							service.ping();
							stop = clock();
							released = true;
							_services.Release(std::pair<HostID, ServiceClient>(hostId, service));							
						}
						catch(const std::exception&)
						{
							if (!released)
								_services.Release(std::pair<HostID, ServiceClient>(hostId, service));
						}				

						return std::make_pair(hostId, stop - start);
					}
				)
			)
		);
	}

	// Gather ping results
	std::map<HostID, long long> result;
	for (auto iter = tasks.begin(); iter != tasks.end(); ++iter)
	{
		(*iter)->wait();
		result.insert((*iter)->get());
	}

	return result;
}

void Database::checkpoint()
{
	try
	{
		for (auto iter = _hiveState.services.begin(), end = _hiveState.services.end(); iter != end; ++ iter)
		{
			communication::HostID hostId = iter->first;
			//TODO: needs all columns for all workers on the service
			//ServiceInvoke(hostId, [](fastore::communication::ServiceClient client)-> void { client.checkpoint(i; });
		}
	}
	catch(...)
	{
		throw;
	}
}

TransactionID Database::generateTransactionID()
{
	// Lazy create the generator because creation is relatively expensive
	if (_generator == nullptr)
		_generator = unique_ptr<TransactionIDGenerator>(new TransactionIDGenerator());
	
	return _generator->generate();
}
