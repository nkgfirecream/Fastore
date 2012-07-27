#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <boost/shared_ptr.hpp>

namespace fastore
{
	class ClientException : public std::exception
	{
	public:
		enum class Codes
		{
			General = 0,
			/// <summary> There is no worker for the present column. </summary>
			NoWorkerForColumn = 10000,
			NoWorkersInHive = 10001

			// TODO: fill out rest of codes and update throw sites
		};

		Codes Code;
		std::unique_ptr<std::exception> Inner;

		ClientException();
		ClientException(const std::string &message);
		ClientException(const std::string &message, Codes code);
		ClientException(const std::string &message, std::unique_ptr<std::exception> &inner);

		static void ThrowErrors(std::vector<std::exception> &errors);

		static void ForceCleanup(vector<function<void>> statements);
	};
}
