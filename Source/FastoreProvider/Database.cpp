#include "Database.h"
#include <vector>
#include <sstream>
#include <exception>

using namespace std;
using namespace fastore::provider;
namespace client = fastore::client;

const char* const SQLITE_MODULE_NAME = "Fastore";

void checkSQLiteResult(int sqliteResult, sqlite3 *sqliteConnection)
{
	// TODO: thread safety
	if (SQLITE_OK != sqliteResult)
	{
		throw exception(sqlite3_errmsg(sqliteConnection));
	}
}

// For questions on this SQLite module, see SQLite virtual table documentation: http://www.sqlite.org/vtab.html

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

ColumnDef ParseColumnDef(string text)
{
	ColumnDef result;
	auto reader = istringstream(text);
	if (!std::getline(reader, result.name, " ")) 
		throw exception("Missing column name");
	if (!std::getline(reader, result.dataType))
		result.dataType = "String";
	auto stringText = string(text);
	result.bufferType =
		insensitiveStrPos(stringText, string("unique")) >= 0 || insensitiveStrPos(stringText, string("primary")) >= 0 ? BufferType::Unique
			: insensitiveStrPos(stringText, string("not null")) >= 0 ? BufferType::Required
			: BufferType::Multi;
}

// This method is called to create a new instance of a virtual table in response to a CREATE VIRTUAL TABLE statement
int moduleCreate(sqlite3 *db, void *pAux, int argc, const char *const*argv, sqlite3_vtab **ppVTab, char**)
{
	auto database = (Database*)pAux;
	auto tableName = argv[2];

	// Parse each column into a ColumnDef
	vector<client::ColumnDef> defs;
	defs.reserve(argc - 4);
	string idType = "Int";
	for (int i = 4, i < argc; i++)
	{
		defs.push_back(ParseColumnDef(argv[i]));
		// TODO: track row ID type candidates
	}
	for (auto def : defs)
		def.idType = idType;

	sqlite3_declare_vtab(
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
	moduleBegin,	// int (*xBegin)(sqlite3_vtab *pVTab);
	moduleSync,	// int (*xSync)(sqlite3_vtab *pVTab);
	moduleCommit,	// int (*xCommit)(sqlite3_vtab *pVTab);
	moduleRollback,	// int (*xRollback)(sqlite3_vtab *pVTab);
	moduleFindFunction,	// int (*xFindFunction)(sqlite3_vtab *pVtab, int nArg, const char *zName, void (**pxFunc)(sqlite3_context*,int,sqlite3_value**), void **ppArg);
	moduleRename,	// int (*xRename)(sqlite3_vtab *pVtab, const char *zNew);
	
	/* v2 methods */
	moduleSavepoint,	// int (*xSavepoint)(sqlite3_vtab *pVTab, int);
	moduleRelease,	// int (*xRelease)(sqlite3_vtab *pVTab, int);
	moduleRollbackTo,	// int (*xRollbackTo)(sqlite3_vtab *pVTab, int);
};

Database::Database(vector<ServerAddress> addresses)
{
	//_client = unique_ptr<fastore::client::Database>(new fastore::Database(addresses));

	// Open and wrap the SQLite connection
	sqlite3 *sqliteConnection = nullptr;
	checkSQLiteResult(sqlite3_open(":memory:", &sqliteConnection), sqliteConnection);
	_sqliteConnection = shared_ptr<sqlite3>(sqliteConnection, sqlite3_close);

	// Register our module
	checkSQLiteResult(sqlite3_create_module_v2(sqliteConnection, SQLITE_MODULE_NAME, &fastoreModule, this, nullptr), sqliteConnection);
}

unique_ptr<Cursor> Database::prepare(const std::string &sql)
{
	return unique_ptr<Cursor>(new Cursor(this, sql));
}

unique_ptr<Transaction> Database::begin()
{
	return unique_ptr<Transaction>(new Transaction(this));
}
