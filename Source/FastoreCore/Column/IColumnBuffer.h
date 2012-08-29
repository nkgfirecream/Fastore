#pragma once

#include "../Schema/scalar.h"
#include "../../FastoreCommunication/Comm_types.h"
#include <vector>

using namespace fastore::communication;

class IColumnBuffer
{
	public:		
		virtual void Apply(const ColumnWrites& writes) = 0;
		virtual RangeResult GetRows(const RangeRequest& range) = 0;
		virtual std::vector<OptionalValue> GetValues(const std::vector<std::string>& rowIds) = 0;
		virtual Statistic GetStatistic() = 0;
};
