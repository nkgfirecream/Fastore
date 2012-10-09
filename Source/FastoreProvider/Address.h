#pragma once
#include <string>

namespace fastore
{
	namespace provider
	{
		struct Address
		{
			std::string Name;
			uint64_t Port;
		};
	}
}