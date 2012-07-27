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
		static const int DefaultPort = 8765;		
		static ServiceAddress ParseOne(const std::string &address);
		static std::vector<ServiceAddress> ParseList(const std::string &composite);

	private:
		int privatePort;
		std::string privateName;

	public:
		const int &getPort() const;
		void setPort(const int &value);
		const std::string &getName() const;
		void setName(const std::string &value);

	};
}}
