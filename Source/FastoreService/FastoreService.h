#pragma once
#include <boost/shared_ptr.hpp>
#include "ServiceConfig.h"
#include <thrift/TProcessor.h>

class FastoreService
{
	class impl;
	boost::shared_ptr<impl> _pimpl;

public:
	FastoreService(const ServiceConfig& config, const boost::shared_ptr<apache::thrift::TProcessor>& processor);
	void Run();
	void Stop();
};
