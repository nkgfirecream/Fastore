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
#include "../FastoreCore/safe_cast.h"

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

ConnectResult fastoreConnect(size_t addressCount, const FastoreAddress addresses[])
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
				serverAddresses[i].Port = SAFE_CAST(int, 
								    addresses[i].port);
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

GeneralResult fastoreReset(StatementHandle statement)
{
	return WrapCall<GeneralResult>
	(
		[&](GeneralResult &result) 
		{ 
			static_cast<prov::PStatementObject>(statement)->get()->reset();
		}
	);
}

GeneralResult fastoreBindInt64(StatementHandle statement, int32_t argumentIndex, int64_t value)
{
	return WrapCall<GeneralResult>
	(
		[&](GeneralResult &result) 
		{ 
			static_cast<prov::PStatementObject>(statement)->get()->bindInt64(argumentIndex, value);
		}
	);
}

GeneralResult fastoreBindDouble(StatementHandle statement, int32_t argumentIndex, double value)
{
	return WrapCall<GeneralResult>
	(
		[&](GeneralResult &result) 
		{ 
			static_cast<prov::PStatementObject>(statement)->get()->bindDouble(argumentIndex, value);
		}
	);
}

GeneralResult fastoreBindAString(StatementHandle statement, int32_t argumentIndex, const char *value)
{
	return WrapCall<GeneralResult>
	(
		[&](GeneralResult &result) 
		{ 
			static_cast<prov::PStatementObject>(statement)->get()->bindAString(argumentIndex, std::string(value));
		}
	);
}

GeneralResult fastoreBindWString(StatementHandle statement, int32_t argumentIndex, const wchar_t *value)
{
	return WrapCall<GeneralResult>
	(
		[&](GeneralResult &result) 
		{ 
			static_cast<prov::PStatementObject>(statement)->get()->bindWString(argumentIndex, std::wstring(value));
		}
	);
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
	return WrapCall<ColumnInfoResult>
	(
		[&](ColumnInfoResult &result) 
		{ 
			auto info = static_cast<prov::PStatementObject>(statement)->get()->getColumnInfo(columnIndex);

			strncpy(result.name, info.name.c_str(), sizeof(result.name) - 1);
			result.name[sizeof(result.name) - 1] = '\0';
			
			result.type = info.type; 
		}
	);
}

ColumnValueInt64Result fastoreColumnValueInt64(StatementHandle statement, int32_t columnIndex)
{
	return WrapCall<ColumnValueInt64Result>
	(
		[&](ColumnValueInt64Result &result) 
		{ 
			auto value = static_cast<prov::PStatementObject>(statement)->get()->getColumnValueInt64(columnIndex);
			result.isNull = !value.is_initialized();
			if (!result.isNull)
				result.value = value.get();
		}
	);
}

ColumnValueDoubleResult fastoreColumnValueDouble(StatementHandle statement, int32_t columnIndex)
{
	return WrapCall<ColumnValueDoubleResult>
	(
		[&](ColumnValueDoubleResult &result) 
		{ 
			auto value = static_cast<prov::PStatementObject>(statement)->get()->getColumnValueDouble(columnIndex);
			result.isNull = !value.is_initialized();
			if (!result.isNull)
				result.value = value.get();
		}
	);
}

ColumnValueStringResult fastoreColumnValueAString(StatementHandle statement, int32_t columnIndex, int *targetMaxBytes, char *valueTarget)
{
	return WrapCall<ColumnValueStringResult>
	(
		[&](ColumnValueStringResult &result) 
		{ 
			auto value = static_cast<prov::PStatementObject>(statement)->get()->getColumnValueAString(columnIndex);
			result.isNull = !value.is_initialized();
			if (!result.isNull)
			{
				*targetMaxBytes = min(*targetMaxBytes, INT_CAST(value.get().size()));
				memcpy(valueTarget, value.get().data(), *targetMaxBytes);
			}
		}
	);
}

ColumnValueStringResult fastoreColumnValueWString(StatementHandle statement, int32_t columnIndex, int32_t *targetMaxBytes, wchar_t *valueTarget)
{
	return WrapCall<ColumnValueStringResult>
	(
		[&](ColumnValueStringResult &result) 
		{ 
			auto value = static_cast<prov::PStatementObject>(statement)->get()->getColumnValueWString(columnIndex);
			result.isNull = !value.is_initialized();
			if (!result.isNull)
			{
				*targetMaxBytes = min(*targetMaxBytes, INT_CAST(value.get().size() * sizeof(wchar_t)));
				memcpy(valueTarget, value.get().data(), *targetMaxBytes);
			}
		}
	);
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

// Short-hand for Prepare followed by Next (and a close if eof or no columns)
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
	if (nextResult.result != FASTORE_OK || nextResult.eof || prepareResult.columnCount == 0)
	{
		fastoreClose(prepareResult.statement);	// Ignore any close error so as not to lose original error
		result.statement = nullptr;
	}
	else
		result.statement = prepareResult.statement;

	if (nextResult.result != FASTORE_OK)
	{
		result.result = nextResult.result;
		return result;
	}

	result.columnCount = prepareResult.columnCount;
	result.eof = nextResult.eof;
	return result;
};

