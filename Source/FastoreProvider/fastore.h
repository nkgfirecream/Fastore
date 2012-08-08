#pragma once

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
const int MAX_ERROR_MESSAGE = 255;
const int MAX_NAME = 127;

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

enum TransactionEndAction 
{
	FASTORE_TRANSACTION_COMMIT, 
	FASTORE_TRANSACTION_ROLLBACK,
	FASTORE_TRANSACTION_COMMIT_FLUSH 
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

// All result types can be cast to this type for generic error handling
struct FastoreResult
{
	bool success;
	FastoreError error;
};

struct ConnectResult
{
	bool success;
	union
	{
		FastoreError error;
		DatabaseHandle database;
	};
};

struct BeginResult
{
	bool success;
	union
	{
		FastoreError error;
		DatabaseHandle transaction;
	};
};

struct PrepareResult
{
	bool success;
	union
	{
		FastoreError error;
		struct
		{
			CursorHandle cursor;
			int columnCount;
		};
	};
};

struct NextResult
{
	bool success;
	union
	{
		FastoreError error;
		bool eof;
	};
};

struct ExecuteResult
{
	bool success;
	union
	{
		FastoreError error;
		struct
		{
			CursorHandle cursor;
			int columnCount;
			bool eof;
		};
	};
};

struct ColumnInfoResult
{
	bool success;
	union
	{
		FastoreError error;
		struct
		{
			char name[MAX_NAME];
			char type[MAX_NAME];
		};
	};
};

// Creates a new database connection
FASTOREAPI ConnectResult APIENTRY fastoreConnect(int addressCount, const struct FastoreAddress addresses[]);
// Dereferences the given database connection; the connection may remain open if any transactions are still open on it
FASTOREAPI FastoreResult APIENTRY fastoreDisconnect(DatabaseHandle database);

// Begins a transaction against the given database.  The given handle must not be a transaction.
FASTOREAPI BeginResult APIENTRY fastoreBegin(DatabaseHandle database);
// Commits or rolls back a previously began transaction
FASTOREAPI FastoreResult APIENTRY fastoreEnd(DatabaseHandle database, TransactionEndAction action);

// Prepares a given query or statement statement and returns a cursor
FASTOREAPI PrepareResult APIENTRY fastorePrepare(DatabaseHandle database, const char *sql);
// Provides values for any parameters included in the prepared statement and resets the cursor
FASTOREAPI FastoreResult APIENTRY fastoreBind(CursorHandle cursor, int argumentCount, void *arguments, const ArgumentType ArgumentType[]);
// Executes the statement, or navigates to the first or next row
FASTOREAPI NextResult APIENTRY fastoreNext(CursorHandle cursor);
// Gets the column name for the given column index
FASTOREAPI ColumnInfoResult APIENTRY fastoreColumnInfo(CursorHandle cursor, int columnIndex);
// Gets the column value of the current row given an index
FASTOREAPI FastoreResult APIENTRY fastoreColumnValue(CursorHandle cursor, int columnIndex, int targetMaxBytes, void *valueTarget);
// Closes the given cursor
FASTOREAPI FastoreResult APIENTRY fastoreClose(CursorHandle cursor);

// Short-hand for Prepare followed by Next... then close if eof.
FASTOREAPI ExecuteResult APIENTRY fastoreExecute(DatabaseHandle database, const char *sql);