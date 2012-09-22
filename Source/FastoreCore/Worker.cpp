// $Id$
#include "Worker.h"

static string id2str( PodID id ) 
{	// just a little temporary hack to help instantiate the Wal object
	ostringstream out;
	out << id;
	return out.str();
}


Worker::Worker( const PodID podId, 
				const string& path, 
				int port,
				const boost::shared_ptr<Scheduler> pscheduler ) 
	: phandler( new WorkerHandler(podId, path, pscheduler) )
	, pprocessor( new WorkerProcessor(phandler) )
	, config(port)
	, endpoint( config, pprocessor )
#if USE_WAL
	, _wal(path, id2str(podId), NetworkAddress() ) 
#endif
//	, thread( &Endpoint::Run )
{
	WorkerProcessor& processor(*pprocessor);

	processor.setEventHandler(phandler);
}

#if 0
void foo() {
	std::list<Worker> workers;
	std::vector<WorkerState> worker_states(3);
	std::vector<fastore::server::Path>  workerPaths(3);

	boost::shared_ptr<Scheduler> p;

	std::transform( worker_states.begin(), worker_states.end(), workerPaths.begin(), std::back_inserter(workers), Worker::InitWith(p) );
}
#endif
