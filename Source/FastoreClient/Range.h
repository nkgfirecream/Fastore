#pragma once

#include "RangeBound.h"
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <Communication\Comm_types.h>

namespace fastore { namespace client
{
	class Range
	{
	public:
		int64_t ColumnID;
		boost::optional<RangeBound> Start;
		boost::optional<RangeBound> End;
		bool Ascending;
	};

	fastore::communication::RangeRequest rangeToRangeRequest(const Range& range, const int limit);
}}
