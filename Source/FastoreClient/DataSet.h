#pragma once
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <Communication/Comm_types.h>

namespace fastore { namespace client
{
	class DataSetRow
	{
	public:
		DataSetRow(size_t columns)
			: ID(std::string()), newGroup(true), Values(std::vector<fastore::communication::OptionalValue>(columns, fastore::communication::OptionalValue()))
		{ }

		std::vector<fastore::communication::OptionalValue> Values;
		std::string ID;
		
		// Indicates that the given row represents the beginning of a new value group for whatever ordering
		bool newGroup;

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
