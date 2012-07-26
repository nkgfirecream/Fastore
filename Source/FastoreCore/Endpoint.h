#pragma once
#include <boost/shared_ptr.hpp>
#include "EndpointConfig.h"
#include <thrift/TProcessor.h>

class Endpoint
{
	class impl;
	boost::shared_ptr<impl> _pimpl;

public:
	Endpoint(const EndpointConfig& config, const boost::shared_ptr<apache::thrift::TProcessor>& processor);
	void Run();
	void Stop();

	boost::shared_ptr<apache::thrift::TProcessor> getProcessor();
	EndpointConfig getConfig();
};
