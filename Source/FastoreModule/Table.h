#pragma once
#include <sqlite3.h>
#include "Connection.h"
#include "..\FastoreClient\ColumnDef.h"

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
			boost::shared_ptr<client::Transaction> _transaction;
			Connection* _connection;
			std::string _name;
			std::vector<client::ColumnDef> _columns;
			std::vector<communication::ColumnID> _columnIds;

		public:
			Table(Connection* connection, std::string& name, std::vector<client::ColumnDef>& columns);

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
			void createColumn(client::ColumnDef& column, std::string& combinedName, client::ColumnDef& rowIDColumn, RangeSet& podIds, int nextPod);

			//For use by the cursor. Depending on how SQLite is implemented this may either need to go through the transaction or around it.
			client::RangeSet getRange(client::Range& range, boost::optional<std::string>& startId);
		};
	}
}
		