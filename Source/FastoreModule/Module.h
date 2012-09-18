#pragma once
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#include "../FastoreCore/safe_cast.h"
#include "../FastoreClient/ColumnDef.h"
#include "../FastoreClient/Dictionary.h"
#include "../FastoreClient/Encoder.h"
#include <boost/assign/list_of.hpp>
#include "Cursor.h"
#include "Table.h"
#include "Address.h"
#include "Connection.h"
#include "Dictionary.h"
#include <sqlite3.h>

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
		client::RangeSet result = connection->_database->GetRange(module::Dictionary::TableColumns, tableRange, 500, startId);

		for(size_t i = 0; i < result.Data.size(); i++)
		{
			//NULL
			if (!result.Data[i].Values[2].__isset.value)
				throw "Null value found for table definition!";

			std::string ddl = result.Data[i].Values[2].value;

			std::string statement = "create virtual table " + result.Data[i].Values[1].value + " using fastore(" + ddl + ");";

			sqlite3_exec(sqliteConnection, statement.c_str(), NULL, NULL, NULL);
		}

		if (result.Eof)
			break;
		else
			startId = result.Data[result.Data.size() - 1].ID;
	}
}

void ensureColumns(module::Connection* connection, std::vector<fastore::client::ColumnDef>& defs)
{
	//TODO: This is more or less a duplicate of the table create code. Refactor to reduce duplication

	 // Pull a list of candidate pods
    Range podQuery;
    podQuery.ColumnID = client::Dictionary::PodID;
    podQuery.Ascending = true;

	std::vector<ColumnID> podidv;
	podidv.push_back(client::Dictionary::PodID);

	//TODO: Gather pods - we may have more than 2000 
    auto podIds = connection->_database->GetRange(podidv, podQuery, 2000);
    if (podIds.Data.size() == 0)
        throw "FastoreModule can't create a new table. The hive has no pods. The hive must be initialized first.";

    int nextPod = rand() % SAFE_CAST(int,(podIds.Data.size() - 1));

    //This is so we have quick access to all the ids (for queries). Otherwise, we have to iterate the 
    //TableVar Columns and pull the id each time.
    //List<int> columnIds = new List<int>();
    for (size_t i = 0; i < defs.size(); i++)
	{
		// Attempt to find the column by name
		Range query;
		query.ColumnID = client::Dictionary::ColumnName;

		client::RangeBound bound;
		bound.Bound = defs[i].Name;
		bound.Inclusive = true;
		query.Start = bound;
		query.End = bound;

		//Just pull one row. If any exist we should verify it's the correct one.
		auto result = connection->_database->GetRange(client::Dictionary::ColumnColumns, query, 1);

		if (result.Data.size() == 0)
		{
			//TODO: Determine the storage pod - default, but let the user override -- we'll need to extend the sql to support this.
			auto podId = podIds.Data[nextPod++ % podIds.Data.size()].Values[0].value;


			//TODO: Make workers smart enough to create/instantiate a column within one transaction.
			//(They currently don't check for actions to perform until the end of the transaction, which means they may miss part of it currently)
			auto transaction = connection->_database->Begin(true, true);
			transaction->Include
			(
				Dictionary::ColumnColumns,
				client::Encoder<communication::ColumnID>::Encode(defs[i].ColumnID),
				boost::assign::list_of<std::string>
				(client::Encoder<communication::ColumnID>::Encode(defs[i].ColumnID))
				(defs[i].Name)
				(defs[i].Type)
				(defs[i].IDType)
				(client::Encoder<client::BufferType_t>::Encode(defs[i].BufferType))
				(client::Encoder<bool>::Encode(defs[i].Required))
			);
			transaction->Commit();

			transaction = connection->_database->Begin(true, true);
			transaction->Include
			(
				Dictionary::PodColumnColumns,
				client::Encoder<long long>::Encode(connection->_generator->Generate(Dictionary::PodColumnPodID)),
				boost::assign::list_of<std::string>
				(podId)
				(client::Encoder<communication::ColumnID>::Encode(defs[i].ColumnID))
			);
			transaction->Commit();		
		}
		
		//TODO: if column does exist, compare values to 
	}
}

