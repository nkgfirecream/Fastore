#include "ClientException.h"
#include <functional>
#include <vector>

#include "../FastoreCore/Log/Syslog.h"

using namespace fastore::client;
using namespace std;

using fastore::Log;
using fastore::log_endl;
using fastore::log_info;
using fastore::log_err;

ClientException::ClientException() 
    : std::runtime_error(std::string()), Code(Codes::General)
{
}

ClientException::ClientException(const std::string &message) 
  : std::runtime_error(message), Code(Codes::General)
{
}

ClientException::ClientException(const std::string &message, Codes code) 
  : std::runtime_error(message), Code(code)
{
}

ClientException::ClientException(const std::string &message, std::exception &inner) 
  : std::runtime_error(message), Code(Codes::General), Inner(inner)
{
}

void ClientException::ThrowErrors(std::vector<std::exception> &errors)
{
	if (errors.size() > 0)
	{
		// TODO: implement aggregate exceptions
		Log << log_info << "ClientException::" << __func__ 
			<< ": throwing 1 of " << errors.size() << " errors" << log_endl;
		throw errors[0];
	}
}

void ClientException::ForceCleanup(vector<function<void()>> statements)
{
	auto errors = vector<exception>();
	for (function<void()> statement : statements)
	{
		try
		{
			statement();
		}
		catch (std::exception &e)
		{
			errors.push_back(e);
		}
	}
	ThrowErrors(errors);
}
