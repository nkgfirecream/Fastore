#include "fastore.h"
#include <memory>
#include "Connection.h"
#include "Statement.h"
#include "Address.h"
#include <exception>
#include <functional>
#include <vector>
#include <cstring>
#include "../FastoreClient/ClientException.h"

using namespace std;
using namespace fastore::client;
namespace prov = fastore::provider;

struct FastoreError
{
	char message[MAX_ERROR_MESSAGE];
	FastoreErrorCode code;
};

#if (_MSC_VER)
  __declspec( thread ) FastoreError _lastError;
  __declspec( thread ) FastoreResultCode _lastErrorRevision = 1;
#else
# define thread_local __thread
  thread_local FastoreError _lastError;
  thread_local FastoreResultCode _lastErrorRevision = 1;
#endif

FastoreResultCode ErrorToFastoreResult(const char *message, FastoreErrorCode code)
{
	enum { len = sizeof(_lastError.message) - 1 };

	_lastError.code = code;
	strncpy(_lastError.message, message, len);
	_lastError.message[len] = '\0';
	return ++_lastErrorRevision;
}

FastoreResultCode ExceptionToFastoreResult(exception e, FastoreErrorCode code)
{
	return ErrorToFastoreResult(e.what(), code);
}

template <typename TResult>
TResult WrapCall(const function<void(TResult&)> &callback)
{
	TResult result = TResult();
	try
	{
		callback(result);
	}
	catch (ClientException &e)
	{
		result.result= ExceptionToFastoreResult(e, (FastoreErrorCode)e.code);
	}
	catch (const exception &e)
	{
		result.result = ExceptionToFastoreResult(e, 0);
	}
	catch (char *e)
	{
		result.result= ErrorToFastoreResult(e, (FastoreErrorCode)ClientException::Codes::General);
	}
	catch (...)
	{
		result.result= ErrorToFastoreResult("General error.", (FastoreErrorCode)ClientException::Codes::General);
	}
	return result;
}

bool fastoreGetLastError(const FastoreResultCode result, size_t messageMaxLength, char* message, FastoreErrorCode *code)
{
	if (result == _lastErrorRevision)
	{
		strncpy(message,  _lastError.message, messageMaxLength - 1);
		message[messageMaxLength] = '\0';
		*code = _lastError.code;
		return true;
	}	
	else
		return false;
}

ConnectResult fastoreConnect(size_t addressCount, const struct FastoreAddress addresses[])
{
	return WrapCall<ConnectResult>
	(
		[&](ConnectResult &result)
		{
			// Convert addresses
			vector<prov::Address> serverAddresses = vector<prov::Address>();
			serverAddresses.resize(addressCount);
			for (size_t i = 0; i < addressCount; i++)
			{
				serverAddresses[i].Name = string(addresses[i].hostName);
				serverAddresses[i].Port = addresses[i].port;
			}

			// Create database
			result.connection = new shared_ptr<prov::Connection>(new prov::Connection(serverAddresses));
		}
	);
}

GeneralResult fastoreDisconnect(ConnectionHandle connection)
{
	return WrapCall<GeneralResult>
	(
		[&](GeneralResult &result) 
		{ 
			// Free the shared pointer, database provider will be freed when all shared pointers are freed
			delete static_cast<prov::PConnectionObject>(connection); 
		}
	);
}

PrepareResult fastorePrepare(ConnectionHandle connection, const char *batch)
{
	return WrapCall<PrepareResult>
	(
		[&](PrepareResult &result) 
		{ 
			auto statement = 
				new shared_ptr<prov::Statement>
				(
					static_cast<prov::PConnectionObject>(connection)->get()->execute(batch)
				); 
			result.statement = statement;
			result.columnCount = statement->get()->columnCount();
		}
	);
}

GeneralResult fastoreBind(StatementHandle statement, size_t argumentCount, void *arguments, const ArgumentType argumentTypes[])
{
	return GeneralResult();
}

NextResult fastoreNext(StatementHandle statement)
{
	return WrapCall<NextResult>
	(
		[&](NextResult &result) 
		{ 
			result.eof = !static_cast<prov::PStatementObject>(statement)->get()->next(); 
		}
	);
}

ColumnInfoResult fastoreColumnInfo(StatementHandle statement, int columnIndex)
{
	return ColumnInfoResult();
}

GeneralResult fastoreColumnValue(StatementHandle statement, int columnIndex, int targetMaxBytes, void *valueTarget)
{
	return GeneralResult();
}

GeneralResult fastoreClose(StatementHandle statement)
{
	return WrapCall<GeneralResult>
	(
		[&](GeneralResult &result) 
		{ 
			// Free the shared pointer, statement provider will be freed when all shared pointers are freed
			delete static_cast<prov::PStatementObject>(statement); 
		}
	);
}

// Short-hand for Prepare followed by Next (and a close if eof)
inline ExecuteResult fastoreExecute(ConnectionHandle connection, const char *batch)
{
	ExecuteResult result = { FASTORE_OK };

	PrepareResult prepareResult = fastorePrepare(connection, batch);
	if (prepareResult.result != FASTORE_OK)
	{
		result.result = prepareResult.result;
		return result;
	}

	NextResult nextResult = fastoreNext(prepareResult.statement);
	if (nextResult.result != FASTORE_OK || nextResult.eof)
		fastoreClose(prepareResult.statement);	// Ignore any close error so as not to lose original error

	if (nextResult.result != FASTORE_OK)
	{
		result.result = nextResult.result;
		return result;
	}

	result.columnCount = prepareResult.columnCount;
	result.eof = nextResult.eof;
	return result;
};

