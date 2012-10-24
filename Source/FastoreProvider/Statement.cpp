#include "Statement.h"
#include <sqlite3.h>
#include "../FastoreCore/safe_cast.h"
#include "../FastoreClient/Encoder.h"
using namespace std;
using namespace fastore::provider;


std::map<int, ArgumentType> createTypeMap()
{
	map<int, ArgumentType> m;
	
	//m[] = ArgumentType::FASTORE_ARGUMENT_BOOL;
	//m[] =  ArgumentType::FASTORE_ARGUMENT_INT32;
	m[SQLITE_INTEGER] = ArgumentType::FASTORE_ARGUMENT_INT64;
	m[SQLITE3_TEXT] = ArgumentType::FASTORE_ARGUMENT_STRING8;
	//m["WString"] = ArgumentType::FASTORE_ARGUMENT_STRING16;
	m[SQLITE_FLOAT] = ArgumentType::FASTORE_ARGUMENT_DOUBLE;
	m[SQLITE_NULL] = ArgumentType::FASTORE_ARGUMENT_NULL;

	return m;
}

Statement::Statement(sqlite3* db, const string &sql)
{
	_types = createTypeMap();
	_eof = (!(sqlite3_prepare_v2(db, sql.c_str(), -1, &_statement, NULL) == SQLITE_ROW));
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
	sqlite3_reset(_statement);
}

void Statement::bind(std::vector<fastore::provider::Argument> arguments)
{
	sqlite3_reset(_statement);
	int parameterCount = sqlite3_bind_parameter_count(_statement);
	if (parameterCount != SAFE_CAST(int,arguments.size()))
		throw "Wrong number of arguments";

	for (int i = 0; i < parameterCount; ++i)
	{
		auto arg = arguments[i];
		internalBind(i, arg.type, arg.value);
	}
}

void Statement::internalBind(int index, ArgumentType type, std::string& value)
{
	switch(type)
	{
	case ArgumentType::FASTORE_ARGUMENT_BOOL : sqlite3_bind_int(_statement, index, fastore::client::Encoder<bool>::Decode(value)); break;
	case ArgumentType::FASTORE_ARGUMENT_DOUBLE :  sqlite3_bind_double(_statement, index, fastore::client::Encoder<double>::Decode(value)); break;
	case ArgumentType::FASTORE_ARGUMENT_INT32 : sqlite3_bind_int(_statement, index, fastore::client::Encoder<int>::Decode(value)); break;
	case ArgumentType::FASTORE_ARGUMENT_INT64 : sqlite3_bind_int64(_statement, index, fastore::client::Encoder<long long>::Decode(value)); break;
	case ArgumentType::FASTORE_ARGUMENT_NULL : sqlite3_bind_null(_statement, index); break;
	case ArgumentType::FASTORE_ARGUMENT_STRING16 : sqlite3_bind_text16(_statement, index, value.c_str(), SAFE_CAST(int,value.length()), NULL); break;
	case ArgumentType::FASTORE_ARGUMENT_STRING8 : sqlite3_bind_text(_statement, index, value.c_str(), SAFE_CAST(int, value.length()), NULL); break;
	default :
		throw "Unrecognized argument type";
	}
}

bool Statement::next()
{
	_eof = (!(sqlite3_step(_statement) == SQLITE_ROW));
	return _eof;
}

int Statement::columnCount()
{
	return sqlite3_column_count(_statement);
}

ColumnInfo Statement::getColumnInfo(int index)
{
	auto iter = _infos.find(index);

	if (iter != _infos.end())
		return iter->second;

	ColumnInfo info;
	//TODO: encode type information as a string and return it.
	//Problem here is bools. Sqlite will return them as integers, so we need
	//to compare what sqlite reports and what we have actually defined the column as.
	info.type = _types[sqlite3_column_type(_statement, index)];
	info.name = sqlite3_column_name(_statement, index);

	_infos.insert(std::pair<int, ColumnInfo>(index, info));

	return info;
}

std::string Statement::getColumn(int index)
{
	auto type = _types[sqlite3_column_type(_statement, index)];
	switch (type)
	{
		//case ArgumentType::FASTORE_ARGUMENT_BOOL : break;
		case ArgumentType::FASTORE_ARGUMENT_DOUBLE : return fastore::client::Encoder<double>::Encode(sqlite3_column_double(_statement, index));
		case ArgumentType::FASTORE_ARGUMENT_INT32 : return fastore::client::Encoder<int32_t>::Encode(sqlite3_column_int(_statement, index));
		case ArgumentType::FASTORE_ARGUMENT_INT64 : return fastore::client::Encoder<int64_t>::Encode(sqlite3_column_int64(_statement, index));
		case ArgumentType::FASTORE_ARGUMENT_NULL : /* return null string */
		//case ArgumentType::FASTORE_ARGUMENT_STRING16 : sqlite3_bind_text16(_statement, index, value.c_str(), value.length(), NULL); break;
		case ArgumentType::FASTORE_ARGUMENT_STRING8 : return std::string((char*)sqlite3_column_text(_statement, index));
		default :
			throw "Unrecognized argument type";
	}
}
