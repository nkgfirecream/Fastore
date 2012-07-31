#pragma once

#include "DataSet.h"
#include "RangeSet.h"
#include "Range.h"
#include <boost/shared_ptr.hpp>
#include <map>
#include "..\FastoreCommunication\Comm_types.h"

using namespace fastore::communication;

namespace fastore { namespace client
{	
	class IDataAccess
	{
	public:
		virtual RangeSet GetRange(std::vector<int>& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId) = 0;
		virtual DataSet GetValues(const std::vector<int>& columnIds, const std::vector<std::string>& rowIds) = 0;

		virtual void Include(const std::vector<int>& columnIds, const std::string& rowId, const std::vector<std::string>& row) = 0;
		virtual void Exclude(const std::vector<int>& columnIds, const std::string& rowId) = 0;

		virtual std::vector<Statistic> GetStatistics(const std::vector<int>& columnIds) = 0;

		virtual std::map<int, long long> Ping() = 0;
	};
}}
