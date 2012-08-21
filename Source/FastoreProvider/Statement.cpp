#include "Statement.h"
#include <sqlite3.h>
#include "../FastoreClient/Encoder.h"
using namespace std;
using namespace fastore::provider;

Statement::Statement(sqlite3* db, const string &sql)
{
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

void Statement::bind(std::vector<Argument> arguments)
{
	sqlite3_reset(_statement);
	int parameterCount = sqlite3_bind_parameter_count(_statement);
	if (parameterCount != arguments.size())
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
	case ArgumentType::FASTORE_ARGUMENT_STRING16 : sqlite3_bind_text16(_statement, index, value.c_str(), value.length(), NULL); break;
	case ArgumentType::FASTORE_ARGUMENT_STRING8 : sqlite3_bind_text(_statement, index, value.c_str(), value.length(), NULL); break;
	default :
		throw "Unrecognized argument type";
	}
}

bool Statement::next()
{
	_eof = (!(sqlite3_step(_statement) == SQLITE_ROW));
}

int Statement::columnCount()
{
	return sqlite3_column_count(_statement);
}

ColumnInfo Statement::getColumnInfo(int index)
{
	ColumnInfo info;
	//TODO: encode type information as a string and return it.
	//Problem here is bools. Sqlite will return them as integers, so we need
	//to compare what sqlite reports and what we have actually defined the column as.
	info.type = sqlite3_column_type(_statement, index);
	info.name = sqlite3_column_name(_statement, index);
}
