﻿#include "ServiceAddress.h"
#include <boost\algorithm\string.hpp>

using namespace fastore::client;

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

ServiceAddress ServiceAddress::ParseOne(const std::string &address)
{
	ServiceAddress result;

	std::vector<std::string> components;

	boost::split(components, address, boost::is_any_of(":"));

	//TODO: Erase empty strings. Couldn't find a good way to remove empty entries with boost::split
	for (auto iter = components.begin(); iter != components.end(); )
	{
		if (iter->empty())
			iter = components.erase(iter);
	}

	if (components.size() < 1 || components.size() > 2)
		throw std::exception("Must provide at least one address.");

	int port;

	if (components.size() < 2 || !intTryParse(port, components[1].c_str()))
		port = DefaultPort;

	result.setPort(port);

	boost::trim(components[0]);
	result.setName(components[0]);

	if (result.getName().empty())
		throw std::exception("Port is optional, but service host name must be given.");

	return result;
}

std::vector<ServiceAddress> ServiceAddress::ParseList(const std::string &composite)
{
	std::vector<std::string> addressstrings;

	boost::split(addressstrings, composite, boost::is_any_of(";"));

	//TODO: Erase empty strings. Couldn't find a good way to remove empty entries with boost::split
	for (auto iter = addressstrings.begin(); iter != addressstrings.end(); )
	{
		if (iter->empty())
			iter = addressstrings.erase(iter);
	}

	if (addressstrings.size() < 0)
		throw std::exception("Must provide at least one address.");

	std::vector<ServiceAddress> addresses;

	for (auto a : addressstrings)
	{
		addresses.push_back(ParseOne(a));
	}

	return addresses;
}

bool ServiceAddress::intTryParse (int &i, char const *s)
{
    char *end;
    long  l;
    errno = 0;
    l = strtol(s, &end, 0);
    if ((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) {
		return false;
    }
    if ((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) {
        return false;
    }
    if (*s == '\0' || *end != '\0') {
        return false;
    }
    i = l;
    return true;
}
