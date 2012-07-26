#include "fastore.h"
#include <memory>
#include "Connection.h"
#include <exception>

using namespace std;

PFastoreError ExceptionToFastoreError(exception e)
{
	auto error = unique_ptr<FastoreError>(new FastoreError());
	// TODO: error codes
	//error->code = ;
	strcpy_s(error->message, MAX_ERROR_MESSAGE, e.what());
	return error.release();
}

FASTOREAPI void APIENTRY fastoreFreeError(PFastoreError error)
{
	if (error)
		delete error;
}

FASTOREAPI ConnectResult APIENTRY fastoreConnect(int addressCount, FastoreAddress *addresses)
{
	try
	{
		// Convert addresses
		vector<ServerAddress> serverAddresses = vector<ServerAddress>();
		serverAddresses.resize(addressCount);
		for (auto i = 0; i < addressCount; i++)
		{
			serverAddresses[i].hostName	= string(addresses[i].hostName);
			serverAddresses[i].port = addresses[i].port;
		}

		// Create connection
		auto connection = unique_ptr<Connection>(new Connection(serverAddresses));
		
		ConnectResult result = { nullptr, connection.release() };
		return result;
	}
	catch (exception e)
	{
		ConnectResult result = { ExceptionToFastoreError(e), nullptr };
		return result;
	}
	catch (...)
	{
		ConnectResult result = { new FastoreError(), nullptr };
	}
}

FASTOREAPI PFastoreError APIENTRY fastoreDisconnect(DatabaseHandle database)
{
	delete database;
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

