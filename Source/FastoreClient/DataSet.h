#pragma once

#include <boost/shared_ptr.hpp>

namespace fastore
{
	class DataSet
	{
	public:
		class DataSetRow
		{
		public:
			object *Values;
			boost::shared_ptr<object> ID;
		};

	private:
		DataSetRow *_rows;
		int _columnCount;

	public:
		DataSet(int rows, int columnCount);

		DataSetRow &operator [](int index);

		const int &getCount() const;

		boost::shared_ptr<IEnumerator<DataSetRow>> GetEnumerator();

		boost::shared_ptr<IEnumerator> IEnumerable_GetEnumerator();

	};
}
