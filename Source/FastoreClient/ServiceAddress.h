#pragma once

#include <string>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace fastore { namespace client
{
	class ServiceAddress
	{
	public:
		ServiceAddress();

		static const int DefaultPort = 8765;		
		static ServiceAddress ParseOne(const std::string &address);
		static std::vector<ServiceAddress> ParseList(const std::string &composite);

		int Port;
		std::string Name;	

	private:
		static bool intTryParse (int &i, char const *s);
	};
}}
