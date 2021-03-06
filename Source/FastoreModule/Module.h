#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

//#include "../FastoreCore/safe_cast.h"
#include "../FastoreCommon/Buffer/ColumnDef.h"
#include "../FastoreCommon/Type/Standardtypes.h"
#include "../FastoreClient/Dictionary.h"
#include "../FastoreClient/Encoder.h"
#include <Schema/Dictionary.h>
#include <boost/assign/list_of.hpp>
#include "Cursor.h"
#include "Table.h"
#include "Address.h"
#include "Connection.h"
#include "Dictionary.h"
#include <sqlite3.h>
#include <assert.h>

namespace client = fastore::client;
namespace module = fastore::module;
namespace communication = fastore::communication;

using namespace std;

// For questions on this SQLite module, see SQLite virtual table documentation: http://www.sqlite.org/vtab.html
const char* const SQLITE_MODULE_NAME = "Fastore";

module::Connection* createModuleConnection(std::vector<fastore::module::Address>& addresses)
{
	return new module::Connection(addresses);
}

void createVirtualTables(module::Connection* connection, sqlite3* sqliteConnection)
{
	client::Range tableRange;
	tableRange.Ascending = true;
	tableRange.ColumnID = module::Dictionary::TableID;

	boost::optional<std::string> startId;

	while (true)
	{
		client::RangeSet result = connection->_database->getRange(module::Dictionary::TableColumns, tableRange, 500, startId);

		for(size_t i = 0; i < result.Data.size(); i++)
		{
			//NULL
			if (!result.Data[i].Values[2].__isset.value)
				throw std::runtime_error("Null value found for table definition");

			std::string ddl = result.Data[i].Values[2].value;

			std::string statement = "create table " + result.Data[i].Values[1].value + "(" + ddl + ");";

			sqlite3_exec(sqliteConnection, statement.c_str(), NULL, NULL, NULL);
		}

		if (result.Eof)
			break;
		else
			startId = result.Data[result.Data.size() - 1].ID;
	}
}

void ensureColumns(module::Connection* connection, std::vector<ColumnDef>& defs)
{
	//TODO: This is more or less a duplicate of the table create code. Refactor to reduce duplication

	 // Pull a list of candidate pods
    Range podQuery;
    podQuery.ColumnID = fastore::common::Dictionary::PodID;
    podQuery.Ascending = true;

	std::vector<ColumnID> podidv;
	podidv.push_back(fastore::common::Dictionary::PodID);

	//TODO: Gather pods - we may have more than 2000 
    auto podIds = connection->_database->getRange(podidv, podQuery, 2000);
    if (podIds.Data.size() == 0)
	{
        throw std::logic_error( "FastoreModule can't create a new table. "
								"The hive has no pods. "
								"The hive must be initialized first." );
	}

	// TODO: move computation for nextPod into DataSet class. 
    int nextPod = 0; // rand() % SAFE_CAST(int,(podIds.Data.size() - (podIds.Data.size() == 1? 0 : 1)));

    //This is so we have quick access to all the ids (for queries). Otherwise, we have to iterate the 
    //TableVar Columns and pull the id each time.
    //List<int> columnIds = new List<int>();
    for (size_t i = 0; i < defs.size(); i++)
	{
		// Attempt to find the column by name
		Range query;
		query.ColumnID = fastore::common::Dictionary::ColumnName;

		client::RangeBound bound;
		bound.Bound = defs[i].Name;
		bound.Inclusive = true;
		query.Start = bound;
		query.End = bound;

		//Just pull one row. If any exist we should verify it's the correct one.
		auto result = connection->_database->getRange(fastore::common::Dictionary::ColumnColumns, query, 1);

 		if (result.Data.size() == 0)
		{
			//TODO: Determine the storage pod - default, but let the user override -- we'll need to extend the sql to support this.
			auto podId = podIds.Data.at(nextPod++ % podIds.Data.size()).Values[0].value;


			//TODO: Make workers smart enough to create/instantiate a column within one transaction.
			//(They currently don't check for actions to perform until the end of the transaction, which means they may miss part of it currently)
			auto transaction = connection->_database->begin(true);
			transaction->include
			(
				fastore::common::Dictionary::ColumnColumns,
				client::Encoder<communication::ColumnID>::Encode(defs[i].ColumnID),
				boost::assign::list_of<std::string>
				(client::Encoder<communication::ColumnID>::Encode(defs[i].ColumnID))
				(defs[i].Name)
				(defs[i].ValueTypeName)
				(defs[i].RowIDTypeName)
				(client::Encoder<BufferType_t>::Encode(defs[i].BufferType))
				(client::Encoder<bool>::Encode(defs[i].Required))
			);

			int64_t surrogateId = connection->_generator->Generate(fastore::common::Dictionary::PodColumnPodID);
			transaction->include
			(
				fastore::common::Dictionary::PodColumnColumns,
				client::Encoder<int64_t>::Encode(surrogateId),
				boost::assign::list_of<std::string>
				(podId)
				(client::Encoder<communication::ColumnID>::Encode(defs[i].ColumnID))
			);
			transaction->commit();		
		}
		
		//TODO: if column does exist, compare values to see if it's the same definition
	}
}

