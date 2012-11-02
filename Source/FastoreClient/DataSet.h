#pragma once

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "../FastoreCommon/Comm_types.h"


namespace fastore { namespace client
{
	class DataSetRow
	{
	public:
		DataSetRow(size_t columns)
			: ID(std::string())
		{
			Values = std::vector<fastore::communication::OptionalValue>(columns, fastore::communication::OptionalValue());
		}

		std::vector<fastore::communication::OptionalValue> Values;
		std::string ID;

		fastore::communication::OptionalValue& operator[](size_t index)
		{
			return Values[index];
		}
	};


	class DataSet : public std::vector<DataSetRow>
	{

	private:
		size_t _columnCount;

	public:
		DataSet(size_t rows, size_t columnCount);
		DataSet() : _columnCount(0) { }

		size_t getColumnCount() { return _columnCount; }
	};
}}
