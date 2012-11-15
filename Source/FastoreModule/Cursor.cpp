#include "Cursor.h"
//#include "../FastoreCore/safe_cast.h"
#include "../FastoreClient/Encoder.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace client = fastore::client;
namespace module = fastore::module;

module::Cursor::Cursor(module::Table* table) : _table(table), _index(-1), _lastIdxNum(0), _initialUse(true) { }

int module::Cursor::next()
{
	++_index;
	if (size_t(_index) == _set.size() || (_lastIdxString.size() > 0 && _lastIdxString[0] == SQLITE_INDEX_CONSTRAINT_EQ && _lastValues[0].compare(_set[_index].Values[_colIndex].value) != 0))
	{
		_index = -1;
	}

	return SQLITE_OK;
}

int module::Cursor::eof()
{
	return (_set.size() == 0 || _index == -1)? 1 : 0;
}

int module::Cursor::setColumnResult(sqlite3_context *pContext, int index)
{	
	auto value = _set[_index].Values[index];

	if (!value.__isset.value)
		sqlite3_result_null(pContext);

	auto declaredType = _table->_declaredTypes[index];
	
	//Special handing for dates
	if (strcasecmp(declaredType.c_str(), "date") == 0 || strcasecmp(declaredType.c_str(), "datetime") == 0)
	{
		time_t epoch =  Encoder<int64_t>::Decode(value.value);
		boost::posix_time::ptime p = boost::posix_time::from_time_t(epoch);
		auto string = boost::posix_time::to_simple_string(p);
		sqlite3_result_text(pContext, string.data(), -1, SQLITE_TRANSIENT);
	}
	else 
	{
		int datatype = _table->declaredTypeToSQLiteTypeID[declaredType];
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
	}

	return SQLITE_OK;
}

int module::Cursor::setRowId(sqlite3_int64 *pRowid)
{
	*pRowid = (sqlite3_int64)client::Encoder<int64_t>::Decode(_set[_index].ID);

	return SQLITE_OK;
}

void module::Cursor::getNextSet()
{
	//First pull...
	if (_set.size() == 0)
	{
		boost::optional<std::string> s;
		auto rangeSet = _table->getRange(_range, s);
		_set = rangeSet.Data;
		if (_set.size() > 0)
			_index = 0;

		while(!rangeSet.Eof)
		{
			s = _set[_set.size() -1].ID;
			rangeSet = _table->getRange(_range, s);
			for (int i = 0; i < rangeSet.Data.size(); ++i)
			{
				_set.push_back(rangeSet.Data[i]);
			}
		}

	}
	//Temporarily disabled for caching cursor.
	//else
	//{
	//	if (_set.Eof)
	//		_index = -1;
	//	else
	//	{
	//		_set = _table->getRange(_range, boost::optional<std::string>(_set.Data[_index - 1].ID)); //Assumes index is currently at _set.Data.size() -- which it should be if this is the result of a next call
	//		if (_set.Data.size() > 0)
	//			_index = 0;
	//	}
	//}
}

int module::Cursor::filter(int idxNum, const char *idxStr, int argc, sqlite3_value **argv)
{
	//For equality constraints, try to figure out if we are inside of a loop.
	//If so, pull extra rows to as a read-ahead cache for query processor.
	//Fetch rows with >=, even if it's an equality constraint. Then detect if subsequent reads are in order.
	//If reverse order, change constraint to <= for future fetches. If no order, revert to == for future fetches.
	//(BONUS POINTS!) Make cursor self tune so that it pulls ~number of rows actually read by query processor.

	//idxNum is either colIdx + 1, ~(colIdx + 1) or 0 (0 = full table pull)
	_colIndex = idxNum > 0 ? idxNum - 1 : (idxNum < 0 ? (~idxNum) - 1 : 0); 

	std::vector<std::string> values;
	int result = getVector(_colIndex, argc, argv, values);
	//In case SQLITE gives us bad values to filter by (for example, a string in a integer column)
	if (result != SQLITE_OK)
		return result;

	if( !_initialUse && _lastIdxNum == idxNum && idxStr[0] == SQLITE_INDEX_CONSTRAINT_EQ && _lastIdxString[0] == SQLITE_INDEX_CONSTRAINT_EQ)
	{
		_lastValues = values;
		_index = seekIndex(values[0], _colIndex); 
	}
	else
	{	
		_initialUse = false;
		std::string idx;
		if (idxStr != NULL)
			idx = std::string(idxStr);
		

		//Pull new data...
		_lastIdxNum = idxNum;
		_lastIdxString = idx;
		_lastValues = values;		


		//Clear variables...
		_index = -1;
		_set = DataSet();

		//Set up new range
		//Idxnum > 0 = ascending, < 0 = desc, 0 = no index column (full table pull)
		bool ascending = idxNum > 0;
		_range = createRange(ascending, _colIndex, _lastIdxString, _lastValues); 

		//Pull first set.
		getNextSet();

		if (argc > 0 && idxStr[0] == SQLITE_INDEX_CONSTRAINT_EQ)
			_index = seekIndex(values[0], _colIndex);
		
	}

	return SQLITE_OK;
}

int module::Cursor::seekIndex(std::string& value, int columnIndex)
{
	int lo = 0;
	int hi = int(_set.size()) - 1;
	int split = 0;
	int result = -1;

	while (lo <= hi)
	{
		split = (lo + hi) >> 1;
		result = value.compare(_set[split].Values[columnIndex].value);

		if (result == 0)
		{
			lo = split;
			break;
		}
		else if (result < 0)
			hi = split - 1;
		else
			lo = split + 1;
	}

	if (result == 0)
	{
		while (lo > 0 && value.compare(_set[lo -1].Values[columnIndex].value) == 0)
			--lo;

		return lo;
	}
	else
		return -1;
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
			//bound.Bound = values[0];
			//bound.Inclusive = true;
		
			//range.End = bound;
			//range.Start = bound;
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

int module::Cursor::convertSqliteValueToString(int col, sqlite3_value* arg, std::string& out)
{
	std::string type = _table->_declaredTypes[col];
	int converted = _table->tryConvertValue(arg, type, out);
	return converted;
}

int module::Cursor::getVector(int colIndex, int argc, sqlite3_value** args, std::vector<std::string>& out)
{
	std::vector<std::string> result(argc);
	for (int i = 0; i < argc; ++i)
	{
		int converted = convertSqliteValueToString(colIndex, args[i], result[i]);
		if (converted != SQLITE_OK)
			return converted;
	}

	out = result;

	return SQLITE_OK;
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

