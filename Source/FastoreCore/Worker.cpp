#include "Worker.h"

Worker::Worker(const PodID podId, uint64_t port):
	phandler(new WorkerHandler(podId)),
	pprocessor(new WorkerProcessor(phandler)),
	config(port),
	pendpoint(new Endpoint(config, pprocessor)),
	_status(idle)
{
	WorkerProcessor& processor(*pprocessor);

	processor.setEventHandler(phandler);

	run();
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