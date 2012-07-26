#include "stdafx.h"

// Import or export appropriately
#if defined(FASTORE_EXPORT) 
#   define FASTOREAPI __declspec(dllexport)
#else
#   define FASTOREAPI __declspec(dllimport)
#endif  

typedef void *DatabaseHandle;
typedef void *StatementHandle;
typedef void *CursorHandle;

const int MAX_HOST_NAME = 255;
const int MAX_ERROR_MESSAGE = 512;

enum ArgumentTypes
{
	FASTORE_ARGUMENT_DOUBLE,
	FASTORE_ARGUMENT_INT32,
	FASTORE_ARGUMENT_INT64,
	FASTORE_ARGUMENT_NULL,
	FASTORE_ARGUMENT_STRING8,
	FASTORE_ARGUMENT_STRING16
};

struct FastoreAddress
{
	char hostName[MAX_HOST_NAME];
	int port;
};

struct FastoreError
{
	char message[MAX_ERROR_MESSAGE];
	int code;
};

typedef FastoreError * PFastoreError;

// Any result structure can be cast to a FastoreResult for generic error processing
struct FastoreResult
{
	PFastoreError error;
};

struct ConnectResult
{
	PFastoreError error;
	DatabaseHandle database;
};

struct BeginResult
{
	PFastoreError error;
	DatabaseHandle transaction;
};

struct PrepareResult
{
	PFastoreError error;
	CursorHandle cursor;
	int columnCount;
};

struct NextResult
{
	PFastoreError error;
	bool eof;
};

struct ExecuteResult
{
	PFastoreError error;
	CursorHandle cursor;
	int columnCount;
	bool eof;
};


FASTOREAPI ConnectResult APIENTRY fastoreConnect(int addressCount, FastoreAddress *addresses);
FASTOREAPI PFastoreError APIENTRY fastoreDisconnect(DatabaseHandle database);

FASTOREAPI BeginResult APIENTRY fastoreBegin(DatabaseHandle database);
FASTOREAPI PFastoreError APIENTRY fastoreCommit(DatabaseHandle database, bool force = false);
FASTOREAPI PFastoreError APIENTRY fastoreRollback(DatabaseHandle database);

FASTOREAPI PrepareResult APIENTRY fastorePrepare(const char *sql);
FASTOREAPI PFastoreError APIENTRY fastoreBind(CursorHandle cursor, int argumentCount, void *arguments, ArgumentTypes *argumentTypes);
FASTOREAPI NextResult APIENTRY fastoreNext(CursorHandle cursor);
FASTOREAPI PFastoreError APIENTRY fastoreColumnName(CursorHandle cursor, int columnIndex, int targetMaxBytes, char *nameTarget);
FASTOREAPI PFastoreError APIENTRY fastoreColumnValue(CursorHandle cursor, int columnIndex, int targetMaxBytes, void *valueTarget);
FASTOREAPI PFastoreError APIENTRY fastoreClose(CursorHandle cursor);

// Short-hand for Prepare followed by Next... then close if eof)
FASTOREAPI inline ExecuteResult APIENTRY fastoreExecute(const char *sql);