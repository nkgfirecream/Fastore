#include "Cursor.h"
#include "SafeCast.h"
#include "../FastoreClient/Encoder.h"

namespace client = fastore::client;
namespace module = fastore::module;

module::Cursor::Cursor(module::Table* table) : _table(table), _index(-1) { }

void module::Cursor::next()
{
	++_index;
	if (SAFE_CAST(size_t(), _index) == _set.Data.size())
		getNextSet();
}

int module::Cursor::eof()
{
	if (_set.Data.size() == 0 || _index == -1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
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
	_range.Ascending = true;
	_range.ColumnID = _table->_columns[0].ColumnID;

	getNextSet();
}


