#pragma once
#include <Communication/Store.h>
#include "Endpoint.h"
#include "StoreHandler.h"

class Store
{
public:
	enum status_t { idle, running, stopped }; 
private:
	boost::shared_ptr<StoreHandler>  phandler;
	//boost::shared_ptr<StoreProcessor> pprocessor;
	const EndpointConfig config;
	boost::shared_ptr<Endpoint> pendpoint;
	status_t _status;
	boost::shared_ptr<boost::thread>  pthread;

public:
	Store
	(
		std::string path,
		uint64_t port,
		const boost::shared_ptr<Scheduler> pscheduler 
	);

	bool run();
	status_t status() const { return _status; }
};