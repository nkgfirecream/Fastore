#include "Store.h"

Store::Store(std::string path, uint64_t port, const boost::shared_ptr<Scheduler> pscheduler) :
	phandler( new StoreHandler(path, port)),
	pprocessor( new fastore::communication::StoreProcessor(phandler) ),
	config(port),
	pendpoint( new Endpoint(config, pprocessor) ),
	_status(idle)
{
	fastore::communication::StoreProcessor& processor(*pprocessor);

	processor.setEventHandler(phandler);

	run();
}

bool Store::run() 
{ 
	try 
	{ 
		pthread = boost::shared_ptr<boost::thread>(
			new boost::thread(std::mem_fun(&Endpoint::Run), pendpoint.get()) );
		_status = running;
	}
	//catch( std::exception& oops ) 
	//{
	//	_status = stopped;
	//	//clog << oops.what() << '\n';
	//}
	catch(...) 
	{
		_status = stopped;
		perror("indeterminate error");
	}
	return _status == running;
}