void ensureTablesTable(module::Connection* connection)
{

	static const ColumnDef tableTable[] =  
	{ 
		{ module::Dictionary::TableID, "Table.ID", standardtypes::Long.Name, standardtypes::Long.Name, BufferType_t::Identity, true }, 
		{ module::Dictionary::TableName, "Table.Name", standardtypes::String.Name, standardtypes::Long.Name, BufferType_t::Unique, true },
		{ module::Dictionary::TableDDL, "Table.DDL", standardtypes::String.Name, standardtypes::Long.Name, BufferType_t::Unique, true },
		{ module::Dictionary::TableColumnTableID, "TableColumn.TableID", standardtypes::Long.Name, standardtypes::Long.Name, BufferType_t::Multi, true },
		{ module::Dictionary::TableColumnColumnID, "TableColumn.ColumnID", standardtypes::Long.Name, standardtypes::Long.Name, BufferType_t::Multi, true }
	};

	std::vector<ColumnDef> defs;
	for_each( tableTable, tableTable + sizeof(tableTable)/sizeof(tableTable[0]), 
		[&](const ColumnDef& def) 
		{
			defs.push_back(def);
		} );

	ensureColumns(connection, defs);
}

void detectExistingSchema(module::Connection* connection, sqlite3* sqliteConnection)
{
	ensureTablesTable(connection);
	createVirtualTables(connection, sqliteConnection);
}

void destroyFastoreModule(void* state)
{
	auto pConnection = (module::Connection*)state;
	delete pConnection;
}

void checkSQLiteResult(int sqliteResult, sqlite3 *sqliteConnection)
{
	// TODO: thread safety
	if (SQLITE_OK != sqliteResult)
	{
		std::runtime_error(sqlite3_errmsg(sqliteConnection));
	}
}

/**
 * http://www.sqlite.org/vtab.html
 * The virtual table implementation may pass error message text to the core 
 * by putting an error message string in zErrMsg. Space to hold this error 
 * message string must be obtained from an SQLite memory allocation function 
 * such as sqlite3_mprintf() or sqlite3_malloc(). Prior to assigning a new 
 * value to zErrMsg, the virtual table implementation must free any 
 * preexisting content of zErrMsg using sqlite3_free(). 
 */
static char * 
mprintf( char **pzErr, const char message[] ) 
{
	assert(message != NULL);
	assert(pzErr != NULL);

	sqlite3_free(*pzErr);
	*pzErr =  sqlite3_mprintf("%s", message? message : "NULL message!");

	return *pzErr;
}

int ExceptionsToError(const function<int(void)> &callback, char **pzErr)
{
	int result;
	try
	{
		result = callback();
	}
	catch (int e)
	{
		result = e;
	}
	catch (char *e)
	{
		*pzErr = mprintf(pzErr, e);
		result = SQLITE_ERROR;
	}
	catch (exception &e)
	{
		*pzErr = mprintf(pzErr, e.what());
		result = SQLITE_ERROR;
	}	
	catch (...)
	{
		// Generic error messages like this are terrible, but we don't know anything more at this time.
		*pzErr = mprintf(pzErr, "Unknown exception.");
		result = SQLITE_ERROR;
	}

	return result;
}

