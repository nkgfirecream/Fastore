#include "fastore.h"

FASTOREAPI ConnectResult APIENTRY fastoreConnect(int addressCount, FastoreAddress *addresses)
{
	ConnectResult result;
	return result;
}

FASTOREAPI PFastoreError APIENTRY fastoreDisconnect(DatabaseHandle database)
{
	return NULL;
}

FASTOREAPI BeginResult APIENTRY fastoreBegin(DatabaseHandle database)
{
	BeginResult result;
	return result;
}

FASTOREAPI PFastoreError APIENTRY fastoreCommit(DatabaseHandle database, bool force)
{
	return NULL;
}

FASTOREAPI PFastoreError APIENTRY fastoreRollback(DatabaseHandle database)
{
	return NULL;
}

FASTOREAPI PrepareResult APIENTRY fastorePrepare(const char *sql)
{
	PrepareResult result;
	return result;
}

FASTOREAPI PFastoreError APIENTRY fastoreBind(CursorHandle cursor, int argumentCount, void *arguments, ArgumentTypes *argumentTypes)
{
	return NULL;
}

FASTOREAPI NextResult APIENTRY fastoreNext(CursorHandle cursor)
{
	NextResult result;
	return result;
}

FASTOREAPI PFastoreError APIENTRY fastoreColumnName(CursorHandle cursor, int columnIndex, int targetMaxBytes, char *nameTarget)
{
	return NULL;
}

FASTOREAPI PFastoreError APIENTRY fastoreColumnValue(CursorHandle cursor, int columnIndex, int targetMaxBytes, void *valueTarget)
{
	return NULL;
}

FASTOREAPI PFastoreError APIENTRY fastoreClose(CursorHandle cursor)
{
	return NULL;
}

// Short-hand for Prepare followed by Next (and a close if eof)
FASTOREAPI inline ExecuteResult APIENTRY fastoreExecute(const char *sql)
{
	PrepareResult prepareResult = fastorePrepare(sql);
	if (prepareResult.error)
	{
		ExecuteResult result =
		{
			prepareResult.error,
			NULL,
			0,
			true
		};
		return result;
	}

	NextResult nextResult = fastoreNext(prepareResult.cursor);
	if (nextResult.error || nextResult.eof)
	{
		fastoreClose(prepareResult.cursor);	// Ignore any close error so as not to lose original error
		ExecuteResult result =
		{
			nextResult.error,
			NULL,
			0,
			true
		};
		return result;
	}

	// Success and cursor left open
	ExecuteResult result =
	{
		nextResult.error,
		prepareResult.cursor,
		prepareResult.columnCount,
		nextResult.eof
	};
	return result;
};

