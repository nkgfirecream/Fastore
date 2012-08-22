#include <sqlite3.h>
#include <vector>
#include <memory>
#include <exception>
#include <sstream>
#include <map>
#include "..\FastoreClient\ColumnDef.h"
#include "Cursor.h"
#include "Table.h"
#include "Connection.h"


namespace client = fastore::client;
namespace module = fastore::module;
namespace provider = fastore::provider;

using namespace std;

// For questions on this SQLite module, see SQLite virtual table documentation: http://www.sqlite.org/vtab.html
const char* const SQLITE_MODULE_NAME = "Fastore";

void checkSQLiteResult(int sqliteResult, sqlite3 *sqliteConnection)
{
	// TODO: thread safety
	if (SQLITE_OK != sqliteResult)
	{
		throw exception(sqlite3_errmsg(sqliteConnection));
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

template<typename charT>
class CaseInsensitiveComparer
{
    const std::locale& _locale;
public:
    CaseInsensitiveComparer(const std::locale& loc) : _locale(loc) {}
    bool operator()(charT ch1, charT ch2) 
	{
        return std::toupper(ch1, _locale) == std::toupper(ch2, _locale);
    }
};

template<typename T>
int insensitiveStrPos(const T& str1, const T& str2, const std::locale& locale = std::locale() )
{
    T::const_iterator it = std::search(str1.begin(), str1.end(), str2.begin(), str2.end(), CaseInsensitiveComparer<T::value_type>(locale));
    if (it != str1.end()) 
		return it - str1.begin();
    else 
		return -1;
}

client::ColumnDef ParseColumnDef(string text)
{
	client::ColumnDef result;
	istringstream reader(text, istringstream::in);
	if (!std::getline(reader, result.Name, ' ')) 
		throw exception("Missing column name");
	if (!std::getline(reader, result.Type))
		result.Type = "String";
	auto stringText = string(text);
	//TODO: When should we use an identity buffer? Primary key?
	result.BufferType =	insensitiveStrPos(stringText, string("primary")) >= 0 ? client::BufferType::Identity : insensitiveStrPos(stringText, string("unique")) >= 0 ? client::BufferType::Unique : client::BufferType::Multi;
	result.Required = insensitiveStrPos(stringText, string("not null")) >= 0 || insensitiveStrPos(stringText, string("primary")) >= 0;

	return result;
}

static map<string, string> fastoreTypesToSQLiteTypes;
static map<string, string> sqliteTypesToFastoreTypes;
static map<int, string> sqliteTypeIDToFastoreTypes;

void EnsureFastoreTypeMaps()
{
	if (fastoreTypesToSQLiteTypes.size() == 0)
	{
		fastoreTypesToSQLiteTypes["WString"] = "nvarchar";
		fastoreTypesToSQLiteTypes["String"] = "varchar";
		fastoreTypesToSQLiteTypes["Int"] = "int";
		fastoreTypesToSQLiteTypes["Long"] = "bigint";
		fastoreTypesToSQLiteTypes["Bool"] = "int";
	}

	if (sqliteTypesToFastoreTypes.size() == 0)
	{
		sqliteTypesToFastoreTypes["nvarchar"] = "WString";
		sqliteTypesToFastoreTypes["varchar"] = "String";
		sqliteTypesToFastoreTypes["int"] = "Int";
		sqliteTypesToFastoreTypes["bigint"] = "Long";
		//TODO: Must add special handling for bool. Either don't use it in Fastore, or check column def.
		//(actually, can we even define a bool column in sqlite?)
		//sqliteTypesToFastoreTypes["int"] = "Bool";
	}

	if (sqliteTypesToFastoreTypes.size() == 0)
	{
		sqliteTypeIDToFastoreTypes[SQLITE_INTEGER] = "Long";
		sqliteTypeIDToFastoreTypes[SQLITE_TEXT] = "String";
	}
}

string FastoreTypeToSQLiteType(const string &fastoreType)
{
	EnsureFastoreTypeMaps();
	auto result = fastoreTypesToSQLiteTypes.find(fastoreType);
	if (result == fastoreTypesToSQLiteTypes.end())
	{
		ostringstream message;
		message << "Unknown type '" << fastoreType << "'.";
		throw exception(message.str().c_str());
	}
	return result->second;
}

string SQLiteTypeToFastoreType(const string &SQLiteType)
{
	EnsureFastoreTypeMaps();
	auto result = sqliteTypesToFastoreTypes.find(SQLiteType);
	if (result == sqliteTypesToFastoreTypes.end())
	{
		ostringstream message;
		message << "Unknown type '" << SQLiteType << "'.";
		throw exception(message.str().c_str());
	}
	return result->second;
}

// This method is invoked by both Create and Connect
int moduleInit(sqlite3 *db, const vector<client::ColumnDef> &defs)
{
	ostringstream tableDef;
	tableDef << "create table x(";
	bool first = true;
	for (auto def : defs)
	{
		if (first)
			first = false;
		else
			tableDef << ", ";

		tableDef << def.Name << " " << FastoreTypeToSQLiteType(def.Type);
	}
	tableDef << ")";

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

// This method is called to create a new instance of a virtual table in response to a CREATE VIRTUAL TABLE statement
int moduleCreate(sqlite3 *db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char**pzErr)
{
	return ExceptionsToError
	(
		[&]() -> int
		{
			auto connection = (provider::Connection*)pAux;
			auto tableName = argv[2];

			// Parse each column into a ColumnDef
			std::vector<client::ColumnDef> defs;
			defs.reserve(argc - 4);
			string idType = "Int";
			for (int i = 4; i < argc; i++)
			{
				defs.push_back(ParseColumnDef(argv[i]));
				// TODO: track row ID type candidates
			}
			for (auto def : defs)
				def.IDType = idType;

			auto vtab = unique_ptr<fastore_vtab>((fastore_vtab *)sqlite3_malloc(sizeof(fastore_vtab)));
			*ppVTab = &vtab->base;

			//Create the table in fastore.
			vtab->table = new module::Table(connection, string(tableName), defs);

			//Try to create the table in Fastore
			vtab->table->create();

			vtab.reset();
			moduleInit(db, defs);
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
			auto connection = (provider::Connection*)pAux;
			auto tableName = argv[2];

			// Parse each column into a ColumnDef
			std::vector<client::ColumnDef> defs;
			defs.reserve(argc - 4);
			string idType = "Int";
			for (int i = 4; i < argc; i++)
			{
				defs.push_back(ParseColumnDef(argv[i]));
				// TODO: track row ID type candidates
			}
			for (auto def : defs)
				def.IDType = idType;

			auto vtab = unique_ptr<fastore_vtab>((fastore_vtab *)sqlite3_malloc(sizeof(fastore_vtab)));
			*ppVTab = &vtab->base;

			//Create the table in fastore.
			vtab->table = new module::Table(connection, string(tableName), defs);

			//Try to connect to the table in Fastore -- test to see if the columns exist and update our local columnIds
			vtab->table->connect();

			vtab.reset();
			moduleInit(db, defs);
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
	free(v);
	return SQLITE_OK;
}

int moduleDestroy(sqlite3_vtab *pVTab)
{
	fastore_vtab* v = (fastore_vtab *)pVTab;
	v->table->drop();
	delete v->table;
	free(v);
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
	if (sqlite3_value_type(argv[0]) != SQLITE_NULL)
		v->table->deleteRow(argv[0]);

	if (argc > 1)
		*pRowid = v->table->insertRow(argc, argv, pRowId);

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