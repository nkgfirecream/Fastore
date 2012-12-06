#pragma once

#include "DataSet.h"
#include "RangeSet.h"
#include "Range.h"
#include <boost/shared_ptr.hpp>
#include <map>
#include "../FastoreCommon/Communication/Comm_types.h"

using namespace fastore::communication;

namespace fastore { namespace client
{	
	class IDataAccess
	{
	public:
		virtual RangeSet getRange(const ColumnIDs& columnIds, const Range& range, const int limit, const boost::optional<std::string> &startId) = 0;
		virtual DataSet getValues(const ColumnIDs& columnIds, const std::vector<std::string>& rowIds) = 0;

		virtual void include(const ColumnIDs& columnIds, const std::string& rowId, const std::vector<std::string>& row) = 0;
		virtual void exclude(const ColumnIDs& columnIds, const std::string& rowId) = 0;

		virtual std::vector<Statistic> getStatistics(const ColumnIDs& columnIds) = 0;
	};
}}