// This method is invoked by both Create and Connect
int moduleInit(sqlite3 *db, const char *tblName, const char* ddl)
{
	ostringstream tableDef;
	tableDef << "create virtual table " << tblName <<"(" << ddl << ")";
	sqlite3_declare_vtab(db, tableDef.str().c_str());

	return SQLITE_OK;
}

struct fastore_vtab
{
	sqlite3_vtab base;	//SQLite expecting this layout
	sqlite3 *db;
	module::Table* table;
};

struct fastore_vtab_cursor
{
	sqlite3_vtab_cursor base;
	module::Cursor* cursor;
};

fastore_vtab* tableInstantiate(sqlite3 *db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab)
{
	auto connection = (module::Connection*)pAux;
	auto tableName = argv[2];

	ostringstream ddl;
	for (int i = 3; i < argc; i++)
	{
		if (i != 3)
			ddl << ",";
		//TODO: Clean arguments. Make sure they are SQLite compatible.
		ddl << argv[i];
	}	

	auto vtab = (fastore_vtab*)sqlite3_malloc(sizeof(fastore_vtab));
	*ppVTab = &vtab->base;
	vtab->base.nRef = 0;
	vtab->base.pModule = 0;
	vtab->base.zErrMsg = 0;

	//Create a fastore table
	vtab->table = new module::Table(connection, string(tableName), ddl.str());

	moduleInit(db, tableName, ddl.str().c_str());
	return vtab;
}

// This method is called to create a new instance of a virtual table in response to a CREATE VIRTUAL TABLE statement
int moduleCreate(sqlite3 *db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char**pzErr)
{
	return ExceptionsToError
	(
		[&]() -> int
		{
			auto vtab = tableInstantiate(db, pAux, argc, argv, ppVTab);	

			//Try to create the table in Fastore
			return vtab->table->create();
		},
		pzErr
	);
}

// This method is called to create a new instance of a virtual table that connects to an existing backing store.
int moduleConnect(sqlite3 *db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char **pzErr)
{
	return ExceptionsToError
	(
		[&]() -> int
		{
			auto vtab = tableInstantiate(db, pAux, argc, argv, ppVTab);

			//Try to connect to the table in Fastore -- test to see if the columns exist and update our local columnIds
			return vtab->table->connect();	
		},
		pzErr
	); 
}

//TODO: all of these function need more detail about why they fail, and also real attempts to recover.
int moduleBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* info, double* numrows, double numIterations, uint64_t colUsed)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
		(
			[&]()->int
			{
				return v->table->bestIndex(info, numrows, numIterations, colUsed);
			},
			&v->base.zErrMsg
		);
}

int moduleDisconnect(sqlite3_vtab *pVTab)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
		(
			[&]()->int
			{
				int result = v->table->disconnect();
				delete v->table;
				return result;
			},
			&v->base.zErrMsg
		);
}

int moduleDestroy(sqlite3_vtab *pVTab)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
		(
			[&]()->int
			{
				int result = v->table->drop();
				delete v->table;
				return result;
			},
			&v->base.zErrMsg
		);
}

int moduleOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
		(
			[&]()->int
			{
				fastore_vtab_cursor* c;
				c = (fastore_vtab_cursor *) calloc(sizeof(fastore_vtab_cursor), 1);
				*ppCursor = &c->base;
				c->cursor = new module::Cursor(v->table);
				return SQLITE_OK;
			},
			&v->base.zErrMsg
		);
}

int moduleClose(sqlite3_vtab_cursor* pCursor)
{
	fastore_vtab_cursor *c = reinterpret_cast<fastore_vtab_cursor*>(pCursor);
	return ExceptionsToError
		(
			[&]()->int
			{
				delete c->cursor;
				free(c);
				return SQLITE_OK;
			},
			&(c->base.pVtab->zErrMsg)
		);
}

