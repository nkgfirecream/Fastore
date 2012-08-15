#include "fastore.h"
#include <memory>
#include "Database.h"
#include "Transaction.h"
#include <exception>
#include <functional>
#include <vector>

using namespace std;
namespace prov = fastore::provider;

void ExceptionToFastoreResult(exception e, int code, FastoreResult &result)
{
	result.success = false;
	result.error.code = code;
	strcpy_s(result.error.message, MAX_ERROR_MESSAGE, e.what());
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
		//result.error = ExceptionToFastoreResult(e, 0, &(FastoreResult)result);
	}
	// TODO: Uncomment this once we have the client compiling
	//catch (ClientException &e)
	//{
	//	result.error = ExceptionToFastoreError(e, (int)e.Code);
	//}
	catch (...)
	{
		result.error = FastoreError();
	}
	return result;
}

FASTOREAPI ConnectResult APIENTRY fastoreConnect(int addressCount, const struct FastoreAddress addresses[])
{
	return WrapCall<ConnectResult>
	(
		[&](ConnectResult result)
		{
			// Convert addresses
			vector<prov::ServerAddress> serverAddresses = vector<prov::ServerAddress>();
			serverAddresses.resize(addressCount);
			for (auto i = 0; i < addressCount; i++)
			{
				serverAddresses[i].hostName	= string(addresses[i].hostName);
				serverAddresses[i].port = addresses[i].port;
			}

			// Create database
			result.database = new shared_ptr<prov::Database>(new prov::Database(serverAddresses));
		}
	);
}

FASTOREAPI FastoreResult APIENTRY fastoreDisconnect(DatabaseHandle database)
{
	return WrapCall<FastoreResult>
	(
		[&](FastoreResult result) 
		{ 
			// Free the shared pointer, database provider will be freed when all shared pointers are freed
			delete static_cast<prov::PDatabaseObject>(database); 
		}
	);
}

FASTOREAPI BeginResult APIENTRY fastoreBegin(DatabaseHandle database)
{
	return WrapCall<BeginResult>
	(
		[&](BeginResult result)
		{
			result.transaction = 
				new shared_ptr<prov::Transaction>
				(
					new prov::Transaction(static_cast<prov::PDatabaseObject>(database)->get())
				);
		}
	);
}

FASTOREAPI FastoreResult APIENTRY fastoreCommit(DatabaseHandle database, bool flush)
{
	return WrapCall<FastoreResult>([&](FastoreResult result) { (*static_cast<prov::PDatabaseObject>(database))->commit(); });
}

FASTOREAPI FastoreResult APIENTRY fastoreRollback(DatabaseHandle database)
{
	return FastoreResult();
}

FASTOREAPI PrepareResult APIENTRY fastorePrepare(DatabaseHandle database, const char *sql)
{
	PrepareResult result;
	return result;
}

FASTOREAPI FastoreResult APIENTRY fastoreBind(CursorHandle cursor, int argumentCount, void *arguments, const struct ArgumentTypes argumentTypes[])
{
	return FastoreResult();
}

FASTOREAPI NextResult APIENTRY fastoreNext(CursorHandle cursor)
{
	NextResult result;
	return result;
}

FASTOREAPI ColumnInfoResult APIENTRY fastoreColumnInfo(CursorHandle cursor, int columnIndex)
{
	return ColumnInfoResult();
}

FASTOREAPI FastoreResult APIENTRY fastoreColumnValue(CursorHandle cursor, int columnIndex, int targetMaxBytes, void *valueTarget)
{
	return FastoreResult();
}

FASTOREAPI FastoreResult APIENTRY fastoreClose(CursorHandle cursor)
{
	return FastoreResult();
}

// Short-hand for Prepare followed by Next (and a close if eof)
FASTOREAPI inline ExecuteResult APIENTRY fastoreExecute(DatabaseHandle database, const char *sql)
{
	ExecuteResult result = { true };

	PrepareResult prepareResult = fastorePrepare(database, sql);
	if (!prepareResult.success)
	{
		result.success = false;
		result.error = prepareResult.error;
		return result;
	}

	NextResult nextResult = fastoreNext(prepareResult.cursor);
	if (!nextResult.success || nextResult.eof)
		fastoreClose(prepareResult.cursor);	// Ignore any close error so as not to lose original error

	if (!nextResult.success)
	{
		result.success = false;
		result.error = nextResult.error;
		return result;
	}

	result.success = true;
	result.columnCount = prepareResult.columnCount;
	result.eof = nextResult.eof;
	return result;
};

