#pragma once

#include "IDataAccess.h"
#include "ConnectionPool.h"
#include "ServiceAddress.h"
#include "ClientException.h"
#include "Transaction.h"
#include "DataSet.h"
#include <Buffer/ColumnDef.h>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <thrift/protocol/TProtocol.h>
#include <Communication/Comm_types.h>
#include <Communication/Service.h>
#include <Communication/Worker.h>
#include <Communication/Store.h>
#include "typedefs.h"
#include <future>
#include "TransactionIDGenerator.h"
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

using namespace fastore::communication;
using namespace apache::thrift::protocol;

namespace fastore { namespace client
{
	class Transaction;

	// TODO: concurrency
	class Database : public IDataAccess
	{

		friend class Transaction;
			
	public:
		// The maximum number of rows to attempt to fetch at one time.
		static const int MaxFetchLimit = 500;
		// The default timeout for write operations.
		static const int DefaultWriteTimeout = 1000;
		// Maximum number of times to retry write operations.
		static const int MaxWriteRetries = 10;

	private:
		struct PodMap
		{
			std::vector<PodID> Pods;
			int Next;
		};

		typedef boost::bimap<boost::bimaps::multiset_of<PodID>, boost::bimaps::multiset_of<ColumnID>, boost::bimaps::set_of_relation<>> WorkerColumnBimap;
		typedef WorkerColumnBimap::value_type WorkerColumnPair;

		typedef boost::bimap<boost::bimaps::set_of<HostID>, boost::bimaps::multiset_of<PodID>, boost::bimaps::set_of_relation<>> StoreWorkerBimap;
		typedef StoreWorkerBimap::value_type StoreWorkerPair;

		/*struct WorkerInfo
		{
			PodID podID;
			ColumnIDs columns;
		};

		struct StoreInfo
		{
			HostID hostId;
			std::vector<PodID> pods;
		};*/

		struct ColumnWriteResult
		{
			bool validateRequired;
			std::map<Revision, std::vector<PodID>> workersByRevision;
			std::vector<PodID> failedWorkers;
		};
		
		boost::shared_ptr<boost::mutex> _lock;

		// Connection pool of services by host ID
		ConnectionPool<HostID, ServiceClient> _services;

		// Connection pool of stores by host ID
		ConnectionPool<HostID, StoreClient> _stores;

		// Connected workers by pod ID
		ConnectionPool<PodID, WorkerClient> _workers;

		// Worker states by pod ID
		std::map<PodID, std::pair<ServiceState, WorkerState>> _workerStates;

		// Pod map (round robin pod IDs) per column
		std::map<PodID, PodMap> _columnWorkers;

		// The next worker to use to retrieve system columns from
		int _nextSystemWorker;

		// Latest known state of the hive
		HiveState _hiveState;

		// Currently known schema
		Schema _schema;

		int _writeTimeout;

		std::unique_ptr<TransactionIDGenerator> _generator;

		Topology CreateTopology(const std::vector<int>& serviceWorkers);
		Query GetRowsQuery(const RangeResult& rangeResult);
		Query CreateQuery(const Range range, const int limit, const boost::optional<std::string>& startId);

		NetworkAddress& GetServiceAddress(HostID hostID);

		NetworkAddress GetStoreAddress(HostID hostID);

		void RefreshSchema();

		HiveState GetHiveState();
		void RefreshHiveState();
		void UpdateHiveState(const HiveState &newState);

		NetworkAddress GetWorkerAddress(PodID podID);

		void UpdateTopologySchema(const Topology &newTopology);
		/// <summary> Get the next worker to use for the given column ID. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		std::pair<PodID, WorkerClient> GetWorker(ColumnID columnID);

		std::pair<PodID, WorkerClient> GetWorkerForColumn(ColumnID columnID);

		PodID GetWorkerIDForColumn(ColumnID columnID);

		std::pair<PodID, WorkerClient> GetWorkerForSystemColumn();

		/// <summary> Determine the workers to write-to for the given column IDs. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		WorkerColumnBimap DetermineWorkers(const std::map<ColumnID, ColumnWrites>& writes);
		StoreWorkerBimap DetermineStores(const WorkerColumnBimap& workers);