int moduleFilter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv)
{
		fastore_vtab_cursor *c = reinterpret_cast<fastore_vtab_cursor*>(pCursor);
		return ExceptionsToError
		(
			[&]()->int
			{
				return c->cursor->filter(idxNum, idxStr, argc, argv);
			},
			&(c->base.pVtab->zErrMsg)
		);
}

int moduleNext(sqlite3_vtab_cursor *pCursor)
{
	fastore_vtab_cursor *c = reinterpret_cast<fastore_vtab_cursor*>(pCursor);
	return ExceptionsToError
		(
			[&]()->int
			{
				return c->cursor->next();
			},
			&(c->base.pVtab->zErrMsg)
		);
}

int moduleEof(sqlite3_vtab_cursor *pCursor)
{
	fastore_vtab_cursor *c = reinterpret_cast<fastore_vtab_cursor*>(pCursor);
	return ExceptionsToError
		(
			[&]()->int
			{
				return c->cursor->eof();
			},
			&(c->base.pVtab->zErrMsg)
		);
}

int moduleColumn(sqlite3_vtab_cursor *pCursor, sqlite3_context *pContext, int index)
{
	fastore_vtab_cursor *c = reinterpret_cast<fastore_vtab_cursor*>(pCursor);
	return ExceptionsToError
		(
			[&]()->int
			{
				return c->cursor->setColumnResult(pContext, index);
			},
			&(c->base.pVtab->zErrMsg)
		);
}

int moduleRowid(sqlite3_vtab_cursor *pCursor, sqlite3_int64 *pRowid)
{
	fastore_vtab_cursor *c = reinterpret_cast<fastore_vtab_cursor*>(pCursor);
	return ExceptionsToError
		(
			[&]()->int
			{
				return c->cursor->setRowId(pRowid);
			},
			&(c->base.pVtab->zErrMsg)
		);	
}

int moduleUpdate(sqlite3_vtab *pVTab, int argc, sqlite3_value **argv, sqlite3_int64 *pRowid)
{	
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
		(
			[&]()->int
			{
				return v->table->update(argc, argv, pRowid);
			},
			&v->base.zErrMsg
		);	
}

int moduleBegin(sqlite3_vtab *pVTab)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
		(
			[&]()->int
			{
				return v->table->begin();
			},
			&v->base.zErrMsg
		);	
}

int moduleSync(sqlite3_vtab *pVTab)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
	(
		[&]()->int
		{
			return v->table->sync();
		},
		&v->base.zErrMsg
	);	
}

int moduleCommit(sqlite3_vtab *pVTab)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
	(
		[&]()->int
		{
			return v->table->commit();
		},
		&v->base.zErrMsg
	);
}

int moduleRollback(sqlite3_vtab *pVTab)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
	(
		[&]()->int
		{
			return v->table->rollback();
		},
		&v->base.zErrMsg
	);
}

int moduleFindFunction(sqlite3_vtab *pVTab, int nArg, const char *zName, void (**pxFunc)(sqlite3_context*,int,sqlite3_value**), void **ppArg)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
	(
		[&]()->int
		{
			//TODO: This is used to overload any functions.
			//Probably don't want to overload anything on V1, but we will see.
			return SQLITE_OK;
		},
		&v->base.zErrMsg
	);
}

int moduleRename(sqlite3_vtab *pVTab, const char *zNew)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
	(
		[&]()->int
		{
			//TODO: Change the name of the vtable. Return error to prevent renaming. Return success to indicate renaming was
			//successful. (disable renaming for now)
			return SQLITE_ERROR;
		},
		&v->base.zErrMsg
	);
}

//Next three are for nested transactions.. Should probably be disabled for the time being.
int moduleSavepoint(sqlite3_vtab *pVTab, int savePoint)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
	(
		[&]()->int
		{
			//current state should be saved with the ID savePoint
			return SQLITE_ERROR;
		},
		&v->base.zErrMsg
	);
}

