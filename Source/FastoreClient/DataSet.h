#pragma once

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>


namespace fastore { namespace client
{
	class DataSet
	{
	public:
		class DataSetRow
		{
		public:
			std::vector<std::string> Values;
			std::string ID;
		};

	private:
		std::vector<DataSetRow> _rows;
		int _columnCount;

	public:
		DataSet(int rows, int columnCount);

		DataSetRow& operator [](int index);

		const int& getCount() const;

		//boost::shared_ptr<IEnumerator<DataSetRow>> GetEnumerator();

		//boost::shared_ptr<IEnumerator> IEnumerable_GetEnumerator();

	};
}}
