#include "Cursor.h"
#include "../FastoreCore/safe_cast.h"
#include "../FastoreClient/Encoder.h"

namespace client = fastore::client;
namespace module = fastore::module;

module::Cursor::Cursor(module::Table* table) : _table(table), _index(-1) { }

int module::Cursor::next()
{
	++_index;
	if (SAFE_CAST(size_t, _index) == _set.Data.size())
		getNextSet();

	return SQLITE_OK;
}

int module::Cursor::eof()
{
	return (_set.Data.size() == 0 || _index == -1)? 1 : 0;
}

int module::Cursor::setColumnResult(sqlite3_context *pContext, int index)
{	
	auto value = _set.Data[_index].Values[index];

	if (!value.__isset.value)
		sqlite3_result_null(pContext);

	auto type = _table->_columns[index].Type;
	//This function is static, and must be intialized. However,
	//this cursor can't exist without table, so the table static functions
	//must be initialized at this point. Consider refactoring.
	int datatype = module::Table::FastoreTypeToSQLiteTypeID(type);
	switch(datatype)
	{
		case SQLITE_TEXT:
			sqlite3_result_text(pContext, value.value.data(), -1, SQLITE_TRANSIENT);
			break;
		case SQLITE_INTEGER:
			sqlite3_result_int64(pContext, Encoder<long long>::Decode(value.value));
			break;
		case SQLITE_FLOAT:
			sqlite3_result_double(pContext, Encoder<double>::Decode(value.value));
			break;
		default:
			return SQLITE_MISMATCH;
	}

	return SQLITE_OK;
}

int module::Cursor::setRowId(sqlite3_int64 *pRowid)
{
	*pRowid = (sqlite3_int64)client::Encoder<long long>::Decode(_set.Data[_index].ID);

	return SQLITE_OK;
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

int module::Cursor::filter(int idxNum, const char *idxStr, int argc, sqlite3_value **argv)
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
			//TODO: Base start and end of natural order. They should be switched based on constraint (for example, a < constraint implies sticking it at the end).
			//Case: 1 argument -
			// if (> or >=) set start
			// else set end
			//Case: 2 arguments -
			// Assumption: if we have two arguments, one is > and the other is < (best index should have enforced this...)
			// start = >
			// end = <
			if (idxStr[0] == SQLITE_INDEX_CONSTRAINT_GT || idxStr[0] == SQLITE_INDEX_CONSTRAINT_GE)
			{
				_range.Start = getBound(colIndex, idxStr[0], argv[0]);
				if (argc > 1)
					_range.End = getBound(colIndex, idxStr[1], argv[1]);
			}
			else
			{
				_range.End = getBound(colIndex, idxStr[0], argv[0]);
				if (argc > 1)
					_range.Start = getBound(colIndex, idxStr[1], argv[1]);					
			}
		}
	}

	getNextSet();

	return SQLITE_OK;
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
	int datatype = module::Table::FastoreTypeToSQLiteTypeID(type);

	switch(datatype)
	{
		case SQLITE_TEXT:
			result = std::string((char *)sqlite3_value_text(arg));
			break;
		case SQLITE_INTEGER:
			result = Encoder<int64_t>::Encode(sqlite3_value_int64(arg));
			break;
		case SQLITE_FLOAT:
			result = Encoder<double>::Encode(sqlite3_value_double(arg));
			break;
		default:
			throw "ModuleCursor can't find correct type for RangeBound encoding!";
	}

	return result;
}


