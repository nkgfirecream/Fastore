#include "fastore.h"
#include <memory>
#include "Database.h"
#include <exception>
#include <functional>

using namespace std;

PFastoreError ExceptionToFastoreError(exception e, int code)
{
	auto error = unique_ptr<FastoreError>(new FastoreError());
	error->code = code;
	strcpy_s(error->message, MAX_ERROR_MESSAGE, e.what());
	return error.release();
}

FASTOREAPI void APIENTRY fastoreFreeError(PFastoreError error)
{
	if (error)
		delete error;
}

template <typename TResult>
TResult WrapCall(const function<void(TResult)> &callback)
{
	TResult result;
	try
	{
		callback(result);
	}
	catch (exception &e)
	{
		result.error = ExceptionToFastoreError(e, 0);
	}
	// TODO: Uncomment this once we have the client compiling
	//catch (ClientException &e)
	//{
	//	result.error = ExceptionToFastoreError(e, (int)e.Code);
	//}
	catch (...)
	{
		result.error = new FastoreError();
	}
	return result;
}

FASTOREAPI ConnectResult APIENTRY fastoreConnect(int addressCount, FastoreAddress *addresses)
{
	return WrapCall<ConnectResult>
	(
		[&](ConnectResult result)
		{
			// Convert addresses
			vector<ServerAddress> serverAddresses = vector<ServerAddress>();
			serverAddresses.resize(addressCount);
			for (auto i = 0; i < addressCount; i++)
			{
				serverAddresses[i].hostName	= string(addresses[i].hostName);
				serverAddresses[i].port = addresses[i].port;
			}

			// Create database
			result.database = new Database(serverAddresses);
		}
	);
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

