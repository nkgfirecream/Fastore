﻿#pragma once

#include "IDataAccess.h"
#include "Database.h"
#include "DataSet.h"
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "..\FastoreCommunication\Comm_types.h"

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
			std::vector<fastore::communication::Include> Includes;
			std::vector<fastore::communication::Exclude> Excludes;
		};

	private:
		boost::shared_ptr<Database> privateDatabase;
		bool privateReadIsolation;
		bool privateWriteIsolation;
		bool _completed;
		TransactionID _transactionId;
		std::map<int, ColumnWrites> GatherWrites();

		// Log entries - by column ID then by row ID - null value means exclude
		std::map<int, LogColumn> _log;
		LogColumn EnsureColumnLog(int columnId);

	public:
		const Database &getDatabase() const;
		void setDatabase(const Database &value);
		const bool &getReadIsolation() const;
		void setReadIsolation(const bool &value);
		const bool &getWriteIsolation() const;
		void setWriteIsolation(const bool &value);

		Transaction(const Database &database, bool readIsolation, bool writeIsolation);
		~Transaction();

		void Commit(bool flush = false);
		void Rollback();

		RangeSet GetRange(std::vector<int>& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId);
		DataSet GetValues(const std::vector<int>& columnIds, const std::vector<std::string>& rowIds);

		void Include(const std::vector<int>& columnIds, const std::string& rowId, const std::vector<std::string>& row);
		void Exclude(const std::vector<int>& columnIds, const std::string& rowId);

		std::vector<Statistic> GetStatistics(const std::vector<int>& columnIds);
		std::map<int, long long> Ping();
	};
}}
