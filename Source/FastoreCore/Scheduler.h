#pragma once
#include "../FastoreCommon/Comm_types.h"


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
private:

	fastore::communication::NetworkAddress _serviceAddress;


public:

	Scheduler(fastore::communication::NetworkAddress serviceAddress);

	//Start and stop the scheduler running. 
	//Need to think about potential problems and 
	//race conditions since it needs to talk to the service,
	//via the thrift interface, but the service owns are starts
	//the service. Furthermore, workers are going to start callbacks into the scheduler once they start running.

	//Once the scheduler is running, it should either ask the service about the state of the workers to find them,
	//or it should listen for callbacks from workers, or both...
	void start();
	void stop();

};