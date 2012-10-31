#pragma once

#include "stdafx.h"
#include <stdint.h>
#include <cstdlib>

#if defined(_WIN32)
// Import or export appropriately
# if defined(FASTORE_EXPORT) 
#   define FASTOREAPI extern "C" // __declspec(dllexport)
# else
#   define FASTOREAPI extern "C" __declspec(dllimport)
# endif  
#else
#   define FASTOREAPI
#   define APIENTRY
#endif

typedef void *ConnectionHandle;
typedef void *StatementHandle;

const int32_t MAX_HOST_NAME = 255;
const int32_t MAX_ERROR_MESSAGE = 255;
const int32_t MAX_NAME = 127;

typedef enum 
{
	FASTORE_ARGUMENT_NULL = 0,
	FASTORE_ARGUMENT_DOUBLE = 1,
	FASTORE_ARGUMENT_INT32 = 2,
	FASTORE_ARGUMENT_INT64 = 3,
	FASTORE_ARGUMENT_STRING8 = 4,
	FASTORE_ARGUMENT_STRING16 = 5,
	FASTORE_ARGUMENT_BOOL = 6
} ArgumentType;

typedef struct 
{
	char hostName[MAX_HOST_NAME];
	int64_t port;
} FastoreAddress;

typedef int32_t FastoreResultCode;
typedef int32_t FastoreErrorCode;

const FastoreResultCode FASTORE_OK = 0;

typedef struct 
{
	FastoreResultCode result;
} GeneralResult;

typedef struct 
{
	FastoreResultCode result;
	ConnectionHandle connection;
} ConnectResult;

typedef struct 
{
	FastoreResultCode result;
	StatementHandle statement;
	int32_t columnCount;
	uint8_t eof;
} ExecuteResult;

typedef struct 
{
	FastoreResultCode result;
	StatementHandle statement;
	int32_t columnCount;
} PrepareResult;

typedef struct 
{
	FastoreResultCode result;
	uint8_t eof;
} NextResult;

typedef struct 
{
	FastoreResultCode result;
	char name[MAX_NAME];
	ArgumentType type;
} ColumnInfoResult;

typedef struct 
{
	FastoreResultCode result;
	uint8_t isNull;
	int64_t value;
} ColumnValueInt64Result;

typedef struct 
{
	FastoreResultCode result;
	uint8_t isNull;
	double value;
} ColumnValueDoubleResult;

typedef struct 
{
	FastoreResultCode result;
	uint8_t isNull;
} ColumnValueStringResult;

// Retrieves the message and code of the last error
FASTOREAPI bool fastoreGetLastError(const FastoreResultCode result, size_t messageMaxLength, char* message, FastoreErrorCode *code);

// Creates a new database connection
FASTOREAPI ConnectResult fastoreConnect(size_t addressCount, const FastoreAddress addresses[]);
// Dereferences the given database connection; the connection may remain open if any transactions are still open on it
FASTOREAPI GeneralResult fastoreDisconnect(ConnectionHandle connection);

// Prepares a given query or statement statement and returns a cursor
FASTOREAPI PrepareResult fastorePrepare(ConnectionHandle connection, const char *batch);
// Resets the cursor to the BOF crack
FASTOREAPI GeneralResult fastoreReset(StatementHandle statement);
// Provides values for any parameters included in the prepared statement and resets the cursor
FASTOREAPI GeneralResult fastoreBindInt64(StatementHandle statement, int32_t argumentIndex, int64_t value);
FASTOREAPI GeneralResult fastoreBindDouble(StatementHandle statement, int32_t argumentIndex, double value);
FASTOREAPI GeneralResult fastoreBindAString(StatementHandle statement, int32_t argumentIndex, const char *value);
FASTOREAPI GeneralResult fastoreBindWString(StatementHandle statement, int32_t argumentIndex, const wchar_t *value);
// Executes the statement, or navigates to the first or next row
FASTOREAPI NextResult fastoreNext(StatementHandle statement);
// Gets the column name for the given column index
FASTOREAPI ColumnInfoResult fastoreColumnInfo(StatementHandle statement, int32_t columnIndex);
// Gets the column value of the current row given an index
FASTOREAPI ColumnValueInt64Result fastoreColumnValueInt64(StatementHandle statement, int32_t columnIndex);
FASTOREAPI ColumnValueDoubleResult fastoreColumnValueDouble(StatementHandle statement, int32_t columnIndex);
FASTOREAPI ColumnValueStringResult fastoreColumnValueAString(StatementHandle statement, int32_t columnIndex, int32_t *targetMaxBytes, char *valueTarget);
FASTOREAPI ColumnValueStringResult fastoreColumnValueWString(StatementHandle statement, int32_t columnIndex, int32_t *targetMaxBytes, wchar_t *valueTarget);
// Closes the given cursor
FASTOREAPI GeneralResult fastoreClose(StatementHandle statement);

// Short-hand for Prepare followed by Next... then close if eof or no columns.
FASTOREAPI ExecuteResult fastoreExecute(ConnectionHandle connection, const char *batch);
