#include "Database.h"
#include "Dictionary.h"
#include <boost/format.hpp>
#include "Encoder.h"

#include <future>

#include "../FastoreCore/safe_cast.h"
#include "../FastoreCore/Log/Syslog.h"

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

	// Number of potential workers for each service (in case we nee to create the hive)
	std::vector<int> serviceWorkers(networkAddresses.size());
	for (size_t i = 0; i < networkAddresses.size(); i++)
	{
		auto service = _services.Connect(networkAddresses[i]);
		try
		{
			// Discover the state of the entire hive from the given service
			OptionalHiveState hiveStateResult;
			service.getHiveState(hiveStateResult, false);
			if (!hiveStateResult.__isset.hiveState)
			{
				// If no hive state is given, the service is not joined, we should proceed to discover the number 
				//  of potential workers for the rest of the services ensuring that they are all not joined.
				serviceWorkers[i] = hiveStateResult.potentialWorkers;
				_services.Destroy(service);
				continue;
			}

			// If we have passed the first host, we are in "discovery" mode for a new topology so we find any services that are joined.
			if (i > 0)
				throw ClientException(boost::str(boost::format("Service '{0}' is joined to topology {1}, while at least one other specified service is not part of any topology.") % networkAddresses[i].name % hiveStateResult.hiveState.topologyID));
			
			UpdateHiveState(hiveStateResult.hiveState);
			BootStrapSchema();
			RefreshHiveState();

			//Everything worked... exit function
			return;
		}
		catch (...)
		{
			// If anything goes wrong, be sure to release the service client
			_services.Destroy(service);
			throw;
		}
	}

	//Create a new topology instead.
	auto newTopology = CreateTopology(serviceWorkers);

	std::map<HostID, NetworkAddress> addressesByHost;
	for (size_t hostID = 0; hostID  < networkAddresses.size(); hostID++) {
		addressesByHost[hostID] = networkAddresses[hostID];
	}

	HiveState newHive;
	newHive.__set_topologyID(newTopology.topologyID);
	newHive.__set_services(std::map<HostID, ServiceState>());
	for (size_t hostID = 0; hostID < networkAddresses.size(); hostID++)
	{
		auto service = _services.Connect(networkAddresses[hostID]);
		try
		{
			ServiceState serviceState;
			service.init(serviceState, newTopology, addressesByHost, HostID(hostID));
			newHive.services.insert(std::make_pair(HostID(hostID), serviceState));
		}
		catch(const std::exception&)
		{
			_services.Destroy(service);
		}
	}

	UpdateHiveState(newHive);
	BootStrapSchema();
	UpdateTopologySchema(newTopology);
}

void Database::UpdateTopologySchema(const Topology &newTopology)
{
	auto writes = std::map<ColumnID, boost::shared_ptr<ColumnWrites>>();

	// Insert the topology
	std::vector<fastore::communication::Include> tempVector;

	fastore::communication::Include inc;
	inc.__set_rowID(Encoder<TopologyID>::Encode(newTopology.topologyID));
	inc.__set_value(Encoder<TopologyID>::Encode(newTopology.topologyID));

	tempVector.push_back(inc);

	boost::shared_ptr<ColumnWrites> topoWrites(new ColumnWrites());
	topoWrites->__set_includes(tempVector);
	writes.insert(std::make_pair(Dictionary::TopologyID, topoWrites));

	// Insert the hosts and pods
	boost::shared_ptr<ColumnWrites> hostWrites(new ColumnWrites());
	hostWrites->__set_includes(std::vector<fastore::communication::Include>());

	boost::shared_ptr<ColumnWrites> podWrites(new ColumnWrites());
	podWrites->__set_includes(std::vector<fastore::communication::Include>());

	boost::shared_ptr<ColumnWrites> podHostWrites(new ColumnWrites());
	podHostWrites->__set_includes(std::vector<fastore::communication::Include>());

	for (auto h = newTopology.hosts.begin(); h != newTopology.hosts.end(); ++h)
	{
		fastore::communication::Include inc;		
		inc.__set_rowID(Encoder<HostID>::Encode(h->first));
		inc.__set_value(Encoder<HostID>::Encode(h->first));
		hostWrites->includes.push_back(inc);

		for (auto p = h->second.begin(); p != h->second.end(); ++p)
		{
			fastore::communication::Include pwinc;
			pwinc.__set_rowID(Encoder<PodID>::Encode(p->first));
			pwinc.__set_value(Encoder<PodID>::Encode(p->first));
			podWrites->includes.push_back(pwinc);

			fastore::communication::Include phinc;
			phinc.__set_rowID(Encoder<PodID>::Encode(p->first));
			phinc.__set_value(Encoder<HostID>::Encode(h->first));
			podHostWrites->includes.push_back(phinc);
		}
	}
	writes.insert(std::make_pair(Dictionary::HostID, hostWrites));
	writes.insert(std::make_pair(Dictionary::PodID, podWrites));
	writes.insert(std::make_pair(Dictionary::PodHostID, podHostWrites));

	Apply(writes, false);
}

