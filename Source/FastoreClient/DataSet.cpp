#include "DataSet.h"

using namespace fastore::client;

DataSet::DataSet(size_t rows, size_t columnCount)
	: _columnCount(columnCount), std::vector<DataSetRow>(rows, DataSetRow(columnCount)) { }