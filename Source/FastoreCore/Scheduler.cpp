#include "Scheduler.h"
#include <Communication/Store.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransport.h>
#include <thrift/transport/TBufferTransports.h>

Scheduler::Scheduler(fastore::communication::NetworkAddress serviceAddress) : 
	_serviceAddress(serviceAddress),
	_status(idle)
	{ }

bool Scheduler::start()
{
	_run = true;
	try 
	{ 
		_pthread = boost::shared_ptr<boost::thread>(
			new boost::thread(std::mem_fun(&Scheduler::run), this) );
		_status = running;
	}
	catch( std::exception& oops ) 
	{
		_status = stopped;
	}
	catch(...) 
	{
		_status = stopped;
		perror("indeterminate error");
	}
	return _status == running;
}

void Scheduler::stop()
{
	_run = false;
	_pthread->join();	
}

void Scheduler::run()
{
	while(_run)
	{
		//try sending the store a heartbeat. Consider using a connection pool.

		 boost::shared_ptr<apache::thrift::transport::TSocket> transport;

		try
		{
			transport = boost::shared_ptr<apache::thrift::transport::TSocket>(
								new apache::thrift::transport::TSocket(_serviceAddress.name, int(_serviceAddress.port + 1)));
			//TODO: Make this all configurable.
			transport->setConnTimeout(2000);
			transport->setRecvTimeout(2000);
			transport->setSendTimeout(2000);
			//transport->setMaxRecvRetries(MaxConnectionRetries);

			transport->open();
			auto bufferedTransport =  boost::shared_ptr<apache::thrift::transport::TTransport>(new apache::thrift::transport::TFramedTransport(transport));
			auto protocol = boost::shared_ptr<apache::thrift::protocol::TProtocol>(new apache::thrift::protocol::TBinaryProtocol(bufferedTransport));

			fastore::communication::StoreClient client(protocol);
			client.heartbeat();
		}
		catch (...)
		{
			transport->close();
		}

		usleep(INTERVAL);
	}

	_status = status_t::stopped;
}