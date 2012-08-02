#pragma once

#include "IDataAccess.h"
#include "Database.h"
#include "DataSet.h"
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "..\FastoreCommunication\Comm_types.h"
#include <hash_set>

using namespace fastore::communication;

namespace fastore { namespace client
{
	class Database;


	/// <Remarks>
	///   If a transaction is disposed and hasn't been committed or rolled back then it is automatically rolled back.
	///   Once a transaction has been committed or rolled back then it is in a new transaction state again and can be used as if it were new again.
	/// </Remarks>
	class Transaction : public IDataAccess
	{
	private:
		class LogColumn
		{
		public:
			std::map<std::string, std::string> Includes;
			std::hash_set<std::string> Excludes;
		};

	private:
		Database& privateDatabase;
		bool privateReadIsolation;
		bool privateWriteIsolation;
		bool _completed;
		TransactionID _transactionId;
		std::map<int, ColumnWrites> GatherWrites();

		// Log entries - by column ID then by row ID - null value means exclude
		std::map<ColumnID, LogColumn> _log;
		LogColumn EnsureColumnLog(const ColumnID& columnId);

	public:
		const Database &getDatabase() const;
		
		const bool &getReadIsolation() const;
	
		const bool &getWriteIsolation() const;

		Transaction(Database& database, bool readIsolation, bool writeIsolation);
		~Transaction();

		void Commit(bool flush = false);
		void Rollback();

		RangeSet GetRange(const ColumnIDs& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId = boost::optional<std::string>());
		DataSet GetValues(const ColumnIDs& columnIds, const std::vector<std::string>& rowIds);

		void Include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row);
		void Exclude(const ColumnIDs& columnIds, const std::string& rowId);

		std::vector<Statistic> GetStatistics(const ColumnIDs& columnIds);
		std::map<int, long long> Ping();
	};
}}
