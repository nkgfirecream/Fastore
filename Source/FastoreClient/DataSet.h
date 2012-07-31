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
		DataSet();

		DataSetRow& operator [](int index);

		const int& getCount() const;

		std::vector<DataSetRow>::iterator begin()
		{
			return _rows.begin();
		}

		std::vector<DataSetRow>::iterator end()
		{
			return _rows.end();
		}
	};
}}
