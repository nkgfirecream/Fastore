#include "Database.h"

//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Collections::Generic;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Linq;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Text;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Threading::Tasks;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Diagnostics;
//C# TO C++ CONVERTER TODO TASK: The .NET System namespace is not available from native C++:
//using namespace System::Threading;
namespace Alphora
{
	namespace Fastore
	{
		namespace Client
		{

			Database::PodMap::PodMap()
			{
				Pods = std::vector<int>();
			}

			const int &Database::getWriteTimeout() const
			{
				return _writeTimeout;
			}

			void Database::setWriteTimeout(const int &value)
			{
				if (value < -1)
					throw boost::make_shared<ArgumentOutOfRangeException>("value", "WriteTimeout must be -1 or greater.");
				_writeTimeout = value;
			}

			Database::Database(ServiceAddress addresses[])
			{
				InitializeInstanceFields();
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
				_services = boost::make_shared<ConnectionPool<int, Service::Client*>> ((proto) => boost::make_shared<Service::Client>(proto), boost::make_shared<Func<int, NetworkAddress*>>(GetServiceAddress), (client) =>(new object[] {client::InputProtocol->Transport->Close();}), (client) => client::InputProtocol->Transport->IsOpen);

//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
				_workers = boost::make_shared<ConnectionPool<int, Worker::Client*>> ((proto) => boost::make_shared<Worker::Client>(proto), boost::make_shared<Func<int, NetworkAddress*>>(GetWorkerAddress), (client) =>(new object[] {client::InputProtocol->Transport->Close();}), (client) => client::InputProtocol->Transport->IsOpen);

				// Convert from service addresses to network addresses
				auto networkAddresses = (from a in addresses select boost::make_shared<NetworkAddress> {Name = a->Name, Port = a::Port})->ToArray();

				// Number of potential workers for each service (in case we nee to create the hive)
				auto serviceWorkers = new int[sizeof(networkAddresses) / sizeof(networkAddresses[0])];

				for (int i = 0; i < sizeof(networkAddresses) / sizeof(networkAddresses[0]); i++)
				{
					auto service = _services->Connect(networkAddresses[i]);
					try
					{
						// Discover the state of the entire hive from the given service
						auto hiveStateResult = service->getHiveState(false);
						if (!hiveStateResult::__isset.hiveState)
						{
							// If no hive state is given, the service is not joined, we should proceed to discover the number 
							//  of potential workers for the rest of the services ensuring that they are all not joined.
							serviceWorkers[i] = hiveStateResult::PotentialWorkers;
							_services->Destroy(service);
							continue;
						}

						// If we have passed the first host, we are in "discovery" mode for a new topology so we find any services that are joined.
						if (i > 0)
							throw boost::make_shared<ClientException>(std::string::Format("Service '{0}' is joined to topology {1}, while at least one other specified service is not part of any topology.", networkAddresses[i]->Name, hiveStateResult::HiveState::TopologyID));

						UpdateHiveState(hiveStateResult::HiveState);
						BootStrapSchema();
						RefreshHiveState();

						//Everything worked... exit function
						return;
					}
					catch (...)
					{
						// If anything goes wrong, be sure to release the service client
						_services->Destroy(service);
						delete _services;
						throw;
					}
				}

				//Create a new topology instead.
				auto newTopology = CreateTopology(serviceWorkers);

				auto addressesByHost = std::map<int, NetworkAddress*>();
				for (var hostID = 0; hostID < sizeof(networkAddresses) / sizeof(networkAddresses[0]); hostID++)
					addressesByHost->Add(hostID, networkAddresses[hostID]);

				auto newHive = boost::make_shared<HiveState> {TopologyID = newTopology->TopologyID, Services = std::map<int, ServiceState*>()};
				for (var hostID = 0; hostID < sizeof(networkAddresses) / sizeof(networkAddresses[0]); hostID++)
				{
					auto service = _services->Connect(networkAddresses[hostID]);
					try
					{
						auto serviceState = service->init(newTopology, addressesByHost, hostID);
						newHive->Services->Add(hostID, serviceState);
					}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
					finally
					{
						_services->Destroy(service);
					}
				}

				UpdateHiveState(newHive);
				BootStrapSchema();
				UpdateTopologySchema(newTopology);
			}

