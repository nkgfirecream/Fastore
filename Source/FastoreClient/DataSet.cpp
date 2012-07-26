#include "DataSet.h"

using namespace fastore;

DataSet::DataSet(int rows, int columnCount)
{
	_rows = new DataSetRow[rows];
	_columnCount = columnCount;
}

DataSet::DataSetRow &DataSet::operator [](int index)
{
	auto result = _rows[index];
	if (result.Values == nullptr)
		result.Values[index] = new object[_columnCount];
	return result;
}

//C# TO C++ CONVERTER TODO TASK: You cannot specify separate 'set' logic for indexers in native C++:
//			void DataSet::setdefault(const int &index, DataSetRow value)
//			{
//				_rows[index] = value;
//			}

const int &DataSet::getCount() const
{
	return sizeof(_rows) / sizeof(_rows[0]);
}

boost::shared_ptr<IEnumerator<DataSetRow>> DataSet::GetEnumerator()
{
	for (int i = 0; i < sizeof(_rows) / sizeof(_rows[0]); i++)
//C# TO C++ CONVERTER TODO TASK: C++ does not have an equivalent to the C# 'yield' keyword:
		yield return this->operator[](i);
}

boost::shared_ptr<IEnumerator> DataSet::IEnumerable_GetEnumerator()
{
	return this->GetEnumerator();
}