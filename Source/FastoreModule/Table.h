#pragma once
#include <sqlite3.h>
#include "Connection.h"
#include "../FastoreCommon/Buffer/ColumnDef.h"
#include "../FastoreCommon/Utility/LexCompare.h"
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
			
			static std::map<std::string, std::string, LexCompare> declaredTypeToFastoreType;
			static std::map<std::string, int, LexCompare>  declaredTypeToSQLiteTypeID;

			static void EnsureFastoreTypeMaps();

			const static int MAXTABLEOPERATIONS = 10000;
			const static int ROWSPERQUERY = 1000;
			const static int QUERYOVERHEAD = 10000;
			const static int NOINDEXPENALTY = 2;

			boost::shared_ptr<client::Transaction> _transaction;
			Connection* _connection;
			std::string _name;
			std::string _ddl;
			int64_t _id;
			int _numoperations;

			//These are all structures to manage columns. Can we combine to reduce management?
			std::vector<std::string> _declaredTypes;
			std::vector<ColumnDef> _columns;
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
			int bestIndex(sqlite3_index_info* info, double* numRows, double numIterations, uint64_t colUsed);

		private:
			void ensureTable();
			void ensureColumns();
			void parseDDL(); 
			void determineRowIDColumn();
			int64_t maxColTot();

			ColumnDef parseColumnDef(std::string text, bool& isDef);
			int convertValues(sqlite3_value **argv, communication::ColumnIDs& columns, std::vector<std::string>& row);
			void encodeSQLiteValue(sqlite3_value* pValue, int type, std::string& out);
			int tryConvertValue(sqlite3_value* pValue, std::string& declaredType, std::string& out);
			double costPerRow(int columnIndex);

			void createColumn(ColumnDef& column, std::string& combinedName, RangeSet& podIds, std::string& podId);

			//For use by the cursor. Depending on how SQLite is implemented this may either need to go through the transaction or around it.
			client::RangeSet getRange(client::Range& range, const ColumnIDs& columnIds, const boost::optional<std::string>& startId);

			void updateStats();
		};
	}
}
		
