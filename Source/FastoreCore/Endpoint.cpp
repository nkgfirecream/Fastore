#include "Endpoint.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/concurrency/BoostThreadFactory.h>
#include "TMultiConnectionServer.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

class Endpoint::impl
{
	boost::shared_ptr<TProcessor> _processor;
	boost::shared_ptr<TServerTransport> _serverTransport;
	boost::shared_ptr<TTransportFactory> _transportFactory;
	boost::shared_ptr<TProtocolFactory> _protocolFactory;
	boost::shared_ptr<TServer> _server;

public:

	impl(const EndpointConfig& config, const boost::shared_ptr<TProcessor> processor)	
	{
		_processor = processor;
		_serverTransport = boost::shared_ptr<TServerTransport>(new TServerSocket(config.port));
		_transportFactory = boost::shared_ptr<TTransportFactory>(new TBufferedTransportFactory());
		_protocolFactory = boost::shared_ptr<TProtocolFactory>(new TBinaryProtocolFactory());

		//boost::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(15);
		//boost::shared_ptr<BoostThreadFactory> threadFactory = boost::shared_ptr<BoostThreadFactory>(new BoostThreadFactory());
		//threadManager->threadFactory(threadFactory);
		//threadManager->start();

		_server = boost::shared_ptr<TMultiConnectionServer>(new TMultiConnectionServer(_processor, _serverTransport, _transportFactory, _protocolFactory));
	}

	~impl()
	{
		Stop();
	}

	void Run()
	{
		_server->serve();
	}

	void Stop()
	{
		_server->stop();
	}

	boost::shared_ptr<apache::thrift::TProcessor> getProcessor()
	{
		return _processor;
	}
};

Endpoint::Endpoint(const EndpointConfig& config, const boost::shared_ptr<TProcessor>& processor) : _pimpl(new impl(config, processor))
{ }

void Endpoint::Run()
{
	_pimpl->Run();
}

void Endpoint::Stop()
{
	_pimpl->Stop();
}

boost::shared_ptr<apache::thrift::TProcessor> Endpoint::getProcessor()
{
	return _pimpl->getProcessor();
}
