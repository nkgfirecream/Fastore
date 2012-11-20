#include "Cursor.h"
//#include "../FastoreCore/safe_cast.h"
#include "../FastoreClient/Encoder.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace client = fastore::client;
namespace module = fastore::module;


struct dateTime
{
	  int64_t iJD; /* The julian day number times 86400000 */
	  int Y, M, D;       /* Year, month, and day */
	  int h, m;          /* Hour and minutes */
	  int tz;            /* Timezone offset in minutes */
	  double s;  
};/* Seconds */

//Compute year, month, day, etc. from the date structure.
//This assumes that iJD has been set correctly before hand.

static void computeYMD(dateTime *p){
	int Z, A, B, C, D, E, X1;
	Z = (int)((p->iJD + 43200000)/86400000);
	A = (int)((Z - 1867216.25)/36524.25);
	A = Z + 1 + A - (A/4);
	B = A + 1524;
	C = (int)((B - 122.1)/365.25);
	D = (36525*C)/100;
	E = (int)((B-D)/30.6001);
	X1 = (int)(30.6001*E);
	p->D = B - D - X1;
	p->M = E<14 ? E-1 : E-13;
	p->Y = p->M>2 ? C - 4716 : C - 4715;
}

/*
** Compute the Hour, Minute, and Seconds from the julian day number.
*/
static void computeHMS(dateTime *p){
  int s;
  //computeJD(p);
  s = (int)((p->iJD + 43200000) % 86400000);
  p->s = s/1000.0;
  s = (int)p->s;
  p->s -= s;
  p->h = s/3600;
  s -= p->h*3600;
  p->m = s/60;
  p->s += s - p->m*60;
}

/*
** Compute both YMD and HMS
*/
static void computeYMD_HMS(dateTime *p){
  computeYMD(p);
  computeHMS(p);
}

module::Cursor::Cursor(module::Table* table) : _table(table), _index(-1), _idxNum(0), _initialUse(true), _isEquality(false), _endOfFilter(true), _needsReset(true) { }

int module::Cursor::next()
{
	++_index;
	if (_index == _set.Data.size())
		getNextSet();

	//Special handling for the for equality case since we are reading ahead.
	if (_isEquality && !_endOfFilter) //If we have more data
	{
		//If we just pulled a fresh set, compare the value to our filter value
		if (_index == 0) 
		{
			auto valueType = _table->_columns[_colIndex].ValueType;

			//If the new value doesn't match the filter value, set End Of Filter.
			if (
					valueType.Compare
					(
						valueType.GetPointer(_set.Data[_index].Values[_colIndexToDataSetIndex[_colIndex]].value),
						valueType.GetPointer(_values[0])
					) != 0
				)
				_endOfFilter = true;
		}
		//Otherwise, just check for a new value
		else if (_set.Data[_index].newGroup)
			_endOfFilter = true;
	}

	return SQLITE_OK;
}

int module::Cursor::eof()
{
	return  _endOfFilter ? 1 : 0;
}