Topology Database::CreateTopology(const std::vector<int>& serviceWorkers)
{
	Topology newTopology;
	//TODO: Generate ID. It was based on the hashcode of a guid.
	newTopology.__set_topologyID(0);
	newTopology.__set_hosts(std::map<HostID, Pods>());
	auto podID = 0;
	for (size_t hostID = 0; hostID < serviceWorkers.size(); hostID++)
	{
		auto pods = std::map<PodID, std::vector<ColumnID>>();
		for (int i = 0; i < serviceWorkers[hostID]; i++)
		{
			pods.insert(std::make_pair(podID, std::vector<ColumnID>())); // No no columns initially
			podID++;
		}
		newTopology.hosts.insert(std::make_pair(HostID(hostID), pods));
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
	cleanupActions.push_back([&](){ UpdateHiveState(HiveState()); });

	ClientException::ForceCleanup(cleanupActions);
}

//TODO: consider rewriting these in terms of a scoped_lock so that it's always released when it goes out of scope.
// This requires including more of the boost library. (Or just write some type of autolock).
NetworkAddress& Database::GetServiceAddress(HostID hostID)
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

HiveState Database::GetHiveState()
{
	//TODO: Need to actually try to get new worker information, update topologyID, etc.
	//This just assumes that we won't add any workers once we are up and running
	HiveState newHive;
	newHive.__set_topologyID(0);
	newHive.__set_services(std::map<HostID, ServiceState>());

	//Using shared_ptr because the copy constructor of future is private, preventing its direct use in a
	//vector.
	std::vector<boost::shared_ptr<std::future<std::pair<HostID, ServiceState>>>> tasks;
	for (auto service : _hiveState.services)
	{
		auto task = boost::shared_ptr<std::future<std::pair<HostID, ServiceState>>>
		(
			new std::future<std::pair<HostID, ServiceState>>
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

		tasks.push_back(task);
	}

	for (auto task : tasks)
	{
		task->wait();
		auto statePair = task->get();
		newHive.services.insert(statePair);
	}

	//Don't care for now...
	newHive.__set_reportingHostID(newHive.services.begin()->first);

	return newHive;
}

void Database::RefreshHiveState()
{
	HiveState newState = GetHiveState();
	UpdateHiveState(newState);
}

void Database::UpdateHiveState(const HiveState &newState)
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
	if (columnID <= Dictionary::MaxSystemColumnID)
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

std::vector<Database::WorkerInfo> Database::DetermineWorkers(const std::map<ColumnID, boost::shared_ptr<ColumnWrites>> &writes)
{
	_lock->lock();
	std::vector<WorkerInfo> results;
	try
	{
		// TODO: is there a better better scheme than writing to every worker?  This method takes column IDs in case one arises.
		std::vector<ColumnID> systemColumns;
		for (auto iter = _schema.begin(); iter != _schema.end(); ++iter)
		{
			if (iter->first <= Dictionary::MaxSystemColumnID)
				systemColumns.push_back(iter->first);
		}

		for (auto ws = _workerStates.begin(); ws != _workerStates.end(); ++ws)
		{
			WorkerInfo info;
			info.podID = ws->first;

			//Union with system columns.
			info.Columns.insert(info.Columns.begin(), systemColumns.begin(), systemColumns.end());	

			for (auto repo = ws->second.second.repositoryStatus.begin(); repo != ws->second.second.repositoryStatus.end(); ++repo)
			{
				if ((repo->second == RepositoryStatus::Online || repo->second == RepositoryStatus::Checkpointing) && repo->first > Dictionary::MaxSystemColumnID)
					info.Columns.push_back(repo->first);
			}		

			//TODO: fix this nasty double for loop. Sort columns ids and do a search
			for (auto iter = writes.begin(); iter != writes.end(); ++iter)
			{
				bool broke = false;
				for (size_t i = 0; i < info.Columns.size(); ++i)
				{
					if (info.Columns[i] == iter->first)
					{
						broke = true;
						results.push_back(info);
						break;
					}
				}

				if (broke)
					break;
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

void Database::AttemptRead( ColumnID columnId, 
						    std::function<void(WorkerClient)> work )
#if 0
{
	std::map<PodID, std::exception> errors;
	clock_t begin, end;

	// Iterate over pods that can work on a column. 
	for( auto worker = GetWorker(columnId);
		 errors.find(worker.first) == errors.end();	// max one error per worker
		 worker = GetWorker(columnId) )
	{
		try
		{
			begin = clock();
			work(worker.second);
			end = clock();

			// Succeeded: record errors and elapsed time. 
			TrackErrors(errors);
			TrackTime(worker.first, end - begin);

			_workers.Release(worker);
			return;
		}
		// work() throws an exception if unavailable. 
		// Log it and try the next pod. 
		catch (std::exception &e)
		{
			Log << log_err << __func__ << ": error: " << e << log_endl;
			if( errors.find(worker.first) != errors.end() ) {
				// should never have more than one 
				ostringstream oops;
				oops << __FILE__ << ":" << __LINE__ 
					 << " (" << __func__ << ") " 
					 << "two exceptions for PodID" << worker.first;
				throw logic_error(oops.str());
			}
			// If the exception is an entity 
			// (exception coming from the remote), rethrow
			// TODO: Figure what this will be since TBase doesn't exist in c++

			errors[worker.first] = e;
			_workers.Release(worker);	// catch & release ... 
		}
	}
		
	ostringstream oops;
	oops << __FILE__ << ":" << __LINE__ 
		 << " (" << __func__ << ") " 
		 << "All " << errors.size() << " workers "
		 << "failed for columnID " << columnId;
		
	Log << log_err << oops.str() << log_endl;
	throw runtime_error( oops.str() );
}
#else
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
				continue;
			}

			// Succeeded, track any errors we received
			if (errors.size() > 0)
				TrackErrors(errors);

			// Succeeded, track the elapsed time
			TrackTime(worker.first, end - begin);

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
#endif

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
		Log << log_info << "Database::" << __func__  << ": "
			<< errors.size() << " errors ... " << log_endl;
		for_each( errors.begin(), errors.end(), 
				  [&]( const std::map<PodID, std::exception>::value_type& e ) {
					  Log << log_info << "    {pod " 
						  << e.first << ", " << e.second.what() 
						  << "}" << log_endl;
				  } );
	}
}

boost::shared_ptr<Transaction> Database::Begin(bool readIsolation, bool writeIsolation)
{
	return boost::shared_ptr<Transaction>(new Transaction(*this, readIsolation, writeIsolation));
}

RangeSet Database::GetRange(const ColumnIDs& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId)
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
	return ResultsToRangeSet(result, range.ColumnID, std::find(columnIds.begin(), columnIds.end(), range.ColumnID) - columnIds.begin(), rangeResult);
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

DataSet Database::GetValues(const ColumnIDs& columnIds, const std::vector<std::string>& rowIds)
{
	Query rowIdQuery;
	rowIdQuery.__set_rowIDs(rowIds);

	// Make the query
	return InternalGetValues(columnIds, -1, rowIdQuery);
}

DataSet Database::ResultsToDataSet(const ColumnIDs& columnIds, const std::vector<std::string>& rowIDs, const ReadResults& rowResults)
{
	DataSet result (rowIDs.size(), columnIds.size());

	for (size_t y = 0; y < rowIDs.size(); y++)
	{
		result[y].ID = rowIDs[y];

		for (size_t x = 0; x < columnIds.size(); ++x)
		{
			auto columnId = columnIds[x];
			auto iter = rowResults.find(columnId);
			if (iter != rowResults.end())
			{
				result[y][x] = iter->second.answer.rowIDValues[y];
			}
		}
	}

	return result;
}

RangeSet Database::ResultsToRangeSet(DataSet& set, size_t, size_t rangeColumnIndex, const RangeResult& rangeResult)
{
	RangeSet result;

	result.Bof = rangeResult.bof;
	result.Eof = rangeResult.eof;
	result.Limited = rangeResult.limited;


	size_t valueRowValue = 0, valueRowRow = 0;
	for (size_t y = 0; y < set.size(); y++)
	{
		set[y].Values[rangeColumnIndex].__set_value(rangeResult.valueRowsList[valueRowValue].value);
		valueRowRow++;
		if (valueRowRow >= rangeResult.valueRowsList[valueRowValue].rowIDs.size())
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
	RangeRequest rangeRequest;
	rangeRequest.__set_ascending(range.Ascending);
	rangeRequest.__set_limit(limit);

	if (range.Start)
	{
		fastore::communication::RangeBound bound;
		bound.__set_inclusive(range.Start->Inclusive);
		bound.__set_value(range.Start->Bound);

		rangeRequest.__set_first(bound);
	}

	if (range.End)
	{
		fastore::communication::RangeBound bound;
		bound.__set_inclusive(range.End->Inclusive);
		bound.__set_value(range.End->Bound);

		rangeRequest.__set_last(bound);
	}

	if (startId)
		rangeRequest.__set_rowID(*startId);

	Query rangeQuery;
	rangeQuery.__set_ranges(std::vector<RangeRequest>());
	rangeQuery.ranges.push_back(rangeRequest);

	return rangeQuery;
}

void Database::Apply(const std::map<ColumnID, boost::shared_ptr<ColumnWrites>>& writes, const bool flush)
{
	if (writes.size() > 0)
		while (true)
		{
			TransactionID transactionID;
			transactionID.__set_key(0);
			transactionID.__set_revision(0);

			auto workers = DetermineWorkers(writes);

			auto tasks = StartWorkerWrites(writes, transactionID, workers);

			std::map<PodID, boost::shared_ptr<TProtocol>> failedWorkers;
			auto workersByTransaction = ProcessWriteResults(workers, tasks, failedWorkers);

			if (FinalizeTransaction(workers, workersByTransaction, failedWorkers))
			{
				// If we've inserted/deleted system table(s), force a schema refresh
				// Supposedly std::map sorts by std::less<T>, so in theory begin is
				// our smallest value. We should test this.
				if (writes.begin()->first <= Dictionary::MaxSystemColumnID)
				{
					RefreshSchema();
					RefreshHiveState();
				}

				if (flush)
					FlushWorkers(transactionID, workers);
				break;
			}
		}
}

std::vector<boost::shared_ptr<std::future<TransactionID>>> Database::StartWorkerWrites(const std::map<ColumnID, boost::shared_ptr<ColumnWrites>> &writes, const TransactionID &transactionID, const std::vector<WorkerInfo>& workers)
{
	std::vector<boost::shared_ptr<std::future<TransactionID>>> tasks;
	// Apply the modification to every worker, even if no work
	for (auto worker = workers.begin(); worker != workers.end(); ++worker)
	{
		// Determine the set of writes that apply to the worker's repositories
		std::map<ColumnID, ColumnWrites> work;
		for (auto columnId = worker->Columns.begin(); columnId != worker->Columns.end(); ++columnId)
		{
			auto iter = writes.find(*columnId);
			if (iter != writes.end())
			{
				work.insert(std::make_pair(*columnId, *(iter->second)));
			}
		}

		auto task = boost::shared_ptr<std::future<TransactionID>>
		(
			new std::future<TransactionID>
			(
				std::async
				(
					std::launch::async,
					[&, worker, work]()-> TransactionID
					{
						TransactionID result;
						AttemptWrite
						(
							worker->podID,
							[&](WorkerClient client)
							{
								client.apply(result, transactionID, work);
							}
						);

						return result;
					}
				)
			)
		);

		tasks.push_back(task);
	}
	return tasks;
}

void Database::FlushWorkers(const TransactionID& transactionID, const std::vector<WorkerInfo>& workers)
{
	std::vector<boost::shared_ptr<std::future<void>>> flushTasks;
	for (auto w = workers.begin(); w != workers.end(); ++w)
	{
		auto task = boost::shared_ptr<std::future<void>>
		(
			new std::future<void>
			(
				std::async
				(
					std::launch::async,
					[&]()
					{
						auto worker = _workers[w->podID];
						bool flushed = false;
						try
						{
							worker.flush(transactionID);
							flushed = true;
							_workers.Release(std::make_pair(w->podID, worker));
						}
						catch(const std::exception&)
						{
							//TODO: track exception errors;
							if(!flushed)
								_workers.Release(std::make_pair(w->podID, worker));
						}
					}
				)
			)
		);
		
		flushTasks.push_back(task);
	}

	// Wait for critical number of workers to flush
	auto neededCount =  workers.size() / 2 > 1 ? workers.size() / 2 : 1;
	for (size_t i = 0; i < flushTasks.size() && i < neededCount; i++)
		flushTasks[i]->wait();
}

std::map<TransactionID, std::vector<Database::WorkerInfo>> 
Database::ProcessWriteResults(const std::vector<WorkerInfo>& workers, 
			      const std::vector<boost::shared_ptr<std::future<TransactionID>>>& tasks, 
				std::map<PodID, boost::shared_ptr<TProtocol>>& failedWorkers)
{
	clock_t start = clock();

	std::map<TransactionID, std::vector<WorkerInfo>> workersByTransaction;
	for (size_t i = 0; i < tasks.size(); i++)
	{
		// Attempt to fetch the result for each task
		TransactionID resultId;
		try
		{
			//tasks[i]->wait();
			//resultId = tasks[i]->get();
			clock_t timeToWait = getWriteTimeout() - (clock() - start);
			auto result = tasks[i]->wait_for(std::chrono::milliseconds(timeToWait > 0 ? timeToWait : 0));
#if __GNUC_MINOR__ == 6
			// ignore what wait_for returns until GNU and Microsoft agree
			if (result)
#else
			if (result == std::future_status::ready)
#endif
				resultId = tasks[i]->get();
			else
			{
				failedWorkers.insert(std::pair<PodID, boost::shared_ptr<apache::thrift::protocol::TProtocol>>(workers[i].podID, boost::shared_ptr<apache::thrift::protocol::TProtocol>()));
				continue;
			}
		}
		catch (std::exception&)
		{
			failedWorkers.insert(std::make_pair(workers[i].podID, boost::shared_ptr<apache::thrift::protocol::TProtocol>()));
			///			failedWorkers.insert(std::pair<int, boost::shared_ptr<apache::thrift::protocol::TProtocol>>(i, boost::shared_ptr<apache::thrift::protocol::TProtocol>()));
			// else: Other errors were managed by AttemptWrite
			continue;
		}

		// If successful, group with other workers that returned the same revision
		auto iter = workersByTransaction.find(resultId);
		if (iter == workersByTransaction.end())
		{
			workersByTransaction.insert(std::make_pair(resultId, std::vector<WorkerInfo>()));
			iter = workersByTransaction.find(resultId);
		}
	
		iter->second.push_back(workers[i]);
	}

	return workersByTransaction;
}

bool Database::FinalizeTransaction(const std::vector<WorkerInfo>& workers, 
								   const std::map<TransactionID, std::vector<WorkerInfo>>& workersByTransaction, 
								   std::map<PodID, boost::shared_ptr<TProtocol>>& failedWorkers)
{
	if (workersByTransaction.size() > 0)
	{
		auto last = (--workersByTransaction.end());
		auto max = last->first;

		auto successes = last->second;

		if (successes.size() > workers.size() / 2)
		{
			// Transaction successful, commit all reached workers
			for (auto group = workersByTransaction.begin(); group != workersByTransaction.end(); ++group)
			{
				for (auto worker = group->second.begin(); worker != group->second.end(); ++worker)
				{
					WorkerInvoke(worker->podID, [&](WorkerClient client)
					{
						client.commit(max);
					}
					);
				}
			}
			// Also send out a commit to workers that timed-out
			for (auto worker = failedWorkers.begin(); worker != failedWorkers.end(); ++worker)
			{
				if (worker->second != nullptr)
				{
					WorkerInvoke(worker->first, [&](WorkerClient client)
					{
						client.commit(max);
					}
					);
				}
			}
			return true;
		}
		else
		{
			// Failure, roll-back successful prepares
			for (auto group = workersByTransaction.begin(); group != workersByTransaction.end(); ++group)
			{
				for (auto worker = group->second.begin(); worker != group->second.end(); ++worker)
				{
					WorkerInvoke(worker->podID, [&](WorkerClient client)
					{
						client.rollback(group->first);
					}
					);
				}
			}
		}
	}
	return false;
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

void Database::Include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row)
{
	auto writes = CreateIncludes(columnIds, rowId, row);
	Apply(writes, false);
}

std::map<ColumnID, boost::shared_ptr<ColumnWrites>> Database::CreateIncludes(const ColumnIDs& columnIds, const std::string& rowId, std::vector<std::string> row)
{
	std::map<ColumnID, boost::shared_ptr<ColumnWrites>> writes;

	for (size_t i = 0; i < columnIds.size(); ++i)
	{
		fastore::communication::Include inc;
		inc.__set_rowID(rowId);
		inc.__set_value(row[i]);

		boost::shared_ptr<ColumnWrites> wt(new ColumnWrites);
		wt->__set_includes(std::vector<fastore::communication::Include>());
		wt->includes.push_back(inc);
		writes.insert(std::make_pair(columnIds[i], wt));
	}
	return writes;
}

void Database::Exclude(const ColumnIDs& columnIds, const std::string& rowId)
{
	Apply(CreateExcludes(columnIds, rowId), false);
}

std::map<ColumnID, boost::shared_ptr<ColumnWrites>> Database::CreateExcludes(const ColumnIDs& columnIds, const std::string& rowId)
{
	std::map<ColumnID, boost::shared_ptr<ColumnWrites>> writes;

	for (size_t i = 0; i < columnIds.size(); ++i)
	{
		fastore::communication::Exclude ex;
		ex.__set_rowID(rowId);

		boost::shared_ptr<ColumnWrites> wt(new ColumnWrites);
		wt->__set_excludes(std::vector<fastore::communication::Exclude>());
		wt->excludes.push_back(ex);
		writes.insert(std::make_pair(columnIds[i], wt));
	}
	return writes;
}

std::vector<Statistic> Database::GetStatistics(const ColumnIDs& columnIds)
{
	// Make the request against each column
	//TODO: Instead of requesting each column individually, combine columns on a single worker.
	std::vector<boost::shared_ptr<std::future<Statistic>>> tasks;
	for (size_t i = 0; i < columnIds.size(); ++i)
	{
		auto columnId = columnIds[i];
		auto task = boost::shared_ptr<std::future<Statistic>>
		(
			new std::future<Statistic>
			(
				std::async
				(
					std::launch::async,
					[&]() -> Statistic
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

		tasks.push_back(task);
	}

	std::vector<Statistic> stats;
	for (auto iter = tasks.begin(); iter != tasks.end(); ++iter)
	{
		(*iter)->wait();
		stats.push_back((*iter)->get());
	}

	return stats; 
}

Schema Database::GetSchema()
{
	if (_schema.empty())
		_schema = LoadSchema();
	return _schema;
}

Schema Database::LoadSchema()
{
	Schema schema;
	bool finished = false;
	while (!finished)
	{
		Range range;
		range.Ascending = true;
		range.ColumnID = Dictionary::ColumnID;
		RangeSet columns = GetRange(Dictionary::ColumnColumns, range, MaxFetchLimit);

		for (auto column : columns.Data)
		{
			ColumnDef def;
			def.ColumnID = Encoder<ColumnID>::Decode(column.ID);
			def.Name = Encoder<std::string>::Decode(column.Values[1].value);
			def.Type = Encoder<std::string>::Decode(column.Values[2].value);
			def.IDType = Encoder<std::string>::Decode(column.Values[3].value);
			def.BufferType = Encoder<BufferType_t>::Decode(column.Values[4].value);
			def.Required = Encoder<bool>::Decode(column.Values[5].value);

			schema.insert(std::make_pair(def.ColumnID, def));
		}
		//TODO: this is wrong. We need to set the startId on the range above for this to resume properly
		finished = !columns.Limited;
	}
	return schema;
}

void Database::RefreshSchema()
{
	_schema = LoadSchema();
}

void Database::BootStrapSchema()
{
	static const std::map<fastore::communication::ColumnID,
						  ColumnDef> defaultSchema 
	{ { Dictionary::ColumnID, 
		  { Dictionary::ColumnID, 
			  "Column.ID", "Long", "Long", BufferType_t::Identity, true } }
	, { Dictionary::ColumnName, 
		  { Dictionary::ColumnName, 
			  "Column.Name", "String", "Long", BufferType_t::Unique, true } }
	, { Dictionary::ColumnValueType, 
		  { Dictionary::ColumnValueType, 
			"Column.ValueType", "String", "Long", BufferType_t::Multi, true } }
   	, { Dictionary::ColumnRowIDType, 
		  { Dictionary::ColumnRowIDType, 
			"Column.RowIDType", "String", "Long", BufferType_t::Multi, true } }
	, { Dictionary::ColumnBufferType, 
		  { Dictionary::ColumnBufferType, 
			"Column.BufferType", "Int", "Long", BufferType_t::Multi, true } }
	, { Dictionary::ColumnRequired, 
		  { Dictionary::ColumnRequired, 
			"Column.Required", "Bool", "Long", BufferType_t::Multi, true } }
	};	

	_schema.insert( defaultSchema.begin(), defaultSchema.end() );

#if 0
	//TODO: Consider making bootstrap information shared between server and client so that 
	//we don't have to change it two places.

	// Actually, we only need the ID and Type to bootstrap properly.
	ColumnDef id;
	id.ColumnID = Dictionary::ColumnID;
	id.Name = "Column.ID";
	id.Type = "Long";
	id.IDType = "Long";
	id.BufferType = BufferType_t::Identity;
	id.Required = true;
	_schema.insert(std::make_pair(Dictionary::ColumnID, id));

	ColumnDef name;
	name.ColumnID = Dictionary::ColumnName;
	name.Name = "Column.Name";
	name.Type = "String";
	name.IDType = "Long";
	name.BufferType = BufferType_t::Unique;
	name.Required = true;
	_schema.insert(std::make_pair(Dictionary::ColumnName, name));
	
	ColumnDef vt;
	vt.ColumnID = Dictionary::ColumnValueType;
	vt.Name = "Column.ValueType";
	vt.Type = "String";
	vt.IDType = "Long";
	vt.BufferType = BufferType_t::Multi;
	vt.Required = true;
	_schema.insert(std::make_pair(Dictionary::ColumnValueType, vt));

	ColumnDef idt;
	idt.ColumnID = Dictionary::ColumnRowIDType;
	idt.Name = "Column.RowIDType";
	idt.Type = "String";
	idt.IDType = "Long";
	idt.BufferType = BufferType_t::Multi;
	idt.Required = true;
	_schema.insert(std::make_pair(Dictionary::ColumnRowIDType, idt));

	ColumnDef unique;
	unique.ColumnID = Dictionary::ColumnBufferType;
	unique.Name = "Column.BufferType";
	unique.Type = "Int";
	unique.IDType = "Long";
	unique.BufferType = BufferType_t::Multi;
	unique.Required = true;
	_schema.insert(std::make_pair(Dictionary::ColumnBufferType, unique));

	ColumnDef required;
	required.ColumnID = Dictionary::ColumnRequired;
	required.Name = "Column.Required";
	required.Type = "Bool";
	required.IDType = "Long";
	required.BufferType = BufferType_t::Multi;
	required.Required = true;
	_schema.insert(std::make_pair(Dictionary::ColumnRequired, required));	
#endif
	//Boot strapping is done, pull in real schema
	RefreshSchema();
}

std::map<HostID, long long> Database::Ping()
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
