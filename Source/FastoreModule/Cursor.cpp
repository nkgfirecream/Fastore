#include "Cursor.h"
//#include "../FastoreCore/safe_cast.h"
#include "../FastoreClient/Encoder.h"

namespace client = fastore::client;
namespace module = fastore::module;

module::Cursor::Cursor(module::Table* table) : _table(table), _index(-1), _lastIdxNum(0), _needsReset(true) { }

int module::Cursor::next()
{
	++_index;
	if (size_t(_index) == _set.Data.size())
	{
		getNextSet();
		_needsReset = true;
	}

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

	auto type = _table->_columns[index].ValueType.Name;
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
			sqlite3_result_int64(pContext, Encoder<int64_t>::Decode(value.value));
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
	*pRowid = (sqlite3_int64)client::Encoder<int64_t>::Decode(_set.Data[_index].ID);

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
	
	//idxNum is either colIdx + 1, ~(colIdx + 1) or 0 (0 = full table pull)
	int colIndex = idxNum > 0 ? idxNum - 1 : (idxNum < 0 ? (~idxNum) - 1 : 0); 
	std::string idx;
	if (idxStr != NULL)
		idx = std::string(idxStr);

	std::vector<std::string> values = getVector(colIndex, argc, argv);
	if (!_needsReset && _lastIdxNum == idxNum && _lastIdxString == idx && compareVectors(values, _lastValues))
	{
		//Exact same request for the same data. Just rewind
		_index = 0;
	}
	else
	{
		//Pull new data...
		_lastIdxNum = idxNum;
		_lastIdxString = idx;
		_lastValues = values;		


		//Clear variables...
		_index = -1;
		_set = RangeSet();

		//Set up new range
		//Idxnum > 0 = ascending, < 0 = desc, 0 = no index column (full table pull)
		bool ascending = idxNum > 0;
		_range = createRange(ascending, colIndex, _lastIdxString, _lastValues); 

		//Pull first set.
		getNextSet();
		_needsReset = false;
	}

	return SQLITE_OK;
}


client::Range module::Cursor::createRange(bool ascending, int colIndex, std::string& idxStr, std::vector<std::string>& values)
{
	client::Range range;
	
	range.Ascending = ascending;	
	//TODO: Figure out how to grab all the rowIds when there isn't a key column. 
	//Should this be masked from SQLite? 
	//Should the client support a "get all keys" operation and we iterate over them?
	//Should we force at least one required column?
	
	range.ColumnID = _table->_columns[colIndex].ColumnID;

	if (!idxStr.empty())
	{
		if (idxStr[0] == SQLITE_INDEX_CONSTRAINT_EQ)
		{
			client::RangeBound bound;
			bound.Bound = values[0];
			bound.Inclusive = true;
		
			range.End = bound;
			range.Start = bound;
		}
		else
		{
			for (int i = 0; i < values.size(); i++)
			{
				client::RangeBound bound = getBound(colIndex, idxStr[i], values[i]);
				if (range.Ascending)
				{
					if (idxStr[i] == SQLITE_INDEX_CONSTRAINT_GT || idxStr[i] == SQLITE_INDEX_CONSTRAINT_GE)
						range.Start = bound;
					else
						range.End = bound;
				}
				else
				{
					if (idxStr[i] == SQLITE_INDEX_CONSTRAINT_GT || idxStr[i] == SQLITE_INDEX_CONSTRAINT_GE)
						range.End = bound;
					else
						range.Start = bound;
				}
			}
		}
	}

	return range;
}


client::RangeBound module::Cursor::getBound(int col, char op, std::string& boundValue)
{
	client::RangeBound bound;
	if (op == SQLITE_INDEX_CONSTRAINT_GE || op == SQLITE_INDEX_CONSTRAINT_LE)
		bound.Inclusive = true;
	else
		bound.Inclusive = false;

	bound.Bound = boundValue;

	return bound;
}

std::string module::Cursor::convertSqliteValueToString(int col, sqlite3_value* arg)
{
	std::string type = _table->_columns[col].ValueType.Name;
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

std::vector<std::string> module::Cursor::getVector(int colIndex, int argc, sqlite3_value** args)
{
	std::vector<std::string> result(argc);
	for (int i = 0; i < argc; ++i)
	{
		result[i] = convertSqliteValueToString(colIndex, args[i]);
	}

	return result;
}

bool module::Cursor::compareVectors(std::vector<std::string> left, std::vector<std::string> right)
{
	if (left.size() != right.size())
		return false;

	for (size_t i = 0; i < left.size(); ++i)
	{
		if (left[i] != right[i])
			return false;
	}

	return true;
}

