#pragma once

#include "DataSet.h"
#include <boost/shared_ptr.hpp>
#include <map>
#include "..\FastoreCommunication\Comm_types.h"

using namespace fastore::communication;

namespace fastore
{	
	class IDataAccess
	{
	public:
		virtual RangeResult GetRange(RangeRequest range) = 0;
		virtual std::vector<std::string> GetValues(std::vector<int> columnIds, std::vector<std::string> rowIds) = 0;

		virtual void Include(std::vector<int> columnIds, std::string rowId, std::vector<std::string> row) = 0;
		virtual void Exclude(std::vector<int> columnIds, std::string rowId) = 0;

		virtual std::vector<Statistic> GetStatistics(std::vector<int> columnIds) = 0;

		//virtual std::map<int, TimeSpan> Ping() = 0;
	};
}
