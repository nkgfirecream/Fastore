#include "DataSet.h"

using namespace fastore::client;

DataSet::DataSet(int rows, int columnCount)
{
	_rows = std::vector<DataSetRow>(rows);
	_columnCount = columnCount;
}

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

//boost::shared_ptr<IEnumerator<DataSetRow>> DataSet::GetEnumerator()
//{
//	for (int i = 0; i < sizeof(_rows) / sizeof(_rows[0]); i++)
////C# TO C++ CONVERTER TODO TASK: C++ does not have an equivalent to the C# 'yield' keyword:
//		yield return this->operator[](i);
//}
//
//boost::shared_ptr<IEnumerator> DataSet::IEnumerable_GetEnumerator()
//{
//	return this->GetEnumerator();
//}