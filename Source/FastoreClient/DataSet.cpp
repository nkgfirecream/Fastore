#include "DataSet.h"

using namespace fastore::client;

DataSet::DataSet(int rows, int columnCount)
	: _columnCount(columnCount), std::vector<DataSetRow>(rows, DataSetRow(columnCount)) { }