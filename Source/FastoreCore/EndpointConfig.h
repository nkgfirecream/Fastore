#pragma once

struct EndpointConfig
{
	enum { DefaultPort = 8765 };

	int port;

	EndpointConfig( int port = DefaultPort) 
  : port(port)
	{}
};
