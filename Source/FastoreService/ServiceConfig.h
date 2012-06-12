#pragma once
const int DefaultPort = 8064;

struct ServiceConfig
{
	int port;

	ServiceConfig() : port(DefaultPort)
	{ }
};