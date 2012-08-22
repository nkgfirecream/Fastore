#pragma once

#include "stdafx.h"
#include "ArgumentType.h"

// Import or export appropriately
#if defined(FASTORE_EXPORT) 
#   define FASTOREAPI __declspec(dllexport)
#else
#   define FASTOREAPI __declspec(dllimport)
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
		ConnectionHandle connection;
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
			StatementHandle statement;
			int columnCount;
			bool eof;
		};
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
			StatementHandle statement;
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
FASTOREAPI FastoreResult APIENTRY fastoreDisconnect(ConnectionHandle connection);

// Prepares a given query or statement statement and returns a cursor
FASTOREAPI PrepareResult APIENTRY fastorePrepare(ConnectionHandle database, const char *sql);
// Provides values for any parameters included in the prepared statement and resets the cursor
FASTOREAPI FastoreResult APIENTRY fastoreBind(StatementHandle statement, int argumentCount, void *arguments, const fastore::provider::ArgumentType ArgumentType[]);
// Executes the statement, or navigates to the first or next row
FASTOREAPI NextResult APIENTRY fastoreNext(StatementHandle statement);
// Gets the column name for the given column index
FASTOREAPI ColumnInfoResult APIENTRY fastoreColumnInfo(StatementHandle statement, int columnIndex);
// Gets the column value of the current row given an index
FASTOREAPI FastoreResult APIENTRY fastoreColumnValue(StatementHandle statement, int columnIndex, int targetMaxBytes, void *valueTarget);
// Closes the given cursor
FASTOREAPI FastoreResult APIENTRY fastoreClose(StatementHandle statement);

// Short-hand for Prepare followed by Next... then close if eof.
FASTOREAPI ExecuteResult APIENTRY fastoreExecute(ConnectionHandle connection, const char *sql);