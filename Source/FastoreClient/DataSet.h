#pragma once

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>


namespace fastore { namespace client
{
	class DataSetRow
	{
	public:
		DataSetRow(int columns)
			: ID(std::string())
		{
			Values = std::vector<std::string>(columns, std::string());
		}

		std::vector<std::string> Values;
		std::string ID;

		std::string& operator[](int index)
		{
			return Values[index];
		}
	};


	class DataSet : public std::vector<DataSetRow>
	{

	private:
		int _columnCount;

	public:
		DataSet(int rows, int columnCount);
		DataSet() : _columnCount(0) { }

		int getColumnCount() { return _columnCount; }
	};
}}
