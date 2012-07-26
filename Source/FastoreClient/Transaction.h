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

using namespace fastore::communication;

namespace fastore
{
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
		boost::shared_ptr<TransactionID> _transactionId;
		std::map<int, boost::shared_ptr<ColumnWrites>> GatherWrites();

		// Log entries - by column ID then by row ID - null value means exclude
		std::map<int, LogColumn*> _log;
		boost::shared_ptr<LogColumn> EnsureColumnLog(int columnId);

	public:
		const boost::shared_ptr<Database> &getDatabase() const;
		void setDatabase(const boost::shared_ptr<Database> &value);
		const bool &getReadIsolation() const;
		void setReadIsolation(const bool &value);
		const bool &getWriteIsolation() const;
		void setWriteIsolation(const bool &value);

		Transaction(const boost::shared_ptr<Database> &database, bool readIsolation, bool writeIsolation);
		~Transaction();

		void Commit(bool flush = false);
		void Rollback();

		RangeResult GetRange(RangeRequest range);
		std::vector<std::string> GetValues(std::vector<int> columnIds, std::vector<std::string> rowIds);
		void Include(std::vector<int> columnIds, std::string rowId, std::vector<std::string> row);
		void Exclude(std::vector<int> columnIds, std::string rowId);
		std::vector<Statistic> GetStatistics(std::vector<int> columnIds);

		//std::map<int, TimeSpan> Ping();
	};
}
