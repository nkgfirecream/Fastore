#include "fastore.h"
#include <memory>
#include "Connection.h"
#include "Statement.h"
#include "Address.h"
#include <exception>
#include <functional>
#include <vector>
#include <cstring>

using namespace std;
namespace prov = fastore::provider;

void ExceptionToFastoreResult(exception e, int code, FastoreResult &result)
{
	result.success = false;
	result.error.code = code;
	const size_t len = min(sizeof(result.error.message) - 1, std::strlen(e.what()) );
	strncpy(result.error.message, e.what(), MAX_ERROR_MESSAGE);
	result.error.message[len] = '\0';
}

template <typename TResult>
TResult WrapCall(const function<void(TResult)> &callback)
{
	TResult result;
	try
	{
		callback(result);
	}
	catch (const exception &)
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
			vector<prov::Address> serverAddresses = vector<prov::Address>();
			serverAddresses.resize(addressCount);
			for (auto i = 0; i < addressCount; i++)
			{
				serverAddresses[i].Name = string(addresses[i].hostName);
				serverAddresses[i].Port = addresses[i].port;
			}

			// Create database
			result.connection = new shared_ptr<prov::Connection>(new prov::Connection(serverAddresses));
		}
	);
}

FASTOREAPI FastoreResult APIENTRY fastoreDisconnect(ConnectionHandle database)
{
	return WrapCall<FastoreResult>
	(
		[&](FastoreResult result) 
		{ 
			// Free the shared pointer, database provider will be freed when all shared pointers are freed
			delete static_cast<prov::PConnectionObject>(database); 
		}
	);
}

FASTOREAPI PrepareResult APIENTRY fastorePrepare(ConnectionHandle database, const char *sql)
{
	PrepareResult result;
	return result;
}

FASTOREAPI FastoreResult APIENTRY fastoreBind(StatementHandle cursor, int argumentCount, void *arguments, const struct ArgumentTypes argumentTypes[])
{
	return FastoreResult();
}

FASTOREAPI NextResult APIENTRY fastoreNext(StatementHandle cursor)
{
	NextResult result;
	return result;
}

FASTOREAPI ColumnInfoResult APIENTRY fastoreColumnInfo(StatementHandle cursor, int columnIndex)
{
	return ColumnInfoResult();
}

FASTOREAPI FastoreResult APIENTRY fastoreColumnValue(StatementHandle cursor, int columnIndex, int targetMaxBytes, void *valueTarget)
{
	return FastoreResult();
}

FASTOREAPI FastoreResult APIENTRY fastoreClose(StatementHandle cursor)
{
	return FastoreResult();
}

// Short-hand for Prepare followed by Next (and a close if eof)
FASTOREAPI inline ExecuteResult APIENTRY fastoreExecute(ConnectionHandle database, const char *sql)
{
	ExecuteResult result = { true };

	PrepareResult prepareResult = fastorePrepare(database, sql);
	if (!prepareResult.success)
	{
		result.success = false;
		result.error = prepareResult.error;
		return result;
	}

	NextResult nextResult = fastoreNext(prepareResult.statement);
	if (!nextResult.success || nextResult.eof)
		fastoreClose(prepareResult.statement);	// Ignore any close error so as not to lose original error

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

