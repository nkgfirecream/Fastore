#include "Database.h"
#include "Dictionary.h"
#include <boost\format.hpp>

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
		[&](int i) { return GetServiceAddress(i); }, 
		[](ServiceClient& client) { client.getInputProtocol()->getTransport()->close(); }, 
		[](ServiceClient& client) { return client.getInputProtocol()->getTransport()->isOpen();}
	),
	_workers
	(
		[](boost::shared_ptr<TProtocol> proto) { return WorkerClient(proto); }, 
		[&](int i) { return GetWorkerAddress(i); }, 
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
		n.__set_name(a.getName());
		n.__set_port(a.getPort());
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
	auto writes = std::map<int, ColumnWrites>();

	// Insert the topology
	std::vector<fastore::communication::Include> tempVector;

	fastore::communication::Include inc;
	//TODO: Encoding
	//inc.__set_rowID(newTopology.topologyID);
	//inc.__set_value(newTopology.topologyID);

	tempVector.push_back(inc);

	ColumnWrites topoWrites;
	topoWrites.__set_includes(tempVector);
	writes.insert(std::pair<int, ColumnWrites>(Dictionary::TopologyID, topoWrites));

	// Insert the hosts and pods
	ColumnWrites hostWrites;
	hostWrites.__set_includes(std::vector<fastore::communication::Include>());

	ColumnWrites podWrites;
	podWrites.__set_includes(std::vector<fastore::communication::Include>());

	ColumnWrites podHostWrites;
	podHostWrites.__set_includes(std::vector<fastore::communication::Include>());

	for (auto h = newTopology.hosts.begin(); h != newTopology.hosts.end(); ++h)
	{
		//TODO: Encoding
		fastore::communication::Include inc;		
		//inc.__set_rowID(h->first);
		//inc.__set_rowID(h->first);
		hostWrites.includes.push_back(inc);

		for (auto p = h->second.begin(); p != h->second.end(); ++p)
		{
			fastore::communication::Include pwinc;
			//pwinc.__set_rowID(p->first);
			//pwinc.__set_value(p->first);
			podWrites.includes.push_back(pwinc);

			fastore::communication::Include phinc;
			//phinc.__set_rowID(p->first);
			//phinc.__set_value(h->first);
			podHostWrites.includes.push_back(phinc);
		}
	}
	writes.insert(std::pair<int, ColumnWrites>(Dictionary::HostID, hostWrites));
	writes.insert(std::pair<int, ColumnWrites>(Dictionary::PodID, podWrites));
	writes.insert(std::pair<int, ColumnWrites>(Dictionary::PodHostID, podHostWrites));

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
	for (auto service = _hiveState.services.begin(); service != _hiveState.services.end(); ++service)
	{
		auto task = boost::shared_ptr<std::future<std::pair<int, ServiceState>>>
		(
			new std::future<std::pair<int, ServiceState>>
			(
				std::async
				(
					std::launch::any,
					[&]()-> std::pair<int, ServiceState>
					{
						ServiceClient serviceClient = _services[service->first];
						OptionalServiceState state;
						serviceClient.getState(state);
						if (!state.__isset.serviceState)						
							throw ClientException(boost::str(boost::format("Host ({0}) is unexpectedly not part of the topology.") % service->first));
						_services.Release(std::pair<int, ServiceClient>(service->first, serviceClient));
						return std::pair<int, ServiceState>(service->first, state.serviceState);
					}
				)
			)
		);

		tasks.push_back(task);
	}

	for (auto task = tasks.begin(); task != tasks.end(); ++task)
	{
		(*task)->wait();
		auto statePair = (*task)->get();
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
						_columnWorkers.insert(currentMap, std::pair<PodID,PodMap>(repo->first, PodMap()));
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
		// (ir built build an exception base class analogous to the C# one)
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

std::vector<Database::WorkerInfo> Database::DetermineWorkers(const std::map<int, ColumnWrites> &writes)
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

			//Union
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

//Transaction Database::Begin(bool readIsolation, bool writeIsolation)
//{
//	return Transaction(this, readIsolation, writeIsolation);
//}

RangeSet Database::GetRange(std::vector<int>& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId)
{
	// Create the range query
	auto query = CreateQuery(range, limit, startId);

	// Make the range request
	std::map<int, ReadResult> rangeResults;
	AttemptRead
	(
		range.ColumnID, 
		[&](WorkerClient client)
		{
			ReadResults results;
			Queries queries;
			queries.insert(std::pair<ColumnID, Query>(range.ColumnID, query));
			client.query(results, queries);
		}
	);

	auto rangeResult = rangeResults[range.ColumnID].answer.rangeValues.at(0);

	// Create the row ID query
	Query rowIdQuery = GetRowsQuery(rangeResult);

	// Get the values for all but the range
	auto result = InternalGetValues(columnIds, range.ColumnID, rowIdQuery);

	// Add the range values into the result
	return ResultsToRangeSet(result, range.ColumnID, std::find(columnIds.begin(), columnIds.end(), range.ColumnID) - columnIds.begin(), rangeResult);
}

DataSet Database::InternalGetValues(const std::vector<int>& columnIds, const int exclusionColumnId, const Query& rowIdQuery)
{
	std::vector<boost::shared_ptr<std::future<std::map<int, ReadResult>>>> tasks;
	for (int i = 0; i < columnIds.size(); ++i)
	{
		auto columnId = columnIds[i];
		if (columnId != exclusionColumnId)
		{
			auto task = boost::shared_ptr<std::future<std::map<int, ReadResult>>>
			(
				new std::future<std::map<int, ReadResult>>
				(
					std::async
					(
						std::launch::any,
						[&]()-> std::map<int, ReadResult>
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
	std::map<int, ReadResult> resultsByColumn;
	for (auto task = tasks.begin(); task != tasks.end(); ++task)
	{
		(*task)->wait();
		auto taskresult = (*task)->get();
		for (auto result = taskresult.begin(); result != taskresult.end(); ++result)
		{
			//TODO: Is this necessary? Aren't the pairs already by columnID?
			resultsByColumn.insert(std::pair<int,ReadResult>(result->first, result->second));
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

//std::vector<unsigned char[]> Database::EncodeRowIds(object rowIds[])
//{
//	auto encodedRowIds = std::vector<unsigned char[]>(sizeof(rowIds) / sizeof(rowIds[0]));
//	for (int i = 0; i < sizeof(rowIds) / sizeof(rowIds[0]); i++)
//		encodedRowIds->Add(Encoder::Encode(rowIds[i]));
//	return encodedRowIds;
//}
//

DataSet Database::ResultsToDataSet(const std::vector<int>& columnIds, const std::vector<std::string>& rowIDs, const std::map<int, ReadResult>& rowResults)
{
	DataSet result (rowIDs.size(), columnIds.size());

	for (int y = 0; y < rowIDs.size(); y++)
	{
		DataSet::DataSetRow row;
		std::string rowId = rowIDs[y];

		row.ID = rowId;
		row.Values = std::vector<std::string>(columnIds.size());

		for (int x = 0; x < columnIds.size(); ++x)
		{
			auto columnId = columnIds[x];
			auto iter = rowResults.find(columnId);
			if (iter != rowResults.end())
			{
				row.Values[x] = iter->second.answer.rowIDValues[y];
			}
		}

		result[y] = row;
	}

	return result;
}

RangeSet Database::ResultsToRangeSet(DataSet& set, const int rangeColumnId, const int rangeColumnIndex, const RangeResult& rangeResult)
{
	RangeSet result;

	result.setBof(rangeResult.bof);
	result.setEof(rangeResult.eof);
	result.setLimited(rangeResult.limited);
	result.setData(set);

	int valueRowValue = 0;
	int valueRowRow = 0;
	for (int y = 0; y < set.getCount(); y++)
	{
		set[y].Values[rangeColumnIndex] = rangeResult.valueRowsList[valueRowValue].value;
		valueRowRow++;
		if (valueRowRow >= rangeResult.valueRowsList[valueRowValue].rowIDs.size())
		{
			valueRowValue++;
			valueRowRow = 0;
		}
	}

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

void Database::Apply(const std::map<int, ColumnWrites>& writes, const bool flush)
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

std::vector<boost::shared_ptr<std::future<TransactionID>>> Database::StartWorkerWrites(const std::map<int, ColumnWrites> &writes, const TransactionID &transactionID, const std::vector<WorkerInfo>& workers)
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
				work.insert(std::pair<ColumnID, ColumnWrites>(*columnId, iter->second));
			}
		}

		auto task = boost::shared_ptr<std::future<TransactionID>>
		(
			new std::future<TransactionID>
			(
				std::async
				(
					std::launch::any,
					[&]()-> TransactionID
					{
						TransactionID result;
						AttemptWrite
						(
							worker->PodID,
							[&](WorkerClient client)
							{
								client.apply(result, transactionID, writes);
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
					std::launch::any,
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

//void Database::Include(int columnIds[], const boost::shared_ptr<object> &rowId, object row[])
//{
//	auto writes = EncodeIncludes(columnIds, rowId, row);
//	Apply(writes, false);
//}

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
			//// if the task doesn't complete in time, assume failure; move on to the next one...
			//if (!tasks[i].Wait(Math.Max(0, WriteTimeout - (int)stopWatch.ElapsedMilliseconds)))
			//{
			//	failedWorkers.Add(i, null);
			//	continue;
			//}
			resultId = tasks[i]->get();
		}
		catch (std::exception &e)
		{
			//if (dynamic_cast<Thrift::Protocol::TBase*>(e) != nullptr)
				//failedWorkers.insert(make_pair(i, dynamic_cast<Thrift::Protocol::TBase*>(e)));
			// else: Other errors were managed by AttemptWrite
			continue;
		}

		// If successful, group with other workers that returned the same revision
		auto iter = workersByTransaction.find(resultId);
		if (iter == workersByTransaction.end())
		{
			workersByTransaction.insert(iter, std::pair<TransactionID, std::vector<WorkerInfo>>(resultId, std::vector<WorkerInfo>()));
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
//

//
//std::map<int, ColumnWrites*> Database::EncodeIncludes(int columnIds[], const boost::shared_ptr<object> &rowId, object row[])
//{
//	auto writes = std::map<int, ColumnWrites*>();
////ORIGINAL LINE: byte[] rowIdb = Fastore.Client.Encoder.Encode(rowId);
////C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
//	unsigned char *rowIdb = Fastore::Client::Encoder::Encode(rowId);
//
//	for (int i = 0; i < sizeof(columnIds) / sizeof(columnIds[0]); i++)
//	{
//		boost::shared_ptr<Alphora::Fastore::Include> inc = boost::make_shared<Fastore::Include>();
//		inc->RowID = rowIdb;
//		inc->Value = Fastore::Client::Encoder::Encode(row[i]);
//
//		boost::shared_ptr<ColumnWrites> wt = boost::make_shared<ColumnWrites>();
//		wt->Includes = std::vector<Fastore::Include*>();
//		wt->Includes->Add(inc);
//		writes->Add(columnIds[i], wt);
//	}
//	return writes;
//}
//
//void Database::Exclude(int columnIds[], const boost::shared_ptr<object> &rowId)
//{
//	Apply(EncodeExcludes(columnIds, rowId), false);
//}
//
//std::map<int, ColumnWrites*> Database::EncodeExcludes(int columnIds[], const boost::shared_ptr<object> &rowId)
//{
//	auto writes = std::map<int, ColumnWrites*>();
////ORIGINAL LINE: byte[] rowIdb = Fastore.Client.Encoder.Encode(rowId);
////C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
//	unsigned char *rowIdb = Fastore::Client::Encoder::Encode(rowId);
//
//	for (int i = 0; i < sizeof(columnIds) / sizeof(columnIds[0]); i++)
//	{
//		boost::shared_ptr<Alphora::Fastore::Exclude> inc = boost::make_shared<Fastore::Exclude>();
//		inc->RowID = rowIdb;
//
//		boost::shared_ptr<ColumnWrites> wt = boost::make_shared<ColumnWrites>();
//		wt->Excludes = std::vector<Fastore::Exclude*>();
//		wt->Excludes->Add(inc);
//		writes->Add(columnIds[i], wt);
//	}
//	return writes;
//}
//
//Statistic *Database::GetStatistics(int columnIds[])
//{
//	// Make the request against each column
//	auto tasks = std::vector<Task<Fastore::Statistic*>*>(sizeof(columnIds) / sizeof(columnIds[0]));
//	for (var i = 0; i < sizeof(columnIds) / sizeof(columnIds[0]); i++)
//	{
//		auto columnId = columnIds[i];
////C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
//		tasks->Add(Task::Factory::StartNew(() =>
//		{
//			boost::shared_ptr<Fastore::Statistic> result = nullptr;
////C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
//			AttemptRead(columnId, (worker) =>
//			{
//				const int tempVector2[] = {columnId};
//				result = worker::getStatistics(std::vector<int>(tempVector2, tempVector2 + sizeof(tempVector2) / sizeof(tempVector2[0])))[0];
//			}
//			);
//			return result;
//		}
//		));
//	}
//
//	return (from t in tasks let r = t::Result select Statistic {Total = r::Total, Unique = r::Unique})->ToArray();
//}
//
//boost::shared_ptr<Schema> Database::GetSchema()
//{
//	if (_schema->empty())
//		_schema = LoadSchema();
//	return boost::make_shared<Schema>(_schema);
//}
//
//boost::shared_ptr<Schema> Database::LoadSchema()
//{
//	auto schema = boost::make_shared<Schema>();
//	auto finished = false;
//	while (!finished)
//	{
//		auto columns = GetRange(std::map::ColumnColumns, Range {ColumnID = std::map::ColumnID, Ascending = true}, MaxFetchLimit);
//		for (Alphora::Fastore::Client::DataSet::const_iterator column = columns->getData()->begin(); column != columns->getData()->end(); ++column)
//		{
//			auto def = ColumnDef {ColumnID = static_cast<int>((*column)->Values[0]), Name = static_cast<std::string>((*column)->Values[1]), Type = static_cast<std::string>((*column)->Values[2]), IDType = static_cast<std::string>((*column)->Values[3]), BufferType = static_cast<BufferType>((*column)->Values[4])};
//			schema->insert(make_pair(def.getColumnID(), def));
//		}
//		finished = !columns->getLimited();
//	}
//	return schema;
//}

void Database::RefreshSchema()
{
	_schema = LoadSchema();
}

void Database::BootStrapSchema()
{
	// Actually, we only need the ID and Type to bootstrap properly.
	ColumnDef id;
	ColumnDef name;
	ColumnDef vt;
	ColumnDef idt;
	ColumnDef unique;

	id.setColumnID(Dictionary::ColumnID);
	id.setName("Column.ID");
	id.setType("Int");
	id.setIDType("Int");
	id.setBufferType(BufferType::Identity);

	name.setColumnID(Dictionary::ColumnName);
	name.setName("Column.Name");
	name.setType("String");
	name.setIDType("Int");
	name.setBufferType(BufferType::Unique);

	vt.setColumnID(Dictionary::ColumnValueType);
	vt.setName("Column.ValueType");
	vt.setType("String");
	vt.setIDType("Int");
	vt.setBufferType(BufferType::Multi);

	idt.setColumnID(Dictionary::ColumnRowIDType);
	idt.setName("Column.RowIDType");
	idt.setType("String");
	idt.setIDType("Int");
	idt.setBufferType(BufferType::Multi);

	unique.setColumnID(Dictionary::ColumnBufferType);
	unique.setName("Column.BufferType");
	unique.setType("Int");
	unique.setIDType("Int");
	unique.setBufferType(BufferType::Multi);

	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnID, id));
	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnName, name));
	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnValueType, vt));
	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnRowIDType, idt));
	_schema.insert(std::pair<ColumnID, ColumnDef>(Dictionary::ColumnBufferType, unique));

	//Boot strapping is done, pull in real schema
	RefreshSchema();
}
//
//std::map<int, TimeSpan> Database::Ping()
//{
//	// Setup the set of hosts
////ORIGINAL LINE: int[] hostIDs;
////C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
//	int *hostIDs;
////C# TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
//	lock (_mapLock)
//	{
//		hostIDs = _hiveState->Services->Keys->ToArray();
//	}
//
//	// Start a ping request to each
////C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
//	auto tasks = from id in hostIDs select Task::Factory::StartNew<KeyValuePair<int, TimeSpan>*> (() =>
//	{
//		auto service = _services[id];
//		try
//		{
//			auto timer = boost::make_shared<Stopwatch>();
//			timer->Start();
//			service->ping();
//			timer->Stop();
//			return KeyValuePair<int, TimeSpan>(id, timer->Elapsed);
//		}
////C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
//		finally
//		{
//			_services->Release(KeyValuePair<int, Service::Client*>(id, service));
//		}
//	}
//	);
//
//	// Gather ping results
//	auto result = std::map<int, TimeSpan>(sizeof(hostIDs) / sizeof(hostIDs[0]));
//	for (System::Collections::Generic::IEnumerable::const_iterator task = tasks->begin(); task != tasks->end(); ++task)
//	{
//		try
//		{
//			auto item = (*task)->Result;
//			result->Add(item::Key, item->Value);
//		}
//		catch (...)
//		{
//			// TODO: report errors
//		}
//	}
//
//	return result;
//}
