#pragma once

struct EndpointConfig
{
	static const int DefaultPort = 8765;

	int port;

	EndpointConfig() : port(DefaultPort)
	{ }
};