		/// <summary> Performs a read operation against a worker and manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void AttemptRead(ColumnID columnId, std::function<void(WorkerClient)> work);

		/// <summary> Performs a write operation against a specific worker; manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void AttemptWrite(PodID podId, std::function<void(WorkerClient)> work);

		/// <summary> Tracks the time taken by the given worker. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void TrackTime(PodID podId, long long p);

		/// <summary> Tracks errors reported by workers. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void TrackErrors(std::map<PodID, std::exception> &errors);

		DataSet InternalGetValues(const ColumnIDs& columnIds, const ColumnID exclusionColumnId, const Query& rowIdQuery);	
		DataSet ResultsToDataSet(const ColumnIDs& columnIds, const std::vector<std::string>& rowIDs, const ReadResults& rowResults);
		RangeSet ResultsToRangeSet(DataSet& set, size_t rangeColumnIndex, const RangeResult& rangeResult);	
		void apply(const std::map<ColumnID, ColumnWrites>& writes, const bool flush);
		void apply(TransactionID transactionID, const std::map<ColumnID, ColumnWrites>& writes, const bool flush);

		/// <summary> Invokes a given command against a worker. </summary>
		void WorkerInvoke(PodID podID, std::function<void(WorkerClient)> work);

		/// <summary> Invokes a given command against a service. </summary>
		void ServiceInvoke(HostID hostID, std::function<void(ServiceClient)> work);

		/// <summary> Tests each worker to see if it can prepare.
		//std::vector<std::future<PrepareResults>> Prepare(const ColumnIDs& columnIDs, const TransactionID &transactionID, const WorkerColumnBimap& workers);

		/// <summary> Apply the writes to each worker, even if there are no modifications for that worker. </summary>
		std::map<PodID, std::future<PrepareResults>> Apply(const ColumnIDs& columnIDs, const TransactionID &transactionID, const WorkerColumnBimap& workers);

		/// <summary> Waits for all the tasks started by prepare to finish
		std::unordered_map<ColumnID, Database::ColumnWriteResult> Database::ProcessPrepareResults(const WorkerColumnBimap& workers, std::map<PodID, std::future<PrepareResults>>& tasks);

		/// <summary> Checks to see if all columns have a majority of their respective workers
		bool CanFinalizeTransaction(const std::unordered_map<ColumnID, ColumnWriteResult>& columnWriteResultsByColumn);

		/// <summary> Writes data to columns and stores
		void Commit(const TransactionID transactionID, const std::map<ColumnID, ColumnWrites>& writes, WorkerColumnBimap& workers, StoreWorkerBimap& stores);

		/// <summary> Rollsback a prepared transaction
		//void Rollback();

		/// <summary> Checks to see if the given transaction has made it to disk
		void Flush(const TransactionID& transactionID, const StoreWorkerBimap& stores);
		
		
		std::map<ColumnID, ColumnWrites> CreateIncludes(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row);
		std::map<ColumnID, ColumnWrites> CreateExcludes(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row);

		Schema LoadSchema();
		void BootStrapSchema();
			
	public:
		Database(std::vector<ServiceAddress> addresses);
		~Database();

		boost::shared_ptr<Transaction> begin(bool readsConflict);

		/// <summary> Given a set of column IDs and range criteria, retrieve a set of values. </summary>
		RangeSet getRange(const ColumnIDs& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId = boost::optional<std::string>());
		DataSet getValues(const ColumnIDs& columnIds, const std::vector<std::string>& rowIds);

		void include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row);
		void exclude(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row);

		//TODO: Needs to pull row values manually if outside of transaction
		void exclude(const ColumnIDs& columnIds, const std::string& rowId);
		
		std::vector<Statistic> getStatistics(const ColumnIDs& columnIds);
		std::map<HostID, long long> ping();

		void checkpoint();
		Schema getSchema();		

				/// <summary> ApplyTimeout specifies the maximum time in milliseconds to wait for workers to respond to an apply request. </summary>
		/// <remarks> The default is 1000 (1 second). </remarks>
		const int& getWriteTimeout() const;
		void setWriteTimeout(const int &value);

		TransactionID generateTransactionID();
	};
}}
