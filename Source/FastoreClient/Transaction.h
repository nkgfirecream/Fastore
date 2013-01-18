#pragma once

#include "IDataAccess.h"
#include "Database.h"
#include "DataSet.h"
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <Communication/Comm_types.h>
#include <Buffer/TreeBuffer.h>
#include <unordered_set>
#include <unordered_map>
#include <Buffer/BufferFactory.h>

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
	public:
		class LogColumn
		{
		public:

			// Included rows by value
			std::unique_ptr<IColumnBuffer> includes;
			// Excluded row IDs
			std::unordered_set<std::string> excludes;
			// Read row IDs
			std::unordered_set<std::string> reads;
			// Isolated Revision (0 reserved for unknown)
			Revision revision;
			

			LogColumn(const std::string& rowTypeName, const std::string& valueTypeName) 
				: revision(0)
			{
				includes = BufferFactory::CreateBuffer(rowTypeName, valueTypeName, BufferType_t::Multi);
			}

			LogColumn(LogColumn&& other)
				: revision(other.revision), reads(std::move(other.reads)), excludes(std::move(other.excludes)), includes(std::move(other.includes))
			{ }
		};

	private:
		Database& _database;
		bool _completed;
		TransactionID _transactionId;
		// Log entries - by column ID then by row ID - null value means exclude
		std::unordered_map<ColumnID, LogColumn> _log;
		bool _readsConflict;

		void gatherWrites(std::map<ColumnID,  ColumnWrites>& output);
		LogColumn& ensureColumnLog(const ColumnID& columnId);
		// Build a log entry per column for quick access
		std::vector<Transaction::LogColumn*> buildLogMap(const ColumnIDs& columnIds, bool &anyMapped);
		void applyColumnOverrides(LogColumn &colLog, DataSetRow &row, size_t colIndex);
	public:
		const Database &getDatabase() const;
		
		Transaction(Database& database, bool readsConflict);
		~Transaction();

		void commit(bool flush = false);
		void rollback();

		RangeSet getRange(const ColumnIDs& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId = boost::optional<std::string>());
		DataSet getValues(const ColumnIDs& columnIds, const std::vector<std::string>& rowIds);

		void include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row);
		void exclude(const ColumnIDs& columnIds, const std::string& rowId);

		std::vector<Statistic> getStatistics(const ColumnIDs& columnIds);
		std::map<HostID, long long> ping();
	};
}}