			void Database::UpdateTopologySchema(const boost::shared_ptr<Topology> &newTopology)
			{
				auto writes = std::map<int, ColumnWrites*>();

				// Insert the topology
				const Fastore::Include* tempVector[] = {boost::make_shared<Fastore::Include> {RowID = Encoder::Encode(newTopology->TopologyID), Value = Encoder::Encode(newTopology->TopologyID)}};
				writes->Add(std::map::TopologyID, boost::make_shared<ColumnWrites> {Includes = std::vector<Fastore::Include*>(tempVector, tempVector + sizeof(tempVector) / sizeof(tempVector[0]))});

				// Insert the hosts and pods
				auto hostWrites = boost::make_shared<ColumnWrites> {Includes = std::vector<Fastore::Include*>()};
				auto podWrites = boost::make_shared<ColumnWrites> {Includes = std::vector<Fastore::Include*>()};
				auto podHostWrites = boost::make_shared<ColumnWrites> {Includes = std::vector<Fastore::Include*>()};
				for (Thrift::Collections::THashSet<Alphora::Fastore::Host*>::const_iterator h = newTopology->Hosts.begin(); h != newTopology->Hosts.end(); ++h)
				{
					hostWrites->Includes->Add(boost::make_shared<Fastore::Include> {RowID = Encoder::Encode((*h)->Key), Value = Encoder::Encode((*h)->Key)});
					for (unknown::const_iterator p = h->Value.begin(); p != h->Value.end(); ++p)
					{
						podWrites->Includes->Add(boost::make_shared<Fastore::Include> {RowID = Encoder::Encode((*p)->Key), Value = Encoder::Encode((*p)->Key)});
						podHostWrites->Includes->Add(boost::make_shared<Fastore::Include> {RowID = Encoder::Encode((*p)->Key), Value = Encoder::Encode((*h)->Key)});
					}
				}
				writes->Add(std::map::HostID, hostWrites);
				writes->Add(std::map::PodID, podWrites);
				writes->Add(std::map::PodHostID, podHostWrites);

				Apply(writes, false);
			}

			boost::shared_ptr<Topology> Database::CreateTopology(int serviceWorkers[])
			{
				auto newTopology = boost::make_shared<Topology> {TopologyID = Guid::NewGuid()->GetHashCode()};
				newTopology->Hosts = std::map<int, std::map<int, std::vector<int>*>*>();
				auto podID = 0;
				for (var hostID = 0; hostID < sizeof(serviceWorkers) / sizeof(serviceWorkers[0]); hostID++)
				{
					auto pods = std::map<int, std::vector<int>*>();
					for (int i = 0; i < serviceWorkers[hostID]; i++)
					{
						pods->Add(podID, std::vector<int>()); // No no columns initially
						podID++;
					}
					newTopology->Hosts->Add(hostID, pods);
				}
				return newTopology;
			}

			Database::~Database()
			{
				InitializeInstanceFields();
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
				ClientException::ForceCleanup(() =>
				{
					delete _services;
				}
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
			   , () =>
			   {
					delete _workers;
			   }
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
			   , () =>
			   {
					UpdateHiveState(nullptr);
			   }
			   );
			}

			boost::shared_ptr<NetworkAddress> Database::GetServiceAddress(int hostID)
			{
//C# TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
				lock (_mapLock)
				{
					boost::shared_ptr<ServiceState> serviceState;
					if (!_hiveState->Services->TryGetValue(hostID, serviceState))
						throw boost::make_shared<ClientException>(std::string::Format("No service is currently associated with host ID ({0}).", hostID));

					return serviceState->Address;
				}
			}

			boost::shared_ptr<HiveState> Database::GetHiveState()
			{
				//TODO: Need to actually try to get new worker information, update topologyID, etc.
				//This just assumes that we won't add any workers once we are up and running
				auto newHive = boost::make_shared<HiveState>(new object[] {TopologyID = 0, Services = std::map<int, ServiceState*>()});

				auto tasks = std::vector<Task<KeyValuePair<int, ServiceState*>*>*>();
				for (unknown::const_iterator service = _hiveState->Services.begin(); service != _hiveState->Services.end(); ++service)
				{
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
					tasks->Add(Task::Factory::StartNew(() =>
					{
						auto serviceClient = _services[(*service)->Key];
						auto state = serviceClient->getState();
						if (!state::__isset.serviceState)
							throw boost::make_shared<ClientException>(std::string::Format("Host ({0}) is unexpectedly not part of the topology.", (*service)->Key));
							_services->Release(KeyValuePair<int, Service::Client*>((*service)->Key, serviceClient));
							return KeyValuePair<int, ServiceState*>((*service)->Key, state::ServiceState);
					}
				   ));
				}

				for (std::vector::const_iterator task = tasks->begin(); task != tasks->end(); ++task)
				{
					auto statePair = (*task)->Result;
					newHive->Services->Add(statePair::Key, statePair->Value);
				}

				//Don't care for now...
				newHive->ReportingHostID = newHive->Services->Keys->First();

				return newHive;
			}

			void Database::RefreshHiveState()
			{
				boost::shared_ptr<HiveState> newState = GetHiveState();
				UpdateHiveState(newState);
			}

