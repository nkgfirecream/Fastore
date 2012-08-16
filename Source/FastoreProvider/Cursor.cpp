#include "Cursor.h"
#include <sqlite3.h>
#include "../FastoreClient/Encoder.h"
using namespace std;

fastore::provider::Cursor::Cursor(IDataAccess *dataAccess, sqlite3* db, const string &sql) : _dataAccess(dataAccess)
{
	sqlite3_prepare(db, sql.c_str(), -1, &_statement, NULL);
}

fastore::provider::Cursor::~Cursor()
{
	sqlite3_finalize(_statement);
}

void fastore::provider::Cursor::bind(std::vector<Argument> arguments)
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

void fastore::provider::Cursor::internalBind(int index, ArgumentType type, std::string& value)
{
	switch(type)
	{
	case ArgumentType::FASTORE_ARGUMENT_BOOL : sqlite3_bind_int(_statement, index, fastore::client::Encoder<bool>::Decode(value)); break;
	case ArgumentType::FASTORE_ARGUMENT_DOUBLE :  sqlite3_bind_double(_statement, index, fastore::client::Encoder<double>::Decode(value)); break;
	case ArgumentType::FASTORE_ARGUMENT_INT32 : sqlite3_bind_int(_statement, index, fastore::client::Encoder<int>::Decode(value)); break;
	case ArgumentType::FASTORE_ARGUMENT_INT64 : sqlite3_bind_int64(_statement, index, fastore::client::Encoder<long long>::Decode(value)); break;
	case ArgumentType::FASTORE_ARGUMENT_NULL : sqlite3_bind_null(_statement, index); break;
	case ArgumentType::FASTORE_ARGUMENT_STRING16 : sqlite3_bind_text16(_statement, index, value.c_str(), value.length, nullptr); break;
	case ArgumentType::FASTORE_ARGUMENT_STRING8 : sqlite3_bind_text(_statement, index, value.c_str(), value.length, nullptr); break;
	default :
		throw "Unrecognized argument type";
	}
}

bool fastore::provider::Cursor::next()
{
	sqlite3_step(_statement);
}

int fastore::provider::Cursor::columnCount()
{
	return sqlite3_column_count(_statement);
}

fastore::provider::ColumnInfo fastore::provider::Cursor::getColumnInfo(int index)
{
	sqlite3_column_type(_statement, index);
	sqlite3_column_name(_statement, index);
	//sqlite3_column_
}
