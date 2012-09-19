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
}}
