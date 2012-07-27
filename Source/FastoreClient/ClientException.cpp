#include "ClientException.h"
#include <functional>
#include <vector>

using namespace fastore;
using namespace std;

ClientException::ClientException() : Code(Codes::General)
{
}

ClientException::ClientException(const std::string &message) : std::exception(message.c_str()), Code(Codes::General)
{
}

ClientException::ClientException(const std::string &message, Codes code) : std::exception(message.c_str()), Code(code)
{
}

ClientException::ClientException(const std::string &message, std::unique_ptr<std::exception> &inner) : std::exception(message.c_str()), Code(Codes::General), Inner(innerException)
{
}

void ClientException::ThrowErrors(std::vector<std::exception> &errors)
{
	if (errors.size() > 0)
	{
		throw errors[0];
		// TODO: implement aggregate exceptions
	}
}

void ClientException::ForceCleanup(vector<function<void>> statements)
{
	auto errors = vector<exception>();
	for (function<void> statement : statements)
		try
		{
			statement();
		}
		catch (std::exception &e)
		{
			errors.push_back(e);
		}
	ThrowErrors(errors);
}
