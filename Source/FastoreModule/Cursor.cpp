#include "Cursor.h"
#include "../FastoreCore/safe_cast.h"
#include "../FastoreClient/Encoder.h"

namespace client = fastore::client;
namespace module = fastore::module;

module::Cursor::Cursor(module::Table* table) : _table(table), _index(-1) { }

void module::Cursor::next()
{
	++_index;
	if (SAFE_CAST(size_t, _index) == _set.Data.size())
		getNextSet();
}

int module::Cursor::eof()
{
	return (_set.Data.size() == 0 || _index == -1)? 1 : 0;
}

void module::Cursor::setColumnResult(sqlite3_context *pContext, int index)
{
	auto type = _table->_columns[index].Type;
	auto value = _set.Data[_index].Values[index];

	//NULL Marker required...
	if (!value.__isset.value)
		sqlite3_result_null(pContext);
	else if (type == "Bool")
		sqlite3_result_int(pContext, (int)Encoder<bool>::Decode(value.value));
	else if (type == "Int")
		sqlite3_result_int(pContext, Encoder<int>::Decode(value.value));
	else if (type == "Long")
		sqlite3_result_int64(pContext, Encoder<long long>::Decode(value.value));
	else if (type == "Double")
		sqlite3_result_double(pContext, Encoder<double>::Decode(value.value));
	else if (type == "String")
		sqlite3_result_text(pContext, value.value.data(), -1, SQLITE_TRANSIENT);
	else if (type == "WString")
	{
		std::wstring decoded = Encoder<std::wstring>::Decode(value.value);
		sqlite3_result_text16(pContext, decoded.data(), -1, SQLITE_TRANSIENT);
	}
}

void module::Cursor::setRowId(sqlite3_int64 *pRowid)
{
	*pRowid = (sqlite3_int64)client::Encoder<int>::Decode(_set.Data[_index].ID);
}

void module::Cursor::getNextSet()
{
	//First pull...
	if (_set.Data.size() == 0)
	{
	    const boost::optional<std::string> s;
	    _set = _table->getRange(_range, s);
		if (_set.Data.size() > 0)
			_index = 0;
	}
	else
	{
		if (_set.Eof)
			_index = -1;
		else
		{
			_set = _table->getRange(_range, boost::optional<std::string>(_set.Data[_index - 1].ID)); //Assumes index is currently at _set.Data.size() -- which it should be if this is the result of a next call
			if (_set.Data.size() > 0)
				_index = 0;
		}
	}
}

void module::Cursor::filter(int idxNum, const char *idxStr, int argc, sqlite3_value **argv)
{
	//Clear variables...
	_index = -1;
	_set = RangeSet();
	//The goal here is to set up our private _range and then load data.
	//_range = something...

	_range = Range();
	//TODO: Make ascending dependent on index information that comes in.
	//Idxnum > 0 = ascending, < 0 = desc, 0 = no index column (full table pull)
	_range.Ascending = idxNum >= 0;
	
	//idxNum is either colIdx + 1, ~(colIdx + 1) or 0
	int colIndex = idxNum > 0 ? idxNum - 1 : (idxNum < 0 ? (~idxNum) - 1 : 0);
	_range.ColumnID = _table->_columns[colIndex].ColumnID;

	if (idxStr != NULL)
	{
		if (idxStr[0] == SQLITE_INDEX_CONSTRAINT_EQ)
		{
			client::RangeBound bound;
			bound.Bound = convertSqliteValueToString(colIndex, argv[0]);
			bound.Inclusive = true;
		
			_range.End = bound;
			_range.Start = bound;
		}
		else
		{
			_range.Start = getBound(colIndex, idxStr[0], argv[0]);
			if (argc > 1)
				_range.End = getBound(colIndex, idxStr[1], argv[1]);
		}
	}

	getNextSet();
}

client::RangeBound module::Cursor::getBound(int col, char op, sqlite3_value* arg)
{
	client::RangeBound bound;
	if (op == SQLITE_INDEX_CONSTRAINT_GE || op == SQLITE_INDEX_CONSTRAINT_LE)
		bound.Inclusive = true;
	else
		bound.Inclusive = false;

	bound.Bound = convertSqliteValueToString(col, arg);

	return bound;
}

std::string module::Cursor::convertSqliteValueToString(int col, sqlite3_value* arg)
{
	std::string type = _table->_columns[col].Type;
	std::string result;

	if (type == "String")
		result = std::string((char *)sqlite3_value_text(arg));
	else if (type == "Int")
		result = Encoder<int>::Encode(sqlite3_value_int(arg));
	else if (type == "Long")
		result = Encoder<int64_t>::Encode(sqlite3_value_int64(arg));
	else if (type == "Double")
		result = Encoder<double>::Encode(sqlite3_value_double(arg));
	else if (type == "Bool")
		result = Encoder<bool>::Encode(sqlite3_value_int(arg) != 0);
	else if (type == "WString")
	{
		std::wstring toEncode((wchar_t*)sqlite3_value_text16(arg));
		result = Encoder<std::wstring>::Encode(toEncode);
	}
	else
		throw "ModuleCursor can't find correct type for RangeBound encoding!";

	return result;

}


