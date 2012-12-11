#pragma once
#include <functional>
#include <boost/thread.hpp>
#include "Endpoint.h"
#include "WorkerHandler.h"
#include <Communication/Server_types.h>

class Worker
{
public:
	enum status_t { idle, running, stopped }; 
private:
	boost::shared_ptr<WorkerHandler>  phandler;
	WorkerState state;
	boost::shared_ptr<WorkerProcessor> pprocessor;
	const EndpointConfig config;
	boost::shared_ptr<Endpoint> pendpoint;
	status_t _status;
	boost::shared_ptr<boost::thread>  pthread;

public:
	Worker
	( 
		const PodID podId, 
		uint64_t port
	);

	bool run();
	status_t status() const { return _status; }
};

