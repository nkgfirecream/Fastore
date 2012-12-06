#pragma once

#include "RangeBound.h"
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

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

	fastore::communication::RangeRequest rangeToRangeRequest(const Range& range, const int limit)
	{
		fastore::communication::RangeRequest rangeRequest;
		rangeRequest.__set_ascending(range.Ascending);
		rangeRequest.__set_limit(limit);

		if (range.Start)
		{
			fastore::communication::RangeBound bound;
			bound.__set_inclusive(range.Start->Inclusive);
			bound.__set_value(range.Start->Bound);

			rangeRequest.__set_first(bound);
		}

		if (range.End)
		{
			fastore::communication::RangeBound bound;
			bound.__set_inclusive(range.End->Inclusive);
			bound.__set_value(range.End->Bound);

			rangeRequest.__set_last(bound);
		}

		return rangeRequest;
	}
}}
