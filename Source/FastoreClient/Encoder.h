#pragma once

#include "../FastoreCommon/Comm_types.h"
#include "../FastoreCore/safe_cast.h"

using namespace fastore::communication;

namespace fastore { namespace client
{
	//TODO: Make the encoder class more flexible so we don't have to rewrite code
	//if the types change for ColumnIds, HostIds, PodIds, etc.
	template <typename T>
	class Encoder
	{
	public:
		static std::string Encode(const T& item)
		{
			std::string result;
			result.assign((const char*)&item, sizeof(T));
			return result;
		}

		static std::string Crunch(size_t input)
		{
			const T item( SAFE_CAST(T, input) );
			std::string result;
			result.assign((const char*)&item, sizeof(T));
			return result;
		}

		static T Decode(const std::string& item)
		{
			return T(*(T*)item.data());
		}
	};

	template<>
	class Encoder<std::string>
	{
	public:
		static std::string Encode(const std::string& item)
		{
			return item;
		}

		static std::string Decode(const std::string& item)
		{
			return item;
		}
	};	
}}
