#pragma once
#include "Cursor.h"
#include "..\FastoreClient\Encoder.h"

using namespace fastore::module;
namespace client = fastore::client;

Cursor::Cursor(fastore::module::Table* table) : _table(table), _index(-1) { }

void Cursor::next()
{
	++_index;
	if (_index == _set.Data.size())
		getNextSet();
}

int Cursor::eof()
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

void Cursor::setColumnResult(sqlite3_context *pContext, int index)
{
	auto type = _table->_columns[index].Type;
	auto value = _set.Data[_index].Values[index];

	//NULL Marker required...
	//if (value is NULL)
	// sqlite3_result_null(pContext);
	// return;
	
	if (type == "Bool")
		sqlite3_result_int(pContext, (int)Encoder<bool>::Decode(value));
	else if (type == "Int")
		sqlite3_result_int(pContext, Encoder<int>::Decode(value));
	else if (type == "Long")
		sqlite3_result_int64(pContext, Encoder<long long>::Decode(value));
	else if (type == "String")
		sqlite3_result_text(pContext, value.data(), value.length(), NULL);
	else if (type == "WString")
	{
		std::wstring decoded = Encoder<std::wstring>::Decode(value);
		sqlite3_result_text16(pContext, decoded.data(), decoded.length(), NULL);
	}
}

void Cursor::setRowId(sqlite3_int64 *pRowid)
{
	*pRowid = (sqlite3_int64)client::Encoder<int>::Decode(_set.Data[_index].ID);
}

void Cursor::getNextSet()
{
	//First pull...
	if (_set.Data.size() == 0)
	{
		_set = _table->getRange(_range, boost::optional<std::string>());
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

void Cursor::filter(int idxNum, const char *idxStr, int argc, sqlite3_value **argv)
{
	//Clear variables...
	_index = -1;
	_set = RangeSet();
	//The goal here is to set up our private _range and then load data.
	//_range = something...

	_range = Range();
	_range.Ascending; // = idxNum != 0;
	_range.ColumnID = _table->_columns[0].ColumnID;

	getNextSet();
}


