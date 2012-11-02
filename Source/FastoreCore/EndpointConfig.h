#pragma once

struct EndpointConfig
{
	enum { DefaultPort = 8765 };

	uint64_t port;

	EndpointConfig( uint64_t port = DefaultPort) 
  : port(port)
	{}
};
