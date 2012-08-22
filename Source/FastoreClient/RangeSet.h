#pragma once

#include "DataSet.h"
#include <boost/shared_ptr.hpp>

namespace fastore { namespace client
{
	class RangeSet
	{
	public:
		bool Eof;
		bool Bof;
		bool Limited;
		DataSet Data;
	};
}}
