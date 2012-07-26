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
			/// <summary> There is no worker for the present column. </summary>
			NoWorkerForColumn = 10000,
			NoWorkersInHive = 10001

			// TODO: fill out rest of codes and update throw sites
		};

	public:
		ClientException();
		ClientException(const std::string &message);
		ClientException(const std::string &message, Codes code);

	public:
		ClientException(const std::string &message, std::exception &innerException);

		static void ThrowErrors(std::vector<std::exception> &errors);

//ORIGINAL LINE: public static void ForceCleanup(params Action[] actions)
//C# TO C++ CONVERTER TODO TASK: Use 'va_start', 'va_arg', and 'va_end' to access the parameter array within this method:
		static void ForceCleanup(...);
	};
}
