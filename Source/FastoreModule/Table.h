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
			static std::map<std::string, std::string> fastoreTypeToSQLiteAffinity;

			static std::string SQLiteTypeToFastoreType(const std::string &SQLiteType);
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
			void begin();
			void sync();
			void commit();
			void rollback();

			//Destroy backing store
			void drop();

			//Probably a no-op for now
			void disconnect();

			//Create columns
			void create();

			//Ensure tables exists in backing store. If not, error. Also, update our columns definitions for the ids we pull.
			void connect();

			//Delete/Update/Inserts a row. Sets pRowid to the id generated (if any)
			void update(int argc, sqlite3_value **argv, sqlite3_int64 *pRowid);

			//Determines best index for table
			void bestIndex(sqlite3_index_info* info);

		private:
			void ensureTable();
			void ensureColumns();
			void parseDDL(); 
			void determineRowIDColumn();

			client::ColumnDef parseColumnDef(std::string text);

			void createColumn(client::ColumnDef& column, std::string& combinedName, RangeSet& podIds, int nextPod);

			//For use by the cursor. Depending on how SQLite is implemented this may either need to go through the transaction or around it.
			client::RangeSet getRange(client::Range& range, const boost::optional<std::string>& startId);

			void updateStats();
		};
	}
}
		