void ensureTablesTable(module::Connection* connection)
{
	std::vector<fastore::client::ColumnDef> defs;

	//TableTable
	//Table.ID
	ColumnDef tableId;
	tableId.BufferType = BufferType_t::Identity;
	tableId.ColumnID = module::Dictionary::TableID;
	tableId.IDType = "Int";
	tableId.Name = "Table.ID";
	tableId.Required = true;
	tableId.Type = "Int";

	defs.push_back(tableId);
	
	//Table.Name
	ColumnDef tableName;
	tableName.BufferType = BufferType_t::Unique;
	tableName.ColumnID = module::Dictionary::TableName;
	tableName.IDType = "Int";
	tableName.Name = "Table.Name";
	tableName.Required = true;
	tableName.Type = "String";

	defs.push_back(tableName);

	//Table.DDL
	ColumnDef tableDDL;
	tableDDL.BufferType = BufferType_t::Unique;
	tableDDL.ColumnID = module::Dictionary::TableDDL;
	tableDDL.IDType = "Int";
	tableDDL.Name = "Table.DDL";
	tableDDL.Required = true;
	tableDDL.Type = "String";

	defs.push_back(tableDDL);
	
	//TableColumnTable
	//TableColumn.TableID
	ColumnDef tableColumnTableId;
	tableColumnTableId.BufferType = BufferType_t::Multi;
	tableColumnTableId.ColumnID = module::Dictionary::TableColumnTableID;
	tableColumnTableId.IDType = "Int";
	tableColumnTableId.Name = "TableColumn.TableID";
	tableColumnTableId.Required = true;
	tableColumnTableId.Type = "Int";

	defs.push_back(tableColumnTableId);

	//TableColumn.ColumnID
	ColumnDef tableColumnColumnId;
	tableColumnColumnId.BufferType = BufferType_t::Multi;
	tableColumnColumnId.ColumnID = module::Dictionary::TableColumnColumnID;
	tableColumnColumnId.IDType = "Int";
	tableColumnColumnId.Name = "TableColumn.ColumnID";
	tableColumnColumnId.Required = true;
	tableColumnColumnId.Type = "Int";

	defs.push_back(tableColumnColumnId);

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

int ExceptionsToError(const function<int(void)> &callback, char **pzErr)
{
	try
	{
		return callback();
	}
	catch (exception &e)
	{
		*pzErr = sqlite3_mprintf(e.what());
	}
	catch (...)
	{
		// Generic error messages like this are terrible, but we don't know anything more at this time.
		*pzErr = sqlite3_mprintf("Unknown exception.");
	}
	return SQLITE_ERROR;
}

// This method is invoked by both Create and Connect
int moduleInit(sqlite3 *db, const char *tblName, const char* ddl)
{
	ostringstream tableDef;
	tableDef << "create table " << tblName <<"(" << ddl << ")";
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
			vtab->table->create();
			return SQLITE_OK;
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
			vtab->table->connect();	
			return SQLITE_OK;
		},
		pzErr
	);
}

int moduleBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info* info)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;
	v->table->bestIndex(info);
	return SQLITE_OK;
}

int moduleDisconnect(sqlite3_vtab *pVTab)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;
	v->table->disconnect();
	delete v->table;
	//free(v);
	return SQLITE_OK;
}

int moduleDestroy(sqlite3_vtab *pVTab)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;
	v->table->drop();
	delete v->table;
	//free(v);
	return SQLITE_OK;
}

int moduleOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;

	fastore_vtab_cursor* c;
	c = (fastore_vtab_cursor *) calloc(sizeof(fastore_vtab_cursor), 1);
	*ppCursor = &c->base;
	c->cursor = new module::Cursor(v->table);
	return SQLITE_OK;
}

