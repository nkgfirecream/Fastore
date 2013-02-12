#pragma once
#include "../FastoreCommon/Communication/Comm_types.h"
#include <boost/thread.hpp>


// The purpose of the scheduler is to manage events that must happen
// on the service on intervals. Since the workers are single threaded,
// they are not able to wake at defined intervals. The scheduler can wake
// and send messages to the workers.

// Examples of potential actions the scheduler might do:
// Coordinating checkpointing
// Worker repair
// Log management/batching

class Scheduler
{
public:
	enum status_t { idle, running, stopped };
	Scheduler(fastore::communication::NetworkAddress serviceAddress);

	//Start and stop the scheduler running. 
	//Need to think about potential problems and 
	//race conditions since it needs to talk to the service,
	//via the thrift interface, but the service owns are starts
	//the service. Furthermore, workers are going to start callbacks into the scheduler once they start running.

	//Once the scheduler is running, it should either ask the service about the state of the workers to find them,
	//or it should listen for callbacks from workers, or both...
	bool start();
	void stop();
	status_t status() const { return _status; }

private:

	const static int INTERVAL = 1000;

	fastore::communication::NetworkAddress _serviceAddress;
	boost::shared_ptr<boost::thread>  _pthread;
	bool _run;
	status_t _status;

	void run();

};