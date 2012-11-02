#include "Statement.h"
#include <sqlite3.h>
//#include "../FastoreCore/safe_cast.h"
#include "../FastoreClient/Encoder.h"
#include "../FastoreClient/ClientException.h"
#include "SQLiteHelpers.h"
#include <boost/format.hpp>
using namespace std;
using namespace fastore::provider;
using boost::optional;
using fastore::client::ClientException;

std::map<string, ArgumentType, LexCompare> createTypeMap()
{
	map<string, ArgumentType, LexCompare> m;
	
	//m[] = ArgumentType::FASTORE_ARGUMENT_BOOL;
	//m[] =  ArgumentType::FASTORE_ARGUMENT_INT32;
	m["int"] = ArgumentType::FASTORE_ARGUMENT_INT64;
	m["varchar"] = ArgumentType::FASTORE_ARGUMENT_STRING8;
	//m["nvarchar"] = ArgumentType::FASTORE_ARGUMENT_STRING16;
	m["float"] = ArgumentType::FASTORE_ARGUMENT_DOUBLE;
	m["null"] = ArgumentType::FASTORE_ARGUMENT_NULL;

	return m;
}

Statement::Statement(sqlite3* db, const string &sql)
{
	_types = createTypeMap();
	int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &_statement, NULL);
	checkSQLiteResult(result);
	_eof = false;
}

Statement::~Statement()
{
	sqlite3_finalize(_statement);
}

bool Statement::eof()
{
	return _eof;
}

void Statement::reset()
{
	_eof = false;
	auto result = sqlite3_reset(_statement);
	checkSQLiteResult(result);
}

void Statement::internalBind(int32_t index)
{
	_eof = false;
	auto result = sqlite3_reset(_statement);
	checkSQLiteResult(result);}

void Statement::checkBindResult(int result)
{
	if (result == SQLITE_RANGE)
	{
		int parameterCount = sqlite3_bind_parameter_count(_statement);
		throw ClientException((boost::format("Parameter index is out of range (max %i)") % parameterCount).str());
	}
	checkSQLiteResult(result);
}

void Statement::bindInt64(int32_t index, int64_t value)
{
	internalBind(index);
	auto result = sqlite3_bind_int64(_statement, index, value);
	checkBindResult(result);
}

void Statement::bindDouble(int32_t index, double value)
{
	internalBind(index);
	auto result = sqlite3_bind_double(_statement, index, value);
	checkBindResult(result);
}

void Statement::bindAString(int32_t index, std::string value)
{
	internalBind(index);
	auto result = sqlite3_bind_text(_statement, index, value.c_str(), -1, SQLITE_TRANSIENT);
	checkBindResult(result);
}

void Statement::bindWString(int32_t index, std::wstring value)
{
	internalBind(index);
	auto result = sqlite3_bind_text16(_statement, index, value.c_str(), -1, SQLITE_TRANSIENT);
	checkBindResult(result);
}

bool Statement::next()
{
	auto result = sqlite3_step(_statement);
	if (result != SQLITE_ROW && result != SQLITE_DONE)
		checkSQLiteResult(result);
	_eof = result != SQLITE_ROW;
	return !_eof;
}

int Statement::columnCount()
{
	return sqlite3_column_count(_statement);
}

ColumnInfo Statement::getColumnInfo(int32_t index)
{
	//TODO: concurrency
	auto iter = _infos.find(index);

	if (iter != _infos.end())
		return iter->second;

	ColumnInfo info;
	auto typeName = sqlite3_column_decltype(_statement, index);
	info.logicalType = typeName == nullptr ? std::string() : std::string(typeName);
	info.physicalType = _types[info.logicalType];
	auto columnName = sqlite3_column_name(_statement, index);
	info.name = columnName == nullptr ? std::string() : std::string(columnName);

	_infos.insert(std::pair<int, ColumnInfo>(index, info));

	return info;
}

optional<int64_t> Statement::getColumnValueInt64(int32_t index)
{
	if (sqlite3_column_type(_statement, index) != SQLITE_NULL)
		return sqlite3_column_int64(_statement, index);	
	return optional<int64_t>();
}

optional<double> Statement::getColumnValueDouble(int32_t index)
{
	if (sqlite3_column_type(_statement, index) != SQLITE_NULL)
		return sqlite3_column_double(_statement, index);	
	return optional<double>();
}

optional<std::string> Statement::getColumnValueAString(int32_t index)
{
	if (sqlite3_column_type(_statement, index) != SQLITE_NULL)
		return std::string((char*)sqlite3_column_text(_statement, index));	
	return optional<std::string>();
}

optional<std::wstring> Statement::getColumnValueWString(int32_t index)
{
	if (sqlite3_column_type(_statement, index) != SQLITE_NULL)
		return std::wstring((wchar_t*)sqlite3_column_text16(_statement, index));	
	return optional<std::wstring>();
}
