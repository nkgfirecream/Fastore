#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <functional>

namespace fastore { namespace client
{
    class ClientException : public std::runtime_error
	{
	public:
		enum class Codes
		{
			OK = 0,

			// General error
			General = 1,

			// There is no worker for the present column. 
			NoWorkerForColumn = 10000,
			// There are no workers configured for the given column.
			NoWorkersInHive = 10001

			// TODO: fill out rest of codes and update throw sites
		};

		Codes code;
		std::exception Inner;

		ClientException();
		ClientException(const std::string &message);
		ClientException(const std::string &message, Codes code);
		ClientException(const std::string &message, std::exception &inner);

		static void ThrowErrors(std::vector<std::exception> &errors);

		static void ForceCleanup(std::vector<std::function<void()>> statements);
	};
}}
