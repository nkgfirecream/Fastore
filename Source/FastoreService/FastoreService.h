#pragma once
#include <boost/shared_ptr.hpp>
#include "ServiceConfig.h"

class FastoreService
{
	class impl;
	boost::shared_ptr<impl> _pimpl;

public:
	FastoreService(const ServiceConfig& config);
	void Run();
	void Stop();
};
