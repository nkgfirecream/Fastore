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
	//sqlite3_result_blob()
	//sqlite3_result_double()
	//sqlite3_result_int()
	//sqlite3_result_int64()
	//sqlite3_result_null()
	//sqlite3_result_text()
	//sqlite3_result_text16()
	//sqlite3_result_text16le()
	//sqlite3_result_text16be()
	//sqlite3_result_zeroblob()
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

	getNextSet();
}