int moduleRelease(sqlite3_vtab *pVTab, int savePoint)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
	(
		[&]()->int
		{
			//Invalidates all savepoints with N >= savePoint
			return SQLITE_ERROR;
		},
		&v->base.zErrMsg
	);
}

int moduleRollbackTo(sqlite3_vtab *pVTab, int savePoint)
{
	fastore_vtab *v = reinterpret_cast<fastore_vtab *>(pVTab);
	return ExceptionsToError
	(
		[&]()->int
		{
			//Rolls back to savePoint
			return SQLITE_ERROR;
		},
		&v->base.zErrMsg
	);
}

void moduleCheckpoint(sqlite3_context* context, int argc, sqlite3_value** argv)
{
	module::Connection* conn = (module::Connection*)sqlite3_user_data(context);
	conn->_database->checkpoint();
}

void moduleTrace(void* userdata, const char* msg)
{
	std::cout << msg << std::endl;
}

sqlite3_module fastoreModule =
{
	0,	// iVersion;
	moduleCreate,	// int (*xCreate)(sqlite3*, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char**);
	moduleConnect,	// int (*xConnect)(sqlite3*, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char**);
	moduleBestIndex,	// int (*xBestIndex)(sqlite3_vtab *pVTab, sqlite3_index_info*);
	moduleDisconnect,	// int (*xDisconnect)(sqlite3_vtab *pVTab);
	moduleDestroy,	// int (*xDestroy)(sqlite3_vtab *pVTab);
	moduleOpen,	// int (*xOpen)(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor);
	moduleClose,	// int (*xClose)(sqlite3_vtab_cursor*);
	moduleFilter,	// int (*xFilter)(sqlite3_vtab_cursor*, int idxNum, const char *idxStr, int argc, sqlite3_value **argv);
	moduleNext,	// int (*xNext)(sqlite3_vtab_cursor*);
	moduleEof,	// int (*xEof)(sqlite3_vtab_cursor*);
	moduleColumn,	// int (*xColumn)(sqlite3_vtab_cursor*, sqlite3_context*, int);
	moduleRowid,	// int (*xRowid)(sqlite3_vtab_cursor*, sqlite3_int64 *pRowid);
	moduleUpdate,	// int (*xUpdate)(sqlite3_vtab *, int, sqlite3_value **, sqlite3_int64 *);
	
	//The are called per table, so figure out how to implement them per table. Each table gets its own transaction perhaps?
	moduleBegin,	// int (*xBegin)(sqlite3_vtab *pVTab);
    0 /*moduleSync */,	// int (*xSync)(sqlite3_vtab *pVTab);
	moduleCommit,	// int (*xCommit)(sqlite3_vtab *pVTab);
	moduleRollback,	// int (*xRollback)(sqlite3_vtab *pVTab);
	0 /* moduleFindFunction */,	// int (*xFindFunction)(sqlite3_vtab *pVtab, int nArg, const char *zName, void (**pxFunc)(sqlite3_context*,int,sqlite3_value**), void **ppArg);
	moduleRename,	// int (*xRename)(sqlite3_vtab *pVtab, const char *zNew);
	
	/* v2 methods */
	0 /* moduleSavepoint */,	// int (*xSavepoint)(sqlite3_vtab *pVTab, int);
	0 /* moduleRelease */,	// int (*xRelease)(sqlite3_vtab *pVTab, int);
	0 /* moduleRollbackTo */,	// int (*xRollbackTo)(sqlite3_vtab *pVTab, int);
};

void intializeFastoreModule(sqlite3* db, std::vector<module::Address> addresses)
{
	module::Connection* conn = createModuleConnection( addresses);
	sqlite3_create_module_v2(db, SQLITE_MODULE_NAME, &fastoreModule, conn, &destroyFastoreModule);
	sqlite3_create_function_v2(db, "CHECKPOINT", 0, SQLITE_ANY, conn, &moduleCheckpoint, NULL, NULL, NULL);
	//sqlite3_trace(db, &moduleTrace, NULL);
	detectExistingSchema(conn, db);
}



#pragma GCC diagnostic pop
