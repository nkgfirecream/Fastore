#include "Range.h"

fastore::communication::RangeRequest fastore::client::rangeToRangeRequest(const Range& range, const int limit)
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