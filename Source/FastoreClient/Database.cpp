#include "Database.h"
#include "Dictionary.h"
#include <boost\format.hpp>
#include "Encoder.h"

using namespace fastore::client;

const int &Database::getWriteTimeout() const
{
	return _writeTimeout;
}

void Database::setWriteTimeout(const int &value)
{
	if (value < -1)
		throw std::exception("WriteTimeout must be -1 or greater.");
	_writeTimeout = value;
}

Database::Database(std::vector<ServiceAddress> addresses)
	:	
	_nextSystemWorker(0) , 
	_writeTimeout(DefaultWriteTimeout),
	_services
	(
		[](boost::shared_ptr<TProtocol> proto) { return ServiceClient(proto); }, 
		[&](int i) { return this->GetServiceAddress(i); }, 
		[](ServiceClient& client) { client.getInputProtocol()->getTransport()->close(); }, 
		[](ServiceClient& client) { return client.getInputProtocol()->getTransport()->isOpen();}
	),
	_workers
	(
		[](boost::shared_ptr<TProtocol> proto) { return WorkerClient(proto); }, 
		[&](int i) { return this->GetWorkerAddress(i); }, 
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
	for (int i = 0; i < networkAddresses.size(); i++)
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

	auto addressesByHost = std::map<int, NetworkAddress>();
	for (int hostID = 0; hostID  < networkAddresses.size(); hostID++)
		addressesByHost.insert(std::pair<int,NetworkAddress>(hostID, networkAddresses[hostID]));

	HiveState newHive;
	newHive.__set_topologyID(newTopology.topologyID);
	newHive.__set_services(std::map<int, ServiceState>());
	for (int hostID = 0; hostID < networkAddresses.size(); hostID++)
	{
		auto service = _services.Connect(networkAddresses[hostID]);
		try
		{
			ServiceState serviceState;
			service.init(serviceState, newTopology, addressesByHost, hostID);
			newHive.services.insert(std::pair<int, ServiceState>(hostID, serviceState));
		}
		catch(std::exception& e)
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
	auto writes = std::map<int, boost::shared_ptr<ColumnWrites>>();

	// Insert the topology
	std::vector<fastore::communication::Include> tempVector;

	fastore::communication::Include inc;
	inc.__set_rowID(Encoder<TopologyID>::Encode(newTopology.topologyID));
	inc.__set_value(Encoder<TopologyID>::Encode(newTopology.topologyID));

	tempVector.push_back(inc);

	boost::shared_ptr<ColumnWrites> topoWrites(new ColumnWrites());
	topoWrites->__set_includes(tempVector);
	writes.insert(std::pair<int, boost::shared_ptr<ColumnWrites>>(Dictionary::TopologyID, topoWrites));

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
	writes.insert(std::pair<int, boost::shared_ptr<ColumnWrites>>(Dictionary::HostID, hostWrites));
	writes.insert(std::pair<int, boost::shared_ptr<ColumnWrites>>(Dictionary::PodID, podWrites));
	writes.insert(std::pair<int, boost::shared_ptr<ColumnWrites>>(Dictionary::PodHostID, podHostWrites));

	Apply(writes, false);
}

Topology Database::CreateTopology(const std::vector<int>& serviceWorkers)
{
	Topology newTopology;
	//TODO: Generate ID. It was based on the hashcode of a guid.
	newTopology.__set_topologyID(0);
	newTopology.__set_hosts(std::map<HostID, Pods>());
	auto podID = 0;
	for (auto hostID = 0; hostID < serviceWorkers.size(); hostID++)
	{
		auto pods = std::map<int, std::vector<int>>();
		for (int i = 0; i < serviceWorkers[hostID]; i++)
		{
			pods.insert(std::pair<int, std::vector<int>>(podID, std::vector<int>())); // No no columns initially
			podID++;
		}
		newTopology.hosts.insert(std::pair<HostID, Pods>(hostID, pods));
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
NetworkAddress& Database::GetServiceAddress(int hostID)
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
	catch(std::exception& e)
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
	newHive.__set_services(std::map<int, ServiceState>());

	//Using shared_ptr because the copy constructor of future is private, preventing its direct use in a
	//vector.
	std::vector<boost::shared_ptr<std::future<std::pair<int, ServiceState>>>> tasks;
	for (auto service : _hiveState.services)
	{
		auto task = boost::shared_ptr<std::future<std::pair<int, ServiceState>>>
		(
			new std::future<std::pair<int, ServiceState>>
			(
				std::async
				(
					std::launch::async,
					[&]()-> std::pair<int, ServiceState>
					{
						ServiceClient serviceClient = _services[service.first];
						OptionalServiceState state;
						serviceClient.getState(state);
						if (!state.__isset.serviceState)						
							throw ClientException(boost::str(boost::format("Host ({0}) is unexpectedly not part of the topology.") % service.first));
						_services.Release(std::pair<int, ServiceClient>(service.first, serviceClient));
						return std::pair<int, ServiceState>(service.first, state.serviceState);
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
				_workerStates.insert(std::pair<PodID, std::pair<ServiceState, WorkerState>>(worker->podID, std::pair<ServiceState,WorkerState>(service->second, *worker)));
				//TODO: Don't assume all repos are online.
				for (auto repo = worker->repositoryStatus.begin(); repo != worker->repositoryStatus.end(); ++repo) //.Where(r => r.Value == RepositoryStatus.Online || r.Value == RepositoryStatus.Checkpointing)
				{
					auto currentMap = _columnWorkers.find(repo->first);
					if (currentMap == _columnWorkers.end())
					{
						_columnWorkers.insert(std::pair<PodID,PodMap>(repo->first, PodMap()));
						currentMap = _columnWorkers.find(repo->first);
					}

					currentMap->second.Pods.push_back(worker->podID);
				}
			}
		}
	}

	_lock->unlock();
}

NetworkAddress Database::GetWorkerAddress(int podID)
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

std::pair<int, WorkerClient> Database::GetWorker(int columnID)
{
	if (columnID <= Dictionary::MaxSystemColumnID)
		return GetWorkerForSystemColumn();
	else
		return GetWorkerForColumn(columnID);
}

std::pair<int, WorkerClient> Database::GetWorkerForColumn(int columnID)
{
	auto podId = GetWorkerIDForColumn(columnID);
	return std::pair<int, WorkerClient>(podId, _workers[podId]);
}

int Database::GetWorkerIDForColumn(int columnID)
{
	_lock->lock();
	
	auto iter = _columnWorkers.find(columnID);
	if (iter == _columnWorkers.end())
	{
		ClientException error(boost::str(boost::format("No worker is currently available for column ID ({0}).") % columnID), ClientException::Codes::NoWorkerForColumn);
		
		//TODO: Possibly add data to client exception
		// (or build an exception base class analogous to the C# one)
		//error->getData()->Add("ColumnID", columnID);
		_lock->unlock();
		throw error;
	}
	else
	{
		iter->second.Next = (iter->second.Next + 1) % iter->second.Pods.size();
		auto podId = iter->second.Pods[iter->second.Next];
		_lock->unlock();
		return podId;
	}	
}

std::pair<int, WorkerClient> Database::GetWorkerForSystemColumn()
{
	// For a system column, any worker will do, so just use an already connected worker
	int podID;
	_lock->lock();
	podID = std::next(_workerStates.begin(), _nextSystemWorker)->first;
	_nextSystemWorker = (_nextSystemWorker + 1) % _workerStates.size();
	_lock->unlock();

	return std::pair<int, WorkerClient>(podID, _workers[podID]);
}

std::vector<Database::WorkerInfo> Database::DetermineWorkers(const std::map<int, boost::shared_ptr<ColumnWrites>> &writes)
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
			info.PodID = ws->first;
			for (auto repo = ws->second.second.repositoryStatus.begin(); repo != ws->second.second.repositoryStatus.end(); ++repo)
			{
				if (repo->second == RepositoryStatus::Online || repo->second == RepositoryStatus::Checkpointing)
					info.Columns.push_back(repo->first);
			}

			//Union with system columns.
			info.Columns.insert(info.Columns.end(), systemColumns.begin(), systemColumns.end());			

			results.push_back(info);
		}

		// Release lock during ensures
		_lock->unlock();
		return results;
	}
	catch(std::exception& e)
	{
		_lock->unlock();
		throw e;
	}
}

void Database::AttemptRead(int columnId, std::function<void(WorkerClient)> work)
{
	std::map<int, std::exception> errors;
	clock_t begin;
	clock_t end;
	while (true)
	{
		// Determine the (next) worker to use
		auto worker = GetWorker(columnId);
		try
		{
			// If we've already failed with this worker, give up
			if (errors.size() > 0 && errors.find(worker.first) != errors.end())
			{
				TrackErrors(errors);
				//TODO: aggregate exception for c++
				throw errors.begin()->second;
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

				errors.insert(std::pair<int, std::exception>(worker.first, e));
				continue;
			}

			// Succeeded, track any errors we received
			if (errors.size() > 0)
				TrackErrors(errors);

			// Succeeded, track the elapsed time
			TrackTime(worker.first, end - begin);

			break;
		}
		catch(std::exception& e)
		{
			_workers.Release(worker);
			throw e;
		}

		_workers.Release(worker);
	}
}

void Database::AttemptWrite(int podId, std::function<void(WorkerClient)> work)
{
	clock_t begin;
	clock_t end;
	try
	{
		begin = clock();
		WorkerInvoke(podId, work);
		end = clock();
	}
	catch (std::exception &e)
	{
		// If the exception is an entity (exception coming from the remote), rethrow
		//TODO: Figure what this will be since TBase doesn't exist in c++ (only c#)
		//if (!(dynamic_cast<Thrift::Protocol::TBase*>(e) != nullptr))
			//TrackErrors(std::map<int, std::exception> {{podId, e}});

		throw;
	}

	// Succeeded, track the elapsed time
	TrackTime(podId, end - begin);
}

void Database::TrackTime(int podId, long long p)
{
	// TODO: track the time taken by the worker for better routing
}

void Database::TrackErrors(std::map<int, std::exception> &errors)
{
	// TODO: stop trying to reach workers that keep giving errors, ask for a state update too
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
			queries.insert(std::pair<ColumnID, Query>(col, query));
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

DataSet Database::InternalGetValues(const ColumnIDs& columnIds, const int exclusionColumnId, const Query& rowIdQuery)
{
	std::vector<boost::shared_ptr<std::future<ReadResults>>> tasks;
	for (int i = 0; i < columnIds.size(); ++i)
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
							queries.insert(std::pair<ColumnID, Query>(columnId, rowIdQuery));
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

DataSet Database::GetValues(const std::vector<int>& columnIds, const std::vector<std::string>& rowIds)
{
	Query rowIdQuery;
	rowIdQuery.__set_rowIDs(rowIds);

	// Make the query
	return InternalGetValues(columnIds, -1, rowIdQuery);
}

DataSet Database::ResultsToDataSet(const ColumnIDs& columnIds, const std::vector<std::string>& rowIDs, const std::map<int, ReadResult>& rowResults)
{
	DataSet result (rowIDs.size(), columnIds.size());

	for (int y = 0; y < rowIDs.size(); y++)
	{
		result[y].ID = rowIDs[y];

		for (int x = 0; x < columnIds.size(); ++x)
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

RangeSet Database::ResultsToRangeSet(DataSet& set, const int rangeColumnId, const int rangeColumnIndex, const RangeResult& rangeResult)
{
	RangeSet result;

	result.Bof = rangeResult.bof;
	result.Eof = rangeResult.eof;
	result.Limited = rangeResult.limited;


	int valueRowValue = 0;
	int valueRowRow = 0;
	for (int y = 0; y < set.size(); y++)
	{
		set[y].Values[rangeColumnIndex] = rangeResult.valueRowsList[valueRowValue].value;
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

void Database::Apply(const std::map<int, boost::shared_ptr<ColumnWrites>>& writes, const bool flush)
{
	if (writes.size() > 0)
		while (true)
		{
			TransactionID transactionID;
			transactionID.__set_key(0);
			transactionID.__set_revision(0);

			auto workers = DetermineWorkers(writes);

			auto tasks = StartWorkerWrites(writes, transactionID, workers);

			std::map<int, boost::shared_ptr<TProtocol>> failedWorkers;
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

std::vector<boost::shared_ptr<std::future<TransactionID>>> Database::StartWorkerWrites(const std::map<int, boost::shared_ptr<ColumnWrites>> &writes, const TransactionID &transactionID, const std::vector<WorkerInfo>& workers)
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
				work.insert(std::pair<ColumnID, ColumnWrites>(*columnId, *(iter->second)));
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
							worker->PodID,
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
						auto worker = _workers[w->PodID];
						bool flushed = false;
						try
						{
							worker.flush(transactionID);
							flushed = true;
							_workers.Release(std::pair<PodID, WorkerClient>(w->PodID, worker));
						}
						catch(std::exception& e)
						{
							//TODO: track exception errors;
							if(!flushed)
								_workers.Release(std::pair<PodID, WorkerClient>(w->PodID, worker));
						}
					}
				)
			)
		);
		
		flushTasks.push_back(task);
	}

	// Wait for critical number of workers to flush
	auto neededCount =  workers.size() / 2 > 1 ? workers.size() / 2 : 1;
	for (int i = 0; i < flushTasks.size() && i < neededCount; i++)
		flushTasks[i]->wait();
}

std::map<TransactionID, std::vector<Database::WorkerInfo>> Database::ProcessWriteResults(const std::vector<WorkerInfo>& workers, const std::vector<boost::shared_ptr<std::future<TransactionID>>>& tasks, std::map<int, boost::shared_ptr<TProtocol>>& failedWorkers)
{
	clock_t start = clock();

	std::map<TransactionID, std::vector<WorkerInfo>> workersByTransaction;
	for (int i = 0; i < tasks.size(); i++)
	{
		// Attempt to fetch the result for each task
		TransactionID resultId;
		try
		{
			tasks[i]->wait();
			resultId = tasks[i]->get();
			/*clock_t timeToWait = getWriteTimeout() - (clock() - start);
			auto result = tasks[i]->wait_for(std::chrono::milliseconds(timeToWait > 0 ? timeToWait : 0));
			if (result == std::future_status::ready)
				resultId = tasks[i]->get();
			else
			{
				failedWorkers.insert(std::pair<int, boost::shared_ptr<apache::thrift::protocol::TProtocol>>(i, boost::shared_ptr<apache::thrift::protocol::TProtocol>()));
				continue;
			}*/
		}
		catch (std::exception &e)
		{
			failedWorkers.insert(std::pair<int, boost::shared_ptr<apache::thrift::protocol::TProtocol>>(i, boost::shared_ptr<apache::thrift::protocol::TProtocol>()));
			// else: Other errors were managed by AttemptWrite
			continue;
		}

		// If successful, group with other workers that returned the same revision
		auto iter = workersByTransaction.find(resultId);
		if (iter == workersByTransaction.end())
		{
			workersByTransaction.insert(std::pair<TransactionID, std::vector<WorkerInfo>>(resultId, std::vector<WorkerInfo>()));
			iter = workersByTransaction.find(resultId);
		}
	
		iter->second.push_back(workers[i]);
	}

	return workersByTransaction;
}

bool Database::FinalizeTransaction(const std::vector<WorkerInfo>& workers, const std::map<TransactionID, std::vector<WorkerInfo>>& workersByTransaction, std::map<int, boost::shared_ptr<TProtocol>>& failedWorkers)
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
					WorkerInvoke(worker->PodID, [&](WorkerClient client)
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
			// Transaction successful, commit all reached workers
			for (auto group = workersByTransaction.begin(); group != workersByTransaction.end(); ++group)
			{
				for (auto worker = group->second.begin(); worker != group->second.end(); ++worker)
				{
					WorkerInvoke(worker->PodID, [&](WorkerClient client)
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

void Database::WorkerInvoke(int podID, std::function<void(WorkerClient)> work)
{
	auto client = _workers[podID];
	bool released = false;
	try
	{
		work(client);
		released = true;
		_workers.Release(std::pair<PodID, WorkerClient>(podID, client));
	}
	catch(std::exception& e)
	{
		if (!released)
			_workers.Release(std::pair<PodID, WorkerClient>(podID, client));
		throw e;
	}
}

void Database::Include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row)
{
	auto writes = CreateIncludes(columnIds, rowId, row);
	Apply(writes, false);
}

std::map<int, boost::shared_ptr<ColumnWrites>> Database::CreateIncludes(const ColumnIDs& columnIds, const std::string& rowId, std::vector<std::string> row)
{
	std::map<int, boost::shared_ptr<ColumnWrites>> writes;

	for (int i = 0; i < columnIds.size(); ++i)
	{
		fastore::communication::Include inc;
		inc.__set_rowID(rowId);
		inc.__set_value(row[i]);

		boost::shared_ptr<ColumnWrites> wt(new ColumnWrites);
		wt->__set_includes(std::vector<fastore::communication::Include>());
		wt->includes.push_back(inc);
		writes.insert(std::pair<int, boost::shared_ptr<ColumnWrites>>(columnIds[i], wt));
	}
	return writes;
}

void Database::Exclude(const ColumnIDs& columnIds, const std::string& rowId)
{
	Apply(CreateExcludes(columnIds, rowId), false);
}

std::map<int, boost::shared_ptr<ColumnWrites>> Database::CreateExcludes(const ColumnIDs& columnIds, const std::string& rowId)
{
	std::map<int, boost::shared_ptr<ColumnWrites>> writes;

	for (int i = 0; i < columnIds.size(); ++i)
	{
		fastore::communication::Exclude ex;
		ex.__set_rowID(rowId);

		boost::shared_ptr<ColumnWrites> wt(new ColumnWrites);
		wt->__set_excludes(std::vector<fastore::communication::Exclude>());
		wt->excludes.push_back(ex);
		writes.insert(std::pair<int, boost::shared_ptr<ColumnWrites>>(columnIds[i], wt));
	}
	return writes;
}

std::vector<Statistic> Database::GetStatistics(const std::vector<int>& columnIds)
{
	// Make the request against each column
	std::vector<boost::shared_ptr<std::future<Statistic>>> tasks;
	for (int i = 0; i < columnIds.size(); ++i)
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
							[&](WorkerClient worker)
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
			def.Name = Encoder<std::string>::Decode(column.Values[1]);
			def.Type = Encoder<std::string>::Decode(column.Values[2]);
			def.IDType = Encoder<std::string>::Decode(column.Values[3]);
			def.BufferType = Encoder<BufferType>::Decode(column.Values[4]);
			def.Required = Encoder<bool>::Decode(column.Values[5]);

			schema.insert(std::pair<int, ColumnDef>(def.ColumnID, def));
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
	// Actually, we only need the ID and Type to bootstrap properly.
	ColumnDef id;
	id.ColumnID = Dictionary::ColumnID;
	id.Name = "Column.ID";
	id.Type = "Int";
	id.IDType = "Int";
	id.BufferType = BufferType::Identity;
	id.Required = true;
	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnID, id));

	ColumnDef name;
	name.ColumnID = Dictionary::ColumnName;
	name.Name = "Column.Name";
	name.Type = "String";
	name.IDType = "Int";
	name.BufferType = BufferType::Unique;
	name.Required = true;
	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnName, name));
	
	ColumnDef vt;
	vt.ColumnID = Dictionary::ColumnValueType;
	vt.Name = "Column.ValueType";
	vt.Type = "String";
	vt.IDType = "Int";
	vt.BufferType = BufferType::Multi;
	vt.Required = true;
	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnValueType, vt));

	ColumnDef idt;
	idt.ColumnID = Dictionary::ColumnRowIDType;
	idt.Name = "Column.RowIDType";
	idt.Type = "String";
	idt.IDType = "Int";
	idt.BufferType = BufferType::Multi;
	idt.Required = true;
	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnRowIDType, idt));

	ColumnDef unique;
	unique.ColumnID = Dictionary::ColumnBufferType;
	unique.Name = "Column.BufferType";
	unique.Type = "Int";
	unique.IDType = "Int";
	unique.BufferType = BufferType::Multi;
	unique.Required = true;
	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnBufferType, unique));

	ColumnDef required;
	required.ColumnID = Dictionary::ColumnRequired;
	required.Name = "Column.Required";
	required.Type = "Bool";
	required.IDType = "Int";
	required.BufferType = BufferType::Multi;
	required.Required = true;
	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnRequired, required));	

	//Boot strapping is done, pull in real schema
	RefreshSchema();
}

std::map<int, long long> Database::Ping()
{
	std::vector<int> hostIds;

	_lock->lock();
	for (auto iter = _hiveState.services.begin(); iter != _hiveState.services.end(); ++iter)
	{
		hostIds.push_back(iter->first);
	}
	_lock->unlock();

	std::vector<boost::shared_ptr<std::future<std::pair<int, long long>>>> tasks;
	for (int i = 0; i < hostIds.size(); ++i)
	{		
		int hostId = hostIds[i];
		auto task = boost::shared_ptr<std::future<std::pair<int, long long>>>
		(
			new std::future<std::pair<int, long long>>
			(
				std::async
				(
					std::launch::async,
					[&]() -> std::pair<int, long long>
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
							_services.Release(std::pair<int, ServiceClient>(hostId, service));							
						}
						catch(std::exception& e)
						{
							if (!released)
								_services.Release(std::pair<int, ServiceClient>(hostId, service));
						}				

						return std::pair<int, long long>(hostId, stop - start);
					}
				)
			)
		);
	}

	// Gather ping results
	std::map<int, long long> result;
	for (auto iter = tasks.begin(); iter != tasks.end(); ++iter)
	{
		(*iter)->wait();
		result.insert((*iter)->get());
	}

	return result;
}
