#include "DataSet.h"

using namespace fastore::client;

DataSet::DataSet(int rows, int columnCount)
{
	_rows = std::vector<DataSetRow>(rows);
	_columnCount = columnCount;
}

DataSet::DataSet()
	: _columnCount(0) { }

DataSet::DataSetRow &DataSet::operator [](int index)
{
	auto result = _rows[index];
	if (result.Values.empty())
		result.Values = std::vector<std::string>(_columnCount);
	return result;
}

const int &DataSet::getCount() const
{
	return _rows.size();
}