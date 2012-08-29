#pragma once

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include "../FastoreCommunication/Comm_types.h"


namespace fastore { namespace client
{
	class DataSetRow
	{
	public:
		DataSetRow(int columns)
			: ID(std::string())
		{
			Values = std::vector<fastore::communication::OptionalValue>(columns, fastore::communication::OptionalValue());
		}

		std::vector<fastore::communication::OptionalValue> Values;
		std::string ID;

		fastore::communication::OptionalValue& operator[](int index)
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
