#pragma once

#include "IDataAccess.h"
#include "ConnectionPool.h"
#include "ServiceAddress.h"
#include "ClientException.h"
#include "Transaction.h"
#include "DataSet.h"
#include "Statistic.h"
#include "ColumnDef.h"
#include <string>
#include <map>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include "..\FastoreCommunication\Comm_types.h"

using namespace fastore::communication;

namespace fastore
{
	// TODO: concurrency
	class Database : public IDataAccess
	{
			
	public:
		/// <summary> The maximum number of rows to attempt to fetch at one time. </summary>
		static const int MaxFetchLimit = 500;
		/// <summary> The default timeout for write operations. </summary>
		static const int DefaultWriteTimeout = 1000;

	private:
		static boost::shared_ptr<Topology> CreateTopology(int serviceWorkers[]);
		static std::vector<unsigned char[]> EncodeRowIds(object rowIds[]);
		static boost::shared_ptr<Query> GetRowsQuery(const boost::shared_ptr<RangeResult> &rangeResult);
		static boost::shared_ptr<Query> CreateQuery(Range range, int limit, const boost::shared_ptr<object> &startId);

		class PodMap
		{
		public:
			PodMap();
			std::vector<int> Pods;
			int Next;
		};

		class WorkerInfo
		{
		public:
			int PodID;
//ORIGINAL LINE: public int[] Columns;
//C# TO C++ CONVERTER WARNING: Since the array size is not known in this declaration, C# to C++ Converter has converted this array to a pointer.  You will need to call 'delete[]' where appropriate:
			int *Columns;
		};


	private:
		boost::shared_ptr<object> _mapLock;

		// Connection pool of services by host ID
		boost::shared_ptr<ConnectionPool<int, ServiceClient*>> _services;

		// Connected workers by pod ID
		boost::shared_ptr<ConnectionPool<int, WorkerClient*>> _workers;

		// Worker states by pod ID
		std::map<int, std::pair<ServiceState*, WorkerState*>*> _workerStates;

		// Pod map (round robin pod IDs) per column
		std::map<int, PodMap*> _columnWorkers;

		// The next worker to use to retrieve system columns from
		int _nextSystemWorker;

		// Latest known state of the hive
		boost::shared_ptr<HiveState> _hiveState;

		// Currently known schema
		boost::shared_ptr<Schema> _schema;

		int _writeTimeout;
	
	public:
	

		Database(ServiceAddress addresses[]);
		~Database();

		boost::shared_ptr<Transaction> Begin(bool readIsolation, bool writeIsolation);

		/// <summary> Given a set of column IDs and range criteria, retrieve a set of values. </summary>
		RangeResult GetRange(RangeRequest range);
		std::vector<std::string> GetValues(std::vector<int> columnIds, std::vector<std::string> rowIds);
		void Include(std::vector<int> columnIds, std::string rowId, std::vector<std::string> row);
		void Exclude(std::vector<int> columnIds, std::string rowId);
		std::vector<Statistic> GetStatistics(std::vector<int> columnIds);

		//std::map<int, TimeSpan> Ping();

		void Apply(std::map<int, ColumnWrites*> &writes, bool flush);
		boost::shared_ptr<Schema> GetSchema();

		void RefreshSchema();

				/// <summary> ApplyTimeout specifies the maximum time in milliseconds to wait for workers to respond to an apply request. </summary>
		/// <remarks> The default is 1000 (1 second). </remarks>
		const int &getWriteTimeout() const;
		void setWriteTimeout(const int &value);

	private:
		boost::shared_ptr<NetworkAddress> GetServiceAddress(int hostID);

		boost::shared_ptr<HiveState> GetHiveState();

		void RefreshHiveState();

		void UpdateHiveState(const boost::shared_ptr<HiveState> &newState);

		boost::shared_ptr<NetworkAddress> GetWorkerAddress(int podID);

		void UpdateTopologySchema(const boost::shared_ptr<Topology> &newTopology);
		/// <summary> Get the next worker to use for the given column ID. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		KeyValuePair<int, Worker::Client*> GetWorker(int columnID);

		KeyValuePair<int, Worker::Client*> GetWorkerForColumn(int columnID);

		int GetWorkerIDForColumn(int columnID);

		KeyValuePair<int, Worker::Client*> GetWorkerForSystemColumn();

		/// <summary> Determine the workers to write-to for the given column IDs. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		WorkerInfo *DetermineWorkers(std::map<int, ColumnWrites*> &writes);

		/// <summary> Performs a read operation against a worker and manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void AttemptRead(int columnId, Action<Worker::Client*> work);

		/// <summary> Performs a write operation against a specific worker; manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void AttemptWrite(int podId, Action<Worker::Client*> work);

		/// <summary> Tracks the time taken by the given worker. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void TrackTime(int podId, long long p);

		/// <summary> Tracks errors reported by workers. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void TrackErrors(std::map<int, std::exception> &errors);

		boost::shared_ptr<DataSet> InternalGetValues(int columnIds[], int exclusionColumnId, const boost::shared_ptr<Query> &rowIdQuery);	
		boost::shared_ptr<DataSet> ResultsToDataSet(int columnIds[], std::vector<unsigned char[]> &rowIDs, std::map<int, ReadResult*> &rowResults);
		boost::shared_ptr<RangeSet> ResultsToRangeSet(const boost::shared_ptr<DataSet> &set_Renamed, int rangeColumnId, int rangeColumnIndex, const boost::shared_ptr<RangeResult> &rangeResult);	

		void FlushWorkers(const boost::shared_ptr<TransactionID> &transactionID, WorkerInfo workers[]);

		boost::shared_ptr<SortedDictionary<TransactionID*, std::vector<WorkerInfo>*>> ProcessWriteResults(WorkerInfo workers[], std::vector<Task<TransactionID*>*> &tasks, std::map<int, Thrift::Protocol::TBase*> &failedWorkers);

		bool FinalizeTransaction(WorkerInfo workers[], const boost::shared_ptr<SortedDictionary<TransactionID*, std::vector<WorkerInfo>*>> &workersByTransaction, std::map<int, Thrift::Protocol::TBase*> &failedWorkers);

		/// <summary> Invokes a given command against a worker. </summary>
		void WorkerInvoke(int podID, Action<Worker::Client*> work);

		/// <summary> Apply the writes to each worker, even if there are no modifications for that worker. </summary>
		std::vector<Task<TransactionID*>*> StartWorkerWrites(std::map<int, ColumnWrites*> &writes, const boost::shared_ptr<TransactionID> &transactionID, WorkerInfo workers[]);

		std::map<int, ColumnWrites*> EncodeIncludes(int columnIds[], const boost::shared_ptr<object> &rowId, object row[]);
		std::map<int, ColumnWrites*> EncodeExcludes(int columnIds[], const boost::shared_ptr<object> &rowId);

		boost::shared_ptr<Schema> LoadSchema();
		void BootStrapSchema();

		void InitializeInstanceFields();
	};
}
