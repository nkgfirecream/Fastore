﻿#pragma once

#include "IDataAccess.h"
#include "ConnectionPool.h"
#include "ServiceAddress.h"
#include "ClientException.h"
#include "Transaction.h"
#include "DataSet.h"
#include "ColumnDef.h"
#include <string>
#include <map>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <thrift/protocol/TProtocol.h>
#include "../FastoreCommunication/Comm_types.h"
#include "../FastoreCommunication/Service.h"
#include "../FastoreCommunication/Worker.h"
#include "typedefs.h"
#include <future>

using namespace fastore::communication;
using namespace apache::thrift::protocol;

namespace fastore { namespace client
{
	class Transaction;

	// TODO: concurrency
	class Database : public IDataAccess
	{
			
	public:
		/// <summary> The maximum number of rows to attempt to fetch at one time. </summary>
		static const int MaxFetchLimit = 500;
		/// <summary> The default timeout for write operations. </summary>
		static const int DefaultWriteTimeout = 1000;

	private:
		static Topology CreateTopology(const std::vector<int>& serviceWorkers);
		static Query GetRowsQuery(const RangeResult& rangeResult);
		static Query CreateQuery(const Range range, const int limit, const boost::optional<std::string>& startId);

		class PodMap
		{
		public:
			std::vector<int> Pods;
			int Next;
		};

		class WorkerInfo
		{
		public:
			int PodID;
			std::vector<int> Columns;
		};


	private:
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
	
	public:
	

		Database(std::vector<ServiceAddress> addresses);
		~Database();

		boost::shared_ptr<Transaction> Begin(bool readIsolation, bool writeIsolation);

		/// <summary> Given a set of column IDs and range criteria, retrieve a set of values. </summary>
		RangeSet GetRange(const ColumnIDs& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId = boost::optional<std::string>());
		DataSet GetValues(const ColumnIDs& columnIds, const std::vector<std::string>& rowIds);

		void Include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row);
		void Exclude(const ColumnIDs& columnIds, const std::string& rowId);

		std::vector<Statistic> GetStatistics(const ColumnIDs& columnIds);
		std::map<int, long long> Ping();

		void Apply(const std::map<int, boost::shared_ptr<ColumnWrites>>& writes, const bool flush);
		Schema GetSchema();

		void RefreshSchema();

				/// <summary> ApplyTimeout specifies the maximum time in milliseconds to wait for workers to respond to an apply request. </summary>
		/// <remarks> The default is 1000 (1 second). </remarks>
		const int& getWriteTimeout() const;
		void setWriteTimeout(const int &value);

	private:
		NetworkAddress& GetServiceAddress(int hostID);

		HiveState GetHiveState();

		void RefreshHiveState();

		void UpdateHiveState(const HiveState &newState);

		NetworkAddress GetWorkerAddress(int podID);

		void UpdateTopologySchema(const Topology &newTopology);
		/// <summary> Get the next worker to use for the given column ID. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		std::pair<int, WorkerClient> GetWorker(int columnID);

		std::pair<int, WorkerClient> GetWorkerForColumn(int columnID);

		int GetWorkerIDForColumn(int columnID);

		std::pair<int, WorkerClient> GetWorkerForSystemColumn();

		/// <summary> Determine the workers to write-to for the given column IDs. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		std::vector<WorkerInfo> DetermineWorkers(const std::map<int, boost::shared_ptr<ColumnWrites>> &writes);

		/// <summary> Performs a read operation against a worker and manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void AttemptRead(int columnId, std::function<void(WorkerClient)> work);

		/// <summary> Performs a write operation against a specific worker; manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void AttemptWrite(int podId, std::function<void(WorkerClient)> work);

		/// <summary> Tracks the time taken by the given worker. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void TrackTime(int podId, long long p);

		/// <summary> Tracks errors reported by workers. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void TrackErrors(std::map<int, std::exception> &errors);

		DataSet InternalGetValues(const ColumnIDs& columnIds, const int exclusionColumnId, const Query& rowIdQuery);	
		DataSet ResultsToDataSet(const ColumnIDs& columnIds, const std::vector<std::string>& rowIDs, const std::map<int, ReadResult>& rowResults);
		RangeSet ResultsToRangeSet(DataSet& set, const int rangeColumnId, const int rangeColumnIndex, const RangeResult& rangeResult);	

		void FlushWorkers(const TransactionID& transactionID, const std::vector<WorkerInfo>& workers);

		std::map<TransactionID, std::vector<WorkerInfo>> ProcessWriteResults(const std::vector<WorkerInfo>& workers, const std::vector<boost::shared_ptr<std::future<TransactionID>>>& tasks, std::map<int, boost::shared_ptr<TProtocol>>& failedWorkers);

		bool FinalizeTransaction(const std::vector<WorkerInfo>& workers, const std::map<TransactionID, std::vector<WorkerInfo>>& workersByTransaction, std::map<int, boost::shared_ptr<TProtocol>>& failedWorkers);

		/// <summary> Invokes a given command against a worker. </summary>
		void WorkerInvoke(int podID, std::function<void(WorkerClient)> work);

		/// <summary> Apply the writes to each worker, even if there are no modifications for that worker. </summary>
		std::vector<boost::shared_ptr<std::future<TransactionID>>> StartWorkerWrites(const std::map<int, boost::shared_ptr<ColumnWrites>> &writes, const TransactionID &transactionID, const std::vector<WorkerInfo>& workers);

		std::map<int, boost::shared_ptr<ColumnWrites>> CreateIncludes(const ColumnIDs& columnIds, const std::string& rowId, std::vector<std::string> row);
		std::map<int, boost::shared_ptr<ColumnWrites>> CreateExcludes(const ColumnIDs& columnIds, const std::string& rowId);

		Schema LoadSchema();
		void BootStrapSchema();
	};
}}