int module::Cursor::setColumnResult(sqlite3_context *pContext, int colIndex)
{	
	auto value = _set.Data[_index].Values[_colIndexToDataSetIndex[colIndex]];

	if (!value.__isset.value)
		sqlite3_result_null(pContext);

	auto declaredType = _table->_declaredTypes[colIndex];
	
	//Special handing for dates
	if (strcasecmp(declaredType.c_str(), "date") == 0 || strcasecmp(declaredType.c_str(), "datetime") == 0)
	{
		time_t epoch =  Encoder<int64_t>::Decode(value.value);
		
		//Setup julian day from the epoch. This code was copied from sqlite with very little modification. I'm not entirely sure
		//it's accurate. TODO: Test dates more extensively.
		dateTime d;
		d.iJD = (int64_t)((double)epoch * 86400000.0 + 0.5); 
		d.iJD = (d.iJD + 43200) / 86400 + 21086676 * (int64_t)10000000;

		//fill out the entire date structure completely
		computeYMD_HMS(&d);

		char buf[100];

		if (strcasecmp(declaredType.c_str(), "date") == 0)
		{
			sqlite3_snprintf(sizeof(buf), buf, "%04d-%02d-%02d", d.Y, d.M, d.D);
		}
		else
		{
			sqlite3_snprintf(sizeof(buf), buf, "%04d-%02d-%02d %02d:%02d:%02d", d.Y, d.M, d.D, d.h, d.m, (int)(d.s));
		}

		sqlite3_result_text(pContext, buf, -1, SQLITE_TRANSIENT);
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
	*pRowid = (sqlite3_int64)client::Encoder<int64_t>::Decode(_set.Data[_index].ID);

	return SQLITE_OK;
}

void module::Cursor::getNextSet()
{
	//First pull...
	if (_set.Data.size() == 0)
	{
		boost::optional<std::string> s;
		_set = _table->getRange(_range, _colUsed, s);
		if (_set.Data.size() > 0)
		{
			_index = 0;
			_endOfFilter = false;
		}
		else
		{
			_index = -1;
			_endOfFilter = true;
		}
	}
	//Continuation.
	else
	{
		if (_set.Eof) //end of Range -- This should be right for either equality conversions or normal calls
		{
			_index = -1;
			_endOfFilter = true;
		}
		else
		{
			//Iterating off the first page of data, set the flag.
			_needsReset = true;
			_set = _table->getRange(_range, _colUsed, boost::optional<std::string>(_set.Data[_index - 1].ID)); //Assumes index is currently at _set.Data.size() -- which it should be if this is the result of a next call
			if (_set.Data.size() > 0)
			{
				_index = 0;
				_endOfFilter = false;
			}
			else
			{
				_index = -1;
				_endOfFilter = true;
			}
		}
	}
}

int module::Cursor::filter(int idxNum, const char *idxStr, int argc, sqlite3_value **argv)
{
	//For equality constraints, try to figure out if we are inside of a loop.
	//If so, pull extra rows to as a read-ahead cache for query processor.
	//Fetch rows with >=, even if it's an equality constraint. Then detect if subsequent reads are in order.
	//TODO: If reverse order, change constraint to <= for future fetches. If no order, revert to == for future fetches.
	//(BONUS POINTS!) Make cursor self tune so that it pulls ~number of rows actually read by query processor.

	//idxNum is either colIdx + 1, ~(colIdx + 1) or 0 (0 = full table pull)
	_colIndex = idxNum > 0 ? idxNum - 1 : (idxNum < 0 ? (~idxNum) - 1 : 0); 

	std::vector<std::string> values;
	int result = getVector(_colIndex, argc, argv, values);

	//In case SQLITE gives us bad values to filter by (for example, a string in a integer column)
	if (result != SQLITE_OK)
		return result;

	std::string idx(idxStr);

	//Check and see if it's possible to reuse the cursor.
	if (!_initialUse && _idxNum == idxNum && _idxString == idx)
	{
		//If the index and operations are the same, the only thing that varies are the range values...
		
		//If we are an equality, try to find the value in the current set.
		if (_isEquality)
		{
			//Attempt to locate the correct value in the current set.
			//Presumably, this filtered occured after a Next call which iterated off the current value, so check and see if we are already pointing at the right value
			auto valueType = _table->_columns[_colIndex].ValueType;

			//if we are at the right place, set _eofOfFilter, create new range information (using the new values), and return
			if (_index >= 0 &&
					valueType.Compare
					(
						valueType.GetPointer(_set.Data[_index].Values[_colIndexToDataSetIndex[_colIndex]].value), 
						valueType.GetPointer(values[0])
					) == 0
				)
			{
				_values = values;
				_endOfFilter = false;
				createRange();
				return SQLITE_OK;
			}
			//Otherwise, try to find the correct value
			else
			{
				_index = seekIndex(values[0], _colIndex);
				//If we found the value, set new range and return
				if (_index >= 0)
				{
					_values = values;
					_endOfFilter = false;
					createRange();
					return SQLITE_OK;
				}
				//Otherwise fall through to  the cursor initialization below.  We need to pull a new set..
			}
		}
		//See if we can reset without pulling new data:
		//We can if the last time this cursor was filtered the parameters were exactly the same
		//And we haven't iterated off the first page of data.
		else if (compareVectors(_values, values) && !_needsReset)
		{
			//Rewind to first value and reset flags.
			_index = _set.Data.size() > 0 ? 0 : -1;
			_endOfFilter = _index < 0; 
			_needsReset = false;
			return SQLITE_OK;
		}
		//Otherwise,  fall through to the below and pull new data.
	}


	//Clear variables...
	_index = -1;
	_set =  RangeSet();
	_colIndexToDataSetIndex.clear();
	_colUsed.clear();

	//Set up a new cursor
	_initialUse = false;
	_needsReset = false;
	_idxNum = idxNum;
	_idxString = idx;
	_values = values;
		

	//Set up new range -- This has the side effect of setting _range and the _isEquality flag.
	createRange(); 



	//Get bitmask from string
	auto maskStart = idx.find(';');
	auto bitString = idx.substr(maskStart + 1);
	std::istringstream bitStream(bitString);

	int64_t bitMask;
	bitStream >> bitMask;
	

	//Set up map for all the columns
	int dataSetIndex = 0;
	for (int64_t i = 0; (i < 64 && (i < int(_table->_columns.size()))); i++)
	{
		auto used = bitMask & (1i64 << i);
		if (used || _range.ColumnID == _table->_columns[i].ColumnID)
		{
			_colIndexToDataSetIndex[int(i)] = dataSetIndex;
			_colUsed.push_back(_table->_columns[i].ColumnID);
			++dataSetIndex;
		}
	}

	//Map all the rest of the columns if column 64 is used
	//(this is how SQLite handles it internally. we'll need to change
	//its source to handle more than 64 columns.
	auto highbit = bitMask & (1i64 << 63);
	if (highbit)
	{
		for (int64_t i = 64; i < int(_table->_columns.size()); i++)
		{
			_colIndexToDataSetIndex[int(i)] = dataSetIndex;
			_colUsed.push_back(_table->_columns[i].ColumnID);
			++dataSetIndex;
		}
	}



		
	//Pull first set.
	getNextSet();

	return SQLITE_OK;
}

int module::Cursor::seekIndex(std::string& value, int columnIndex)
{
	int lo = 0;
	int hi = int(_set.Data.size()) - 1;
	int split = 0;
	int result = -1;

	auto valueType = _table->_columns[_colIndex].ValueType;

	while (lo <= hi)
	{
		split = (lo + hi) >> 1;
		result = valueType.Compare
				(
					valueType.GetPointer(value), 
					valueType.GetPointer(_set.Data[split].Values[_colIndexToDataSetIndex[columnIndex]].value)
				);

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
		while (lo > 0 && !_set.Data[lo].newGroup)
			--lo;

		return lo;
	}
	else
		return -1;
}


void module::Cursor::createRange()
{
	client::Range range;
	
	range.Ascending = _idxNum >= 0;	
	//TODO: Figure out how to grab all the rowIds when there isn't a key column. 
	//Should this be masked from SQLite? 
	//Should the client support a "get all keys" operation and we iterate over them?
	//Should we force at least one required column?
	
	range.ColumnID = _table->_columns[_colIndex].ColumnID;

	if (_values.size() > 0)
	{
		if (_idxString[0] == SQLITE_INDEX_CONSTRAINT_EQ)
		{
			_isEquality = true;
			client::RangeBound bound;
			bound.Bound = _values[0];
			bound.Inclusive = true;
			range.Start = bound;
			//range.End = bound;

		}
		else
		{
			_isEquality = false;
			for (int i = 0; i < _values.size(); i++)
			{
				client::RangeBound bound = getBound(_colIndex, _idxString[i], _values[i]);
				if (range.Ascending)
				{
					if (_idxString[i] == SQLITE_INDEX_CONSTRAINT_GT || _idxString[i] == SQLITE_INDEX_CONSTRAINT_GE)
						range.Start = bound;
					else
						range.End = bound;
				}
				else
				{
					if (_idxString[i] == SQLITE_INDEX_CONSTRAINT_GT || _idxString[i] == SQLITE_INDEX_CONSTRAINT_GE)
						range.End = bound;
					else
						range.Start = bound;
				}
			}
		}
	}
	else
	{
		_isEquality = false;
	}

	_range = range;
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

