#pragma once
#include <sqlite3.h>
#include "Connection.h"
#include "../FastoreClient/ColumnDef.h"
#include <map>

namespace client = fastore::client;
namespace communication = fastore::communication;

namespace fastore
{
	namespace module
	{
		class Cursor;

		class Table
		{
			friend class Cursor;

		private:
			
			static std::map<std::string, std::string> sqliteTypesToFastoreTypes;
			static std::map<int, std::string> sqliteTypeIDToFastoreTypes;
			static std::map<std::string, int> fastoreTypeToSQLiteTypeID;

			static std::string SQLiteTypeToFastoreType(const std::string &SQLiteType);
			static int FastoreTypeToSQLiteTypeID(const std::string &fastoreType);
			static void EnsureFastoreTypeMaps();

			const static int MAXTABLEOPERATIONS = 10000;

			boost::shared_ptr<client::Transaction> _transaction;
			Connection* _connection;
			std::string _name;
			std::string _ddl;
			int64_t _id;
			int _numoperations;
			std::vector<client::ColumnDef> _columns;
			std::vector<communication::ColumnID> _columnIds;
			std::vector<communication::Statistic> _stats;
			int64_t _rowIDIndex;

		public:
			Table(Connection* connection, const std::string& name, const std::string& ddl);

			//Transaction processing...
			int begin();
			int sync();
			int commit();
			int rollback();

			//Destroy backing store
			int drop();

			//Probably a no-op for now
			int disconnect();

			//Create columns
			int create();

			//Ensure tables exists in backing store. If not, error. Also, update our columns definitions for the ids we pull.
			int connect();

			//Delete/Update/Inserts a row. Sets pRowid to the id generated (if any)
			int update(int argc, sqlite3_value **argv, sqlite3_int64 *pRowid);

			//Determines best index for table
			int bestIndex(sqlite3_index_info* info);

		private:
			void ensureTable();
			void ensureColumns();
			void parseDDL(); 
			void determineRowIDColumn();

			client::ColumnDef parseColumnDef(std::string text, bool& isDef);

			void createColumn(client::ColumnDef& column, std::string& combinedName, RangeSet& podIds, int nextPod);

			//For use by the cursor. Depending on how SQLite is implemented this may either need to go through the transaction or around it.
			client::RangeSet getRange(client::Range& range, const boost::optional<std::string>& startId);

			void updateStats();
		};
	}
}
		
