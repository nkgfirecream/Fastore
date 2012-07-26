#include "ServiceAddress.h"

using namespace fastore;

const int &ServiceAddress::getPort() const
{
	return privatePort;
}

void ServiceAddress::setPort(const int &value)
{
	privatePort = value;
}

const std::string &ServiceAddress::getName() const
{
	return privateName;
}

void ServiceAddress::setName(const std::string &value)
{
	privateName = value;
}

boost::shared_ptr<ServiceAddress> ServiceAddress::ParseOne(const std::string &address)
{
	boost::shared_ptr<ServiceAddress> result = boost::shared_ptr<ServiceAddress>();

//C# TO C++ CONVERTER TODO TASK: There is no direct native C++ equivalent to the .NET String 'Split' method:
	auto components = address.Split(new wchar_t[] {':'}, StringSplitOptions::RemoveEmptyEntries);
	if (sizeof(components) / sizeof(components[0]) < 1 || sizeof(components) / sizeof(components[0]) > 2)
		throw std::exception("Must provide at least one address.");

	int port;
	if (sizeof(components) / sizeof(components[0]) < 2 || !int::TryParse(components[1], port))
		port = DefaultPort;

	result->setPort(port);
//C# TO C++ CONVERTER TODO TASK: There is no direct native C++ equivalent to the .NET String 'Trim' method:
	result->setName(components[0]->Trim());

	if (result->getName().empty())
		throw std::exception("Port is optional, but service host name must be given.");

	return result;
}

ServiceAddress *ServiceAddress::ParseList(const std::string &composite)
{
//C# TO C++ CONVERTER TODO TASK: There is no direct native C++ equivalent to the .NET String 'Split' method:
	auto addresses = composite.Split(new wchar_t[] {';'}, StringSplitOptions::RemoveEmptyEntries);
	if (sizeof(addresses) / sizeof(addresses[0]) < 1)
		throw std::exception("Must provide at least one address.");
	return (from a in addresses select ParseOne(a))->ToArray();
}
