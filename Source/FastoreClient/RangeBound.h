#pragma once

#include <boost/shared_ptr.hpp>

namespace fastore { namespace client
{
	class RangeBound
	{
	public:
		std::string Bound;
		bool Inclusive;
	};
}}
