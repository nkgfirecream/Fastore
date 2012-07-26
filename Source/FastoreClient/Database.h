#pragma once

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
#include <thrift\protocol\TProtocol.h>
#include "..\FastoreCommunication\Comm_types.h"
#include "..\FastoreCommunication\Service.h"
#include "..\FastoreCommunication\Worker.h"
#include "typedefs.h"

using namespace fastore::communication;
using namespace apache::thrift::protocol;

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
		static std::vector<unsigned char[]> EncodeRowIds(const std::vector<std::string>& rowIds);
		static boost::shared_ptr<Query> GetRowsQuery(const boost::shared_ptr<RangeResult>& rangeResult);
		static boost::shared_ptr<Query> CreateQuery(RangeRequest range, int limit, const std::string& startId);

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
		//TODO: Locking mechanism
		void* _mapLock;

		// Connection pool of services by host ID
		boost::shared_ptr<ConnectionPool<int, boost::shared_ptr<ServiceClient>>> _services;

		// Connected workers by pod ID
		boost::shared_ptr<ConnectionPool<int, boost::shared_ptr<WorkerClient>>> _workers;

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

		void Apply(std::map<int, boost::shared_ptr<ColumnWrites>>& writes, bool flush);
		boost::shared_ptr<Schema> GetSchema();

		void RefreshSchema();

				/// <summary> ApplyTimeout specifies the maximum time in milliseconds to wait for workers to respond to an apply request. </summary>
		/// <remarks> The default is 1000 (1 second). </remarks>
		const int& getWriteTimeout() const;
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
		std::pair<int, boost::shared_ptr<WorkerClient>> GetWorker(int columnID);

		std::pair<int,boost::shared_ptr<WorkerClient>> GetWorkerForColumn(int columnID);

		int GetWorkerIDForColumn(int columnID);

		std::pair<int, boost::shared_ptr<WorkerClient>> GetWorkerForSystemColumn();

		/// <summary> Determine the workers to write-to for the given column IDs. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		boost::shared_ptr<WorkerInfo> DetermineWorkers(std::map<int, boost::shared_ptr<ColumnWrites>> &writes);

		/// <summary> Performs a read operation against a worker and manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void AttemptRead(int columnId, std::function<void(boost::shared_ptr<WorkerClient>)> work);

		/// <summary> Performs a write operation against a specific worker; manages errors and retries. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void AttemptWrite(int podId, std::function<void(boost::shared_ptr<WorkerClient>)> work);

		/// <summary> Tracks the time taken by the given worker. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void TrackTime(int podId, long long p);

		/// <summary> Tracks errors reported by workers. </summary>
		/// <remarks> This method is thread-safe. </remarks>
		void TrackErrors(std::map<int, std::exception> &errors);

		boost::shared_ptr<DataSet> InternalGetValues(int columnIds[], int exclusionColumnId, const boost::shared_ptr<Query> &rowIdQuery);	
		boost::shared_ptr<DataSet> ResultsToDataSet(int columnIds[], std::vector<unsigned char[]> &rowIDs, std::map<int, ReadResult*> &rowResults);
		boost::shared_ptr<RangeResult> ResultsToRangeSet(const boost::shared_ptr<DataSet> &set_Renamed, int rangeColumnId, int rangeColumnIndex, const boost::shared_ptr<RangeResult> &rangeResult);	

		void FlushWorkers(const boost::shared_ptr<TransactionID> &transactionID, WorkerInfo workers[]);

		boost::shared_ptr<SortedDictionary<boost::shared_ptr<TransactionID>, std::vector<WorkerInfo>*>> ProcessWriteResults(WorkerInfo workers[], std::vector<Task<boost::shared_ptr<TransactionID>>*> &tasks, std::map<int, boost::shared_ptr<TProtocol>> &failedWorkers);

		bool FinalizeTransaction(WorkerInfo workers[], const boost::shared_ptr<SortedDictionary< boost::shared_ptr<TransactionID>, std::vector<WorkerInfo>*>> &workersByTransaction, std::map<int, boost::shared_ptr<TProtocol>> &failedWorkers);

		/// <summary> Invokes a given command against a worker. </summary>
		void WorkerInvoke(int podID, std::function<void(boost::shared_ptr<WorkerClient>)> work);

		/// <summary> Apply the writes to each worker, even if there are no modifications for that worker. </summary>
		std::vector<Task<TransactionID*>*> StartWorkerWrites(std::map<int, ColumnWrites*> &writes, const boost::shared_ptr<TransactionID> &transactionID, WorkerInfo workers[]);

		std::map<int, boost::shared_ptr<ColumnWrites>> EncodeIncludes(int columnIds[], const std::string& rowId, std::vector<std::string> row);
		std::map<int, boost::shared_ptr<ColumnWrites>> EncodeExcludes(int columnIds[], const std::string& rowId);

		boost::shared_ptr<Schema> LoadSchema();
		void BootStrapSchema();

		void InitializeInstanceFields();
	};
}