int moduleClose(sqlite3_vtab_cursor* sqlSur)
{
	fastore_vtab_cursor* c = (fastore_vtab_cursor*)sqlSur;
	delete c->cursor;
	free(c);
	return SQLITE_OK;
}

int moduleFilter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv)
{
	fastore_vtab_cursor* c = (fastore_vtab_cursor*)pCursor;
	c->cursor->filter(idxNum, idxStr, argc, argv);
	return SQLITE_OK;
}

int moduleNext(sqlite3_vtab_cursor *pCursor)
{
	fastore_vtab_cursor* c = (fastore_vtab_cursor*)pCursor;
	c->cursor->next();
	return SQLITE_OK;
}

int moduleEof(sqlite3_vtab_cursor *pCursor)
{
	fastore_vtab_cursor* c = (fastore_vtab_cursor*)pCursor;
	return c->cursor->eof();
}

int moduleColumn(sqlite3_vtab_cursor *pCursor, sqlite3_context *pContext, int index)
{
	fastore_vtab_cursor* c = (fastore_vtab_cursor*)pCursor;
	c->cursor->setColumnResult(pContext, index);
	return SQLITE_OK;
}

int moduleRowid(sqlite3_vtab_cursor *pCursor, sqlite3_int64 *pRowid)
{
	fastore_vtab_cursor* c = (fastore_vtab_cursor*)pCursor;
	c->cursor->setRowId(pRowid);
	return SQLITE_OK;
}

int moduleUpdate(sqlite3_vtab *pVTab, int argc, sqlite3_value **argv, sqlite3_int64 *pRowid)
{	
	fastore_vtab* v = (fastore_vtab *)pVTab;
	v->table->update(argc, argv, pRowid);
	return SQLITE_OK;
}

int moduleBegin(sqlite3_vtab *pVTab)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;
	v->table->begin();
	return SQLITE_OK;
}

int moduleSync(sqlite3_vtab *pVTab)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;
	v->table->sync();
	return SQLITE_OK;
}

int moduleCommit(sqlite3_vtab *pVTab)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;
	v->table->commit();
	return SQLITE_OK;
}

int moduleRollback(sqlite3_vtab *pVTab)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;
	v->table->rollback();
	return SQLITE_OK;
}

int moduleFindFunction(sqlite3_vtab *pVTab, int nArg, const char *zName, void (**pxFunc)(sqlite3_context*,int,sqlite3_value**), void **ppArg)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;
	//TODO: This is used to overload any functions.
	//Probably don't want to overload anything on V1, but we will see.
	return SQLITE_OK;
}

int moduleRename(sqlite3_vtab *pVTab, const char *zNew)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;
	//TODO: Change the name of the vtable. Return error to prevent renaming. Return success to indicate renaming was
	//successful. (disable renaming for now)
	return SQLITE_ERROR;
}

//Next three are for nested transactions.. Should probably be disabled for the time being.
int moduleSavepoint(sqlite3_vtab *pVTab, int savePoint)
{
	//current state should be saved with the ID savePoint
	fastore_vtab* v = (fastore_vtab *)pVTab;
	return SQLITE_OK;
}

int moduleRelease(sqlite3_vtab *pVTab, int savePoint)
{
	//Invalidates all savepoints with N >= savePoint
	fastore_vtab* v = (fastore_vtab *)pVTab;
	return SQLITE_OK;
}

int moduleRollbackTo(sqlite3_vtab *pVTab, int savePoint)
{
	//Rolls back to savePoint
	fastore_vtab* v = (fastore_vtab *)pVTab;
	return SQLITE_OK;
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
	0 /* moduleBegin */,	// int (*xBegin)(sqlite3_vtab *pVTab);
	0 /* moduleSync */,	// int (*xSync)(sqlite3_vtab *pVTab);
	0 /* moduleCommit */,	// int (*xCommit)(sqlite3_vtab *pVTab);
	0 /*moduleRollback */,	// int (*xRollback)(sqlite3_vtab *pVTab);
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
	detectExistingSchema(conn, db);
}



#pragma GCC diagnostic pop
