// $Id$
#include "Worker.h"

Worker::Worker( const PodID podId, 
				const string& path, 
				int port,
				const boost::shared_ptr<Scheduler> pscheduler ) 
	: phandler( new WorkerHandler(podId, path, pscheduler) )
	, pprocessor( new WorkerProcessor(phandler) )
	, config(port)
	, endpoint( config, pprocessor )
{
	WorkerProcessor& processor(*pprocessor);

	processor.setEventHandler(phandler);
}

#include <algorithm>

void foo() {
	std::list<Worker> workers;
	std::vector<WorkerState> worker_states(3);
	std::vector<fastore::server::Path>  workerPaths(3);

	boost::shared_ptr<Scheduler> p;

	std::transform( worker_states.begin(), worker_states.end(), workerPaths.begin(), std::back_inserter(workers), Worker::InitWith(p) );
}
