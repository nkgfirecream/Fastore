#pragma once

struct EndpointConfig
{
	static const int DefaultPort = 8064;

	int port;

	EndpointConfig() : port(DefaultPort)
	{ }
};