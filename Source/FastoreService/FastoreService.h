#pragma once
#include <boost/shared_ptr.hpp>

class FastoreService
{
	class impl;
	boost::shared_ptr<impl> _pimpl;

public:
	FastoreService();
	void Run();
	void Stop();
};
