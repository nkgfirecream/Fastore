#include "Cursor.h"
#include "../FastoreClient/Encoder.h"

#define SAFE_CAST(t,f) safe_cast(__FILE__, __LINE__, (t), (f))

template <typename T, typename F>
T safe_cast(const char file[], size_t line, T, F input) {
  using std::numeric_limits;
  std::ostringstream msg;

  if( numeric_limits<F>::is_signed && !numeric_limits<T>::is_signed ) {
    if( input < 0 ) {
      msg << file << ":" << line << ": " 
	  << "signed value " << input << " cannot be cast to unsigned type";
      throw std::runtime_error(msg.str());
    }
    if( numeric_limits<T>::max() < static_cast<size_t>(input) ) {
      msg << file << ":" << line << ": " 
	  << input << ", size " << sizeof(F) 
	  << ", cannot be cast to unsigned type of size" << sizeof(T);
      throw std::runtime_error(msg.str());
    }
  }
  return static_cast<T>(input);
}

using namespace fastore::module;
namespace client = fastore::client;

Cursor::Cursor(fastore::module::Table* table) : _table(table), _index(-1) { }

void Cursor::next()
{
	++_index;
	if (SAFE_CAST(size_t(), _index) == _set.Data.size())
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

void Cursor::setRowId(sqlite3_int64 *pRowid)
{
	*pRowid = (sqlite3_int64)client::Encoder<int>::Decode(_set.Data[_index].ID);
}

void Cursor::getNextSet()
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

void Cursor::filter(int idxNum, const char *idxStr, int argc, sqlite3_value **argv)
{
	//Clear variables...
	_index = -1;
	_set = RangeSet();
	//The goal here is to set up our private _range and then load data.
	//_range = something...

	_range = Range();
#if WHEN_WE_CARE_ABOUT_range_Ascending
	_range.Ascending; // = idxNum != 0;
#endif
	_range.ColumnID = _table->_columns[0].ColumnID;

	getNextSet();
}


