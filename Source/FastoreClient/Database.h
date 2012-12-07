﻿#pragma once

#include "IDataAccess.h"
#include "ConnectionPool.h"
#include "ServiceAddress.h"
#include "ClientException.h"
#include "Transaction.h"
#include "DataSet.h"
#include "../FastoreCommon/Buffer/ColumnDef.h"
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <thrift/protocol/TProtocol.h>
#include "../FastoreCommon/Communication/Comm_types.h"
#include "../FastoreCommon/Communication/Service.h"
#include "../FastoreCommon/Communication/Worker.h"
#include "typedefs.h"
#include <future>
#include "TransactionIDGenerator.h"

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

		struct WorkerInfo
		{
			PodID podID;
			ColumnIDs Columns;
		};

		struct ColumnWriteResult
		{
			bool validateRequired;
			std::map<Revision, std::vector<WorkerInfo>> workersByRevision;
		};

		
		boost::shared_ptr<boost::mutex> _lock;

		// Connection pool of services by host ID
		ConnectionPool<HostID, ServiceClient> _services;

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

		static std::unique_ptr<TransactionIDGenerator> _generator;

		static Topology CreateTopology(const std::vector<int>& serviceWorkers);
		static Query GetRowsQuery(const RangeResult& rangeResult);
		static Query CreateQuery(const Range range, const int limit, const boost::optional<std::string>& startId);

		NetworkAddress& GetServiceAddress(HostID hostID);

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
		std::vector<WorkerInfo> DetermineWorkers(const std::map<ColumnID, ColumnWrites>& writes);

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

		//void FlushWorkers(const TransactionID& transactionID, const std::vector<WorkerInfo>& workers);

		std::unordered_map<ColumnID, Database::ColumnWriteResult> Database::ProcessWriteResults
		(
			const std::vector<WorkerInfo>& workers, 
			std::vector<std::future<PrepareResults>>& tasks, 
			std::unordered_map<PodID, boost::shared_ptr<TProtocol>>& failedWorkers
		);

		bool FinalizeTransaction(const std::vector<WorkerInfo>& workers, const std::unordered_map<ColumnID, ColumnWriteResult>& workersByTransaction, std::unordered_map<PodID, boost::shared_ptr<TProtocol>>& failedWorkers);

		/// <summary> Invokes a given command against a worker. </summary>
		void WorkerInvoke(PodID podID, std::function<void(WorkerClient)> work);

		/// <summary> Invokes a given command against a service. </summary>
		void ServiceInvoke(HostID hostID, std::function<void(ServiceClient)> work);

		/// <summary> Apply the writes to each worker, even if there are no modifications for that worker. </summary>
		std::vector<std::future<PrepareResults>> StartWorkerWrites(const ColumnIDs& columnIDs, const TransactionID &transactionID, const std::vector<WorkerInfo>& workers);

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

		static TransactionID generateTransactionID();
	};
}}
