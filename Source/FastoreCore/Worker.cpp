// $Id$
#include "Worker.h"

static string stringof( PodID id ) 
{	// just a little temporary hack to help instantiate the Wal object
	ostringstream out;
	out << id;
	return out.str();
}


Worker::Worker( const PodID podId, 
				const string& path, 
				uint64_t port,
				const boost::shared_ptr<Scheduler> pscheduler ) 
	: _wal(path, stringof(podId), NetworkAddress() ) 
	,  phandler( new WorkerHandler(podId, path, pscheduler, _wal) )
	, pprocessor( new WorkerProcessor(phandler) )
	, config(INT_CAST(port))
	, endpoint( config, pprocessor )
	, _status(idle)
{
	WorkerProcessor& processor(*pprocessor);

	processor.setEventHandler(phandler);
}

bool 
Worker::run() { 
	try { 
		pthread = boost::shared_ptr<boost::thread>(
			new boost::thread(std::mem_fun(&Endpoint::Run), &endpoint) );
		_status = running;
	}
	catch( std::exception& oops ) {
		_status = stopped;
		clog << oops.what() << '\n';
	}
	catch(...) {
		_status = stopped;
		perror("indeterminate error");
	}
	return _status == running;
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
