#pragma once

#include "stdafx.h"
#include <stdint.h>
#include <cstdlib>

#if defined(_WIN32)
// Import or export appropriately
# if defined(FASTORE_EXPORT) 
#   define FASTOREAPI extern "C" __declspec(dllexport)
# else
#   define FASTOREAPI extern "C" __declspec(dllimport)
# endif  
#else
#   define FASTOREAPI
#   define APIENTRY
#endif

typedef void *ConnectionHandle;
typedef void *StatementHandle;

const int MAX_HOST_NAME = 255;
const int MAX_ERROR_MESSAGE = 255;
const int MAX_NAME = 127;

enum TransactionEndAction 
{
	FASTORE_TRANSACTION_COMMIT, 
	FASTORE_TRANSACTION_ROLLBACK,
	FASTORE_TRANSACTION_COMMIT_FLUSH 
};

enum ArgumentType
{
	FASTORE_ARGUMENT_NULL,
	FASTORE_ARGUMENT_DOUBLE,
	FASTORE_ARGUMENT_INT32,
	FASTORE_ARGUMENT_INT64,
	FASTORE_ARGUMENT_STRING8,
	FASTORE_ARGUMENT_STRING16,
	FASTORE_ARGUMENT_BOOL
};

struct FastoreAddress
{
	char hostName[MAX_HOST_NAME];
	uint64_t port;
};

typedef int32_t FastoreResultCode;
typedef int32_t FastoreErrorCode;

const FastoreResultCode FASTORE_OK = {0};

struct GeneralResult
{
	FastoreResultCode result;
};

struct ConnectResult
{
	FastoreResultCode result;
	ConnectionHandle connection;
};

struct ExecuteResult
{
	FastoreResultCode result;
	StatementHandle statement;
	int columnCount;
	bool eof;
};

struct PrepareResult
{
	FastoreResultCode result;
	StatementHandle statement;
	int columnCount;
};

struct NextResult
{
	FastoreResultCode result;
	bool eof;
};

struct ColumnInfoResult
{
	FastoreResultCode result;
	char name[MAX_NAME];
	char type[MAX_NAME];
};

// Retrieves the message and code of the last error
FASTOREAPI bool fastoreGetLastError(const FastoreResultCode result, size_t messageMaxLength, char* message, FastoreErrorCode *code);

// Creates a new database connection
FASTOREAPI ConnectResult fastoreConnect(size_t addressCount, const struct FastoreAddress addresses[]);
// Dereferences the given database connection; the connection may remain open if any transactions are still open on it
FASTOREAPI GeneralResult fastoreDisconnect(ConnectionHandle connection);

// Prepares a given query or statement statement and returns a cursor
FASTOREAPI PrepareResult fastorePrepare(ConnectionHandle database, const char *sql);
// Provides values for any parameters included in the prepared statement and resets the cursor
FASTOREAPI GeneralResult fastoreBind(StatementHandle statement, size_t argumentCount, void *arguments, const ArgumentType ArgumentType[]);
// Executes the statement, or navigates to the first or next row
FASTOREAPI NextResult fastoreNext(StatementHandle statement);
// Gets the column name for the given column index
FASTOREAPI ColumnInfoResult fastoreColumnInfo(StatementHandle statement, int columnIndex);
// Gets the column value of the current row given an index
FASTOREAPI GeneralResult fastoreColumnValue(StatementHandle statement, int columnIndex, int targetMaxBytes, void *valueTarget);
// Closes the given cursor
FASTOREAPI GeneralResult fastoreClose(StatementHandle statement);

// Short-hand for Prepare followed by Next... then close if eof.
FASTOREAPI ExecuteResult fastoreExecute(ConnectionHandle connection, const char *sql);