			void Database::UpdateHiveState(const boost::shared_ptr<HiveState> &newState)
			{
//C# TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
				lock(_mapLock)
				{
					_hiveState = newState;

					// Maintain some indexes for quick access
					_workerStates.clear();
					_columnWorkers.clear();

					if (newState != nullptr)
						for (unknown::const_iterator service = newState->Services.begin(); service != newState->Services.end(); ++service)
							for (unknown::const_iterator worker = service->Value->Workers.begin(); worker != service->Value->Workers.end(); ++worker)
							{
								_workerStates.insert(make_pair((*worker)->PodID, boost::make_shared<Tuple<ServiceState*, WorkerState*>>((*service)->Value, *worker)));
								//TODO: Don't assume all repos are online.
								for (unknown::const_iterator repo = worker->RepositoryStatus.begin(); repo != worker->RepositoryStatus.end(); ++repo) //.Where(r => r.Value == RepositoryStatus.Online || r.Value == RepositoryStatus.Checkpointing)
								{
									boost::shared_ptr<PodMap> map;
									if (!_columnWorkers.TryGetValue((*repo)->Key, map))
									{
										map = boost::make_shared<PodMap>();
										_columnWorkers.insert(make_pair((*repo)->Key, map));
									}
									map->Pods.push_back((*worker)->PodID);
								}
							}
				}
			}

			boost::shared_ptr<NetworkAddress> Database::GetWorkerAddress(int podID)
			{
//C# TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
				lock (_mapLock)
				{
					// Attempt to find a worker for the given pod
					boost::shared_ptr<Tuple<ServiceState*, WorkerState*>> workerState;
					if (!_workerStates.TryGetValue(podID, workerState))
						throw boost::make_shared<ClientException>(std::string::Format("No Worker is currently associated with pod ID ({0}).", podID), ClientException::NoWorkerForColumn);

					return boost::make_shared<NetworkAddress> {Name = workerState->Item1->Address->Name, Port = workerState->Item2->Port};
				}
			}

			KeyValuePair<int, Worker::Client*> Database::GetWorker(int columnID)
			{
				if (columnID <= std::map::MaxSystemColumnID)
					return GetWorkerForSystemColumn();
				else
					return GetWorkerForColumn(columnID);
			}

			KeyValuePair<int, Worker::Client*> Database::GetWorkerForColumn(int columnID)
			{
				auto podId = GetWorkerIDForColumn(columnID);
				return KeyValuePair<int, Worker::Client*>(podId, _workers[podId]);
			}

			int Database::GetWorkerIDForColumn(int columnID)
			{
//C# TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
				lock (_mapLock)
				{
					boost::shared_ptr<PodMap> map;
					if (!_columnWorkers.TryGetValue(columnID, map))
					{
						// TODO: Update the hive state before erroring.

						auto error = boost::make_shared<ClientException>(std::string::Format("No worker is currently available for column ID ({0}).", columnID), ClientException::NoWorkerForColumn);
						error->getData()->Add("ColumnID", columnID);
						throw error;
					}

					map->Next = (map->Next + 1) % map->Pods.size();
					auto podId = map->Pods[map->Next];

					return podId;
				}
			}

			KeyValuePair<int, Worker::Client*> Database::GetWorkerForSystemColumn()
			{
				// For a system column, any worker will do, so just use an already connected worker
				int podID;
//C# TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
				lock (_mapLock)
				{
					podID = _workerStates.Keys->ElementAt(_nextSystemWorker);
					_nextSystemWorker = (_nextSystemWorker + 1) % _workerStates.size();
				}
				return KeyValuePair<int, Worker::Client*>(podID, _workers[podID]);
			}

			WorkerInfo *Database::DetermineWorkers(std::map<int, ColumnWrites*> &writes)
			{
				Monitor::Enter(_mapLock);
				auto taken = true;
				try
				{
					// TODO: is there a better better scheme than writing to every worker?  This method takes column IDs in case one arises.

					// Snapshot all needed pods
					auto results = (from ws in _workerStates select WorkerInfo(new object[] {PodID = ws::Key, Columns = (from r in ws->Value->Item2->RepositoryStatus where r->Value == RepositoryStatus::Online || r->Value == RepositoryStatus::Checkpointing select r::Key)->Union(from c in _schema->getKeys() where c <= std::map::MaxSystemColumnID select c)->ToArray()}))->ToArray();

					// Release lock during ensures
					Monitor::Exit(_mapLock);
					taken = false;

					return results;
				}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
				finally
				{
					if (taken)
						Monitor::Exit(_mapLock);
				}
			}

