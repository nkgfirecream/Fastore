#include "Worker.h"

static string stringof( PodID id ) 
{	// just a little temporary hack to help instantiate the Wal object
	ostringstream out;
	out << id;
	return out.str();
}


Worker::Worker
( 
	const PodID podId, 
	const string& path, 
	uint64_t port,
	const boost::shared_ptr<Scheduler> pscheduler 
) 
	: _wal(path, stringof(podId), NetworkAddress() ) 
	, phandler( new WorkerHandler(podId, path, pscheduler, _wal) )
	, pprocessor( new WorkerProcessor(phandler) )
	, config(port)
	, pendpoint( new Endpoint(config, pprocessor) )
	, _status(idle)
{
	WorkerProcessor& processor(*pprocessor);

	processor.setEventHandler(phandler);
}

bool Worker::run() 
{ 
	try 
	{ 
		pthread = boost::shared_ptr<boost::thread>(
			new boost::thread(std::mem_fun(&Endpoint::Run), pendpoint.get()) );
		_status = running;
	}
	catch( std::exception& oops ) 
	{
		_status = stopped;
		clog << oops.what() << '\n';
	}
	catch(...) 
	{
		_status = stopped;
		perror("indeterminate error");
	}
	return _status == running;
}