			void Database::AttemptRead(int columnId, Action<Worker::Client*> work)
			{
				std::map<int, std::exception> errors;
				auto elapsed = boost::make_shared<Stopwatch>();
				while (true)
				{
					// Determine the (next) worker to use
					auto worker = GetWorker(columnId);
					try
					{
						// If we've already failed with this worker, give up
						if (errors.size() > 0 && errors.find(worker.Key) != errors.end())
						{
							TrackErrors(errors);
							throw boost::make_shared<AggregateException>(from e in errors select e->Value);
						}

						try
						{
							elapsed->Restart();
							work(worker.Value);
							elapsed->Stop();
						}
						catch (std::exception &e)
						{
							// If the exception is an entity (exception coming from the remote), rethrow
							if (dynamic_cast<Thrift::Protocol::TBase*>(e) != nullptr)
								throw;

							if (errors.empty())
								errors = std::map<int, std::exception>();
							errors.insert(make_pair(worker.Key, e));
							continue;
						}

						// Succeeded, track any errors we received
						if (errors.size() > 0)
							TrackErrors(errors);

						// Succeeded, track the elapsed time
						TrackTime(worker.Key, elapsed->ElapsedTicks);

						break;
					}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
					finally
					{
						_workers->Release(worker);
					}
				}
			}

			void Database::AttemptWrite(int podId, Action<Worker::Client*> work)
			{
				auto elapsed = boost::make_shared<Stopwatch>();
				try
				{
					elapsed->Start();
					WorkerInvoke(podId, work);
					elapsed->Stop();
				}
				catch (std::exception &e)
				{
					// If the exception is an entity (exception coming from the remote), rethrow
					if (!(dynamic_cast<Thrift::Protocol::TBase*>(e) != nullptr))
						TrackErrors(std::map<int, std::exception> {{podId, e}});
					throw;
				}

				// Succeeded, track the elapsed time
				TrackTime(podId, elapsed->ElapsedTicks);
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
				return boost::make_shared<Transaction>(this, readIsolation, writeIsolation);
			}

			boost::shared_ptr<RangeSet> Database::GetRange(int columnIds[], Range range, int limit = MaxFetchLimit, const boost::shared_ptr<object> &startId
			{
				// Create the range query
				auto query = CreateQuery(range, limit, startId);

				// Make the range request
				std::map<int, ReadResult*> rangeResults;
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
				AttemptRead(range.ColumnID, (worker) =>
				{
					rangeResults = worker::query(std::map<int, Query*> {{range.ColumnID, query}});
				}
			   );
				auto rangeResult = rangeResults[range.ColumnID]->Answer->RangeValues[0];

				// Create the row ID query
				boost::shared_ptr<Query> rowIdQuery = GetRowsQuery(rangeResult);

				// Get the values for all but the range
				auto result = InternalGetValues(columnIds, range.ColumnID, rowIdQuery);

				// Add the range values into the result
				return ResultsToRangeSet(result, range.ColumnID, Array->find(columnIds, range.ColumnID), rangeResult);
			}

			boost::shared_ptr<DataSet> Database::InternalGetValues(int columnIds[], int exclusionColumnId, const boost::shared_ptr<Query> &rowIdQuery)
			{
				// Make the query request against all columns except for the range column
				auto tasks = std::vector<Task<std::map<int, ReadResult*>*>*>(sizeof(columnIds) / sizeof(columnIds[0]));
				for (int i = 0; i < sizeof(columnIds) / sizeof(columnIds[0]); i++)
				{
					auto columnId = columnIds[i];
					if (columnId != exclusionColumnId)
					{
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
						tasks->Add(Task::Factory::StartNew(() =>
						{
							std::map<int, ReadResult*> result;
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
							AttemptRead(columnId, (worker) =>
							{
								result = worker::query(std::map<int, Query*>() {{columnId, rowIdQuery}});
							}
						   );
							return result;
						}
					   ));
					}
				}

				// Combine all results into a single dictionary by column
				auto resultsByColumn = std::map<int, ReadResult*>(sizeof(columnIds) / sizeof(columnIds[0]));
				for (std::vector::const_iterator task = tasks->begin(); task != tasks->end(); ++task)
					for (unknown::const_iterator result = task->Result.begin(); result != task->Result.end(); ++result)
						resultsByColumn->Add((*result)->Key, (*result)->Value);

				return ResultsToDataSet(columnIds, rowIdQuery->RowIDs, resultsByColumn);
			}

			boost::shared_ptr<DataSet> Database::GetValues(int columnIds[], object rowIds[])
			{
				// Create the row ID query
				boost::shared_ptr<Query> rowIdQuery = boost::make_shared<Query> {RowIDs = EncodeRowIds(rowIds)};

				// Make the query
				return InternalGetValues(columnIds, -1, rowIdQuery);
			}

			std::vector<unsigned char[]> Database::EncodeRowIds(object rowIds[])
			{
				auto encodedRowIds = std::vector<unsigned char[]>(sizeof(rowIds) / sizeof(rowIds[0]));
				for (int i = 0; i < sizeof(rowIds) / sizeof(rowIds[0]); i++)
					encodedRowIds->Add(Encoder::Encode(rowIds[i]));
				return encodedRowIds;
			}

			boost::shared_ptr<DataSet> Database::ResultsToDataSet(int columnIds[], std::vector<unsigned char[]> &rowIDs, std::map<int, ReadResult*> &rowResults)
			{
				boost::shared_ptr<DataSet> result = boost::make_shared<DataSet>(rowIDs.size(), sizeof(columnIds) / sizeof(columnIds[0]));

				for (int y = 0; y < rowIDs.size(); y++)
				{
					object rowData[sizeof(columnIds) / sizeof(columnIds[0])];
					boost::shared_ptr<object> rowId = Fastore::Client::Encoder::Decode(rowIDs[y], _schema[columnIds[0]].getIDType());

					for (int x = 0; x < sizeof(columnIds) / sizeof(columnIds[0]); x++)
					{
						auto columnId = columnIds[x];
						if (rowResults.find(columnId) != rowResults.end())
							rowData[x] = Fastore::Client::Encoder::Decode(rowResults[columnId]->Answer->RowIDValues[y], _schema[columnId].getType());
					}

					result[y] = DataSet::DataSetRow {ID = rowId, Values = rowData};
				}

				return result;
			}

			boost::shared_ptr<RangeSet> Database::ResultsToRangeSet(const boost::shared_ptr<DataSet> &set_Renamed, int rangeColumnId, int rangeColumnIndex, const boost::shared_ptr<RangeResult> &rangeResult)
			{
				auto result = boost::make_shared<RangeSet> {Eof = rangeResult->Eof, Bof = rangeResult->Bof, Limited = rangeResult->Limited, Data = set_Renamed};

				int valueRowValue = 0;
				int valueRowRow = 0;
				for (int y = 0; y < set_Renamed->getCount(); y++)
				{
					set_Renamed[y]->Values[rangeColumnIndex] = Fastore::Client::Encoder::Decode(rangeResult->ValueRowsList[valueRowValue]->Value, _schema[rangeColumnId].getType());
					valueRowRow++;
					if (valueRowRow >= rangeResult->ValueRowsList[valueRowValue]->RowIDs->Count)
					{
						valueRowValue++;
						valueRowRow = 0;
					}
				}

				return result;
			}

			boost::shared_ptr<Query> Database::GetRowsQuery(const boost::shared_ptr<RangeResult> &rangeResult)
			{
				std::vector<unsigned char[]> rowIds = std::vector<unsigned char[]>();
				for (unknown::const_iterator valuerow = rangeResult->ValueRowsList.begin(); valuerow != rangeResult->ValueRowsList.end(); ++valuerow)
				{
					for (unknown::const_iterator rowid = valuerow->RowIDs.begin(); rowid != valuerow->RowIDs.end(); ++rowid)
						rowIds.push_back(*rowid);
				}

				return boost::make_shared<Query>(new object[] {RowIDs = rowIds});
			}

			boost::shared_ptr<Query> Database::CreateQuery(Range range, int limit, const boost::shared_ptr<object> &startId)
			{
				boost::shared_ptr<RangeRequest> rangeRequest = boost::make_shared<RangeRequest>();
				rangeRequest->Ascending = range.Ascending;
				rangeRequest->Limit = limit;

				if (range.Start.HasValue)
				{
					boost::shared_ptr<Fastore::RangeBound> bound = boost::make_shared<Fastore::RangeBound>();
					bound->Inclusive = range.Start.Value->Inclusive;
					bound->Value = Fastore::Client::Encoder::Encode(range.Start.Value->Bound);

					rangeRequest->First = bound;
				}

				if (range.End.HasValue)
				{
					boost::shared_ptr<Fastore::RangeBound> bound = boost::make_shared<Fastore::RangeBound>();
					bound->Inclusive = range.End.Value->Inclusive;
					bound->Value = Fastore::Client::Encoder::Encode(range.End.Value->Bound);

					rangeRequest->Last = bound;
				}

				if (startId != nullptr)
					rangeRequest->RowID = Fastore::Client::Encoder::Encode(startId);

				boost::shared_ptr<Query> rangeQuery = boost::make_shared<Query>();
				rangeQuery->Ranges = std::vector<RangeRequest*>();
				rangeQuery->Ranges->Add(rangeRequest);

				return rangeQuery;
			}

			void Database::Apply(std::map<int, ColumnWrites*> &writes, bool flush)
			{
				if (writes.size() > 0)
					while (true)
					{
						auto transactionID = boost::make_shared<TransactionID>(new object[] {Key = 0, Revision = 0});

						auto workers = DetermineWorkers(writes);

						auto tasks = StartWorkerWrites(writes, transactionID, workers);

						auto failedWorkers = std::map<int, Thrift::Protocol::TBase*>();
						auto workersByTransaction = ProcessWriteResults(workers, tasks, failedWorkers);

						if (FinalizeTransaction(workers, workersByTransaction, failedWorkers))
						{
							// If we've inserted/deleted system table(s), force a schema refresh
							if (writes.Keys->Min() <= std::map::MaxSystemColumnID)
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

			void Database::FlushWorkers(const boost::shared_ptr<TransactionID> &transactionID, WorkerInfo workers[])
			{
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
				auto flushTasks = (from w in workers select Task::Factory::StartNew(() =>
				{
					auto worker = _workers[w::PodID];
					try
					{
						worker->flush(transactionID);
					}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
					finally
					{
						_workers->Release(KeyValuePair<int, Worker::Client*>(w::PodID, worker));
					}
				}
			   )).ToArray();

				// TODO: need algorithm that moves on as soon as needed quantity reached without waiting for slower ones

				// Wait for critical number of workers to flush
				auto neededCount = __max(1, sizeof(workers) / sizeof(workers[0]) / 2);
				for (var i = 0; i < flushTasks->Length && i < neededCount; i++)
					flushTasks[i]->Wait();
			}

			void Database::Include(int columnIds[], const boost::shared_ptr<object> &rowId, object row[])
			{
				auto writes = EncodeIncludes(columnIds, rowId, row);
				Apply(writes, false);
			}

			boost::shared_ptr<SortedDictionary<TransactionID*, std::vector<WorkerInfo>*>> Database::ProcessWriteResults(WorkerInfo workers[], std::vector<Task<TransactionID*>*> &tasks, std::map<int, Thrift::Protocol::TBase*> &failedWorkers)
			{
				auto stopWatch = boost::make_shared<Stopwatch>();
				stopWatch->Start();
				auto workersByTransaction = boost::make_shared<SortedDictionary<TransactionID*, std::vector<WorkerInfo>*>>(TransactionIDComparer::Default);
				for (int i = 0; i < tasks.size(); i++)
				{
					// Attempt to fetch the result for each task
					boost::shared_ptr<TransactionID> resultId;
					try
					{
						//// if the task doesn't complete in time, assume failure; move on to the next one...
						//if (!tasks[i].Wait(Math.Max(0, WriteTimeout - (int)stopWatch.ElapsedMilliseconds)))
						//{
						//	failedWorkers.Add(i, null);
						//	continue;
						//}
						resultId = tasks[i]->Result;
					}
					catch (std::exception &e)
					{
						if (dynamic_cast<Thrift::Protocol::TBase*>(e) != nullptr)
							failedWorkers.insert(make_pair(i, dynamic_cast<Thrift::Protocol::TBase*>(e)));
						// else: Other errors were managed by AttemptWrite
						continue;
					}

					// If successful, group with other workers that returned the same revision
					std::vector<WorkerInfo> transactionBucket;
					if (!workersByTransaction->TryGetValue(resultId, transactionBucket))
					{
						transactionBucket = std::vector<WorkerInfo>();
						workersByTransaction->Add(resultId, transactionBucket);
					}
					transactionBucket.push_back(workers[i]);
				}
				return workersByTransaction;
			}

			bool Database::FinalizeTransaction(WorkerInfo workers[], const boost::shared_ptr<SortedDictionary<TransactionID*, std::vector<WorkerInfo>*>> &workersByTransaction, std::map<int, Thrift::Protocol::TBase*> &failedWorkers)
			{
				if (workersByTransaction->Count > 0)
				{
					auto max = workersByTransaction->Keys->Max();
					auto successes = workersByTransaction[max];
					if (successes.size() > (sizeof(workers) / sizeof(workers[0]) / 2))
					{
						// Transaction successful, commit all reached workers
						for (SortedDictionary<TransactionID*, std::vector<WorkerInfo>*>::const_iterator group = workersByTransaction->begin(); group != workersByTransaction->end(); ++group)
							for (unknown::const_iterator worker = group->Value.begin(); worker != group->Value.end(); ++worker)
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
								WorkerInvoke((*worker)->PodID, (client) =>
								{
									client::commit(max);
								}
							   );

						// Also send out a commit to workers that timed-out
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
						for (unknown::const_iterator worker = failedWorkers.Where(w => w->Value == nullptr).begin(); worker != failedWorkers.Where(w => w->Value == nullptr).end(); ++worker)
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
							WorkerInvoke((*worker)->Key, (client) =>
							{
								client::commit(max);
							}
						   );
						return true;
					}
					else
					{
						// Failure, roll-back successful prepares
						for (SortedDictionary<TransactionID*, std::vector<WorkerInfo>*>::const_iterator group = workersByTransaction->begin(); group != workersByTransaction->end(); ++group)
							for (unknown::const_iterator worker = group->Value.begin(); worker != group->Value.end(); ++worker)
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
								WorkerInvoke((*worker)->PodID, (client) =>
								{
									client::rollback((*group)->Key);
								}
							   );
					}
				}
				return false;
			}

			void Database::WorkerInvoke(int podID, Action<Worker::Client*> work)
			{
				auto client = _workers[podID];
				try
				{
					work(client);
				}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
				finally
				{
					_workers->Release(KeyValuePair<int, Worker::Client*>(podID, client));
				}
			}

			std::vector<Task<TransactionID*>*> Database::StartWorkerWrites(std::map<int, ColumnWrites*> &writes, const boost::shared_ptr<TransactionID> &transactionID, WorkerInfo workers[])
			{
				auto tasks = std::vector<Task<TransactionID*>*>(sizeof(workers) / sizeof(workers[0]));
				// Apply the modification to every worker, even if no work
				for (Alphora::Fastore::Client::Database->WorkerInfo::const_iterator worker = workers->begin(); worker != workers->end(); ++worker)
				{
					// Determine the set of writes that apply to the worker's repositories
					auto work = std::map<int, ColumnWrites*>();
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
					for (unknown::const_iterator columnId = worker->Columns->Where(c => writes.find(c) != writes.end()).begin(); columnId != worker->Columns->Where(c => writes.find(c) != writes.end()).end(); ++columnId)
						work->Add(*columnId, writes[*columnId]);

//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
					tasks->Add(Task::Factory::StartNew(() =>
					{
						boost::shared_ptr<TransactionID> result = boost::make_shared<TransactionID>();
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
						AttemptWrite((*worker)->PodID, (client) =>
						{
							result = client::apply(transactionID, work);
						}
					   );
						return result;
					}
				   ));
				}
				return tasks;
			}

			std::map<int, ColumnWrites*> Database::EncodeIncludes(int columnIds[], const boost::shared_ptr<object> &rowId, object row[])
			{
				auto writes = std::map<int, ColumnWrites*>();
//ORIGINAL LINE: byte[] rowIdb = Fastore.Client.Encoder.Encode(rowId);
//C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
				unsigned char *rowIdb = Fastore::Client::Encoder::Encode(rowId);

				for (int i = 0; i < sizeof(columnIds) / sizeof(columnIds[0]); i++)
				{
					boost::shared_ptr<Alphora::Fastore::Include> inc = boost::make_shared<Fastore::Include>();
					inc->RowID = rowIdb;
					inc->Value = Fastore::Client::Encoder::Encode(row[i]);

					boost::shared_ptr<ColumnWrites> wt = boost::make_shared<ColumnWrites>();
					wt->Includes = std::vector<Fastore::Include*>();
					wt->Includes->Add(inc);
					writes->Add(columnIds[i], wt);
				}
				return writes;
			}

			void Database::Exclude(int columnIds[], const boost::shared_ptr<object> &rowId)
			{
				Apply(EncodeExcludes(columnIds, rowId), false);
			}

			std::map<int, ColumnWrites*> Database::EncodeExcludes(int columnIds[], const boost::shared_ptr<object> &rowId)
			{
				auto writes = std::map<int, ColumnWrites*>();
//ORIGINAL LINE: byte[] rowIdb = Fastore.Client.Encoder.Encode(rowId);
//C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
				unsigned char *rowIdb = Fastore::Client::Encoder::Encode(rowId);

				for (int i = 0; i < sizeof(columnIds) / sizeof(columnIds[0]); i++)
				{
					boost::shared_ptr<Alphora::Fastore::Exclude> inc = boost::make_shared<Fastore::Exclude>();
					inc->RowID = rowIdb;

					boost::shared_ptr<ColumnWrites> wt = boost::make_shared<ColumnWrites>();
					wt->Excludes = std::vector<Fastore::Exclude*>();
					wt->Excludes->Add(inc);
					writes->Add(columnIds[i], wt);
				}
				return writes;
			}

			Statistic *Database::GetStatistics(int columnIds[])
			{
				// Make the request against each column
				auto tasks = std::vector<Task<Fastore::Statistic*>*>(sizeof(columnIds) / sizeof(columnIds[0]));
				for (var i = 0; i < sizeof(columnIds) / sizeof(columnIds[0]); i++)
				{
					auto columnId = columnIds[i];
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
					tasks->Add(Task::Factory::StartNew(() =>
					{
						boost::shared_ptr<Fastore::Statistic> result = nullptr;
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
						AttemptRead(columnId, (worker) =>
						{
							const int tempVector2[] = {columnId};
							result = worker::getStatistics(std::vector<int>(tempVector2, tempVector2 + sizeof(tempVector2) / sizeof(tempVector2[0])))[0];
						}
					   );
						return result;
					}
				   ));
				}

				return (from t in tasks let r = t::Result select Statistic {Total = r::Total, Unique = r::Unique})->ToArray();
			}

			boost::shared_ptr<Schema> Database::GetSchema()
			{
				if (_schema->empty())
					_schema = LoadSchema();
				return boost::make_shared<Schema>(_schema);
			}

			boost::shared_ptr<Schema> Database::LoadSchema()
			{
				auto schema = boost::make_shared<Schema>();
				auto finished = false;
				while (!finished)
				{
					auto columns = GetRange(std::map::ColumnColumns, Range {ColumnID = std::map::ColumnID, Ascending = true}, MaxFetchLimit);
					for (Alphora::Fastore::Client::DataSet::const_iterator column = columns->getData()->begin(); column != columns->getData()->end(); ++column)
					{
						auto def = ColumnDef {ColumnID = static_cast<int>((*column)->Values[0]), Name = static_cast<std::string>((*column)->Values[1]), Type = static_cast<std::string>((*column)->Values[2]), IDType = static_cast<std::string>((*column)->Values[3]), BufferType = static_cast<BufferType>((*column)->Values[4])};
						schema->insert(make_pair(def.getColumnID(), def));
					}
					finished = !columns->getLimited();
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
				ColumnDef id = ColumnDef();
				ColumnDef name = ColumnDef();
				ColumnDef vt = ColumnDef();
				ColumnDef idt = ColumnDef();
				ColumnDef unique = ColumnDef();

				id.setColumnID(std::map::ColumnID);
				id.setName("Column.ID");
				id.setType("Int");
				id.setIDType("Int");
				id.setBufferType(Identity);

				name.setColumnID(std::map::ColumnName);
				name.setName("Column.Name");
				name.setType("String");
				name.setIDType("Int");
				name.setBufferType(Unique);

				vt.setColumnID(std::map::ColumnValueType);
				vt.setName("Column.ValueType");
				vt.setType("String");
				vt.setIDType("Int");
				vt.setBufferType(Multi);

				idt.setColumnID(std::map::ColumnRowIDType);
				idt.setName("Column.RowIDType");
				idt.setType("String");
				idt.setIDType("Int");
				idt.setBufferType(Multi);

				unique.setColumnID(std::map::ColumnBufferType);
				unique.setName("Column.BufferType");
				unique.setType("Int");
				unique.setIDType("Int");
				unique.setBufferType(Multi);

				_schema = boost::make_shared<Schema>();

				_schema->insert(make_pair(std::map::ColumnID, id));
				_schema->insert(make_pair(std::map::ColumnName, name));
				_schema->insert(make_pair(std::map::ColumnValueType, vt));
				_schema->insert(make_pair(std::map::ColumnRowIDType, idt));
				_schema->insert(make_pair(std::map::ColumnBufferType, unique));

				//Boot strapping is done, pull in real schema
				RefreshSchema();
			}

			std::map<int, TimeSpan> Database::Ping()
			{
				// Setup the set of hosts
//ORIGINAL LINE: int[] hostIDs;
//C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
				int *hostIDs;
//C# TO C++ CONVERTER TODO TASK: There is no built-in support for multithreading in native C++:
				lock (_mapLock)
				{
					hostIDs = _hiveState->Services->Keys->ToArray();
				}

				// Start a ping request to each
//C# TO C++ CONVERTER TODO TASK: Lambda expressions and anonymous methods are not converted to native C++ unless the option to convert to C++11 lambdas is selected:
				auto tasks = from id in hostIDs select Task::Factory::StartNew<KeyValuePair<int, TimeSpan>*> (() =>
				{
					auto service = _services[id];
					try
					{
						auto timer = boost::make_shared<Stopwatch>();
						timer->Start();
						service->ping();
						timer->Stop();
						return KeyValuePair<int, TimeSpan>(id, timer->Elapsed);
					}
//C# TO C++ CONVERTER TODO TASK: There is no native C++ equivalent to the exception 'finally' clause:
					finally
					{
						_services->Release(KeyValuePair<int, Service::Client*>(id, service));
					}
				}
			   );

				// Gather ping results
				auto result = std::map<int, TimeSpan>(sizeof(hostIDs) / sizeof(hostIDs[0]));
				for (System::Collections::Generic::IEnumerable::const_iterator task = tasks->begin(); task != tasks->end(); ++task)
				{
					try
					{
						auto item = (*task)->Result;
						result->Add(item::Key, item->Value);
					}
					catch (...)
					{
						// TODO: report errors
					}
				}

				return result;
			}

			void Database::InitializeInstanceFields()
			{
				_mapLock = boost::make_shared<object>();
				_workerStates = std::map<int, Tuple<ServiceState*, WorkerState*>*>();
				_columnWorkers = std::map<int, PodMap*>();
				_nextSystemWorker = 0;
				_writeTimeout = DefaultWriteTimeout;
			}
		}
	}
}
