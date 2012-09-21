#include "Endpoint.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TServerSocket.h>
#include "TFastoreServer.h"

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
	EndpointConfig _config;

public:

	impl(const EndpointConfig& config, const boost::shared_ptr<TProcessor> processor) : _config(config)	
	{
		_processor = processor;
		_protocolFactory = boost::shared_ptr<TProtocolFactory>(new TBinaryProtocolFactory());
		_server = boost::shared_ptr<TFastoreServer>(new TFastoreServer(_processor, _protocolFactory, _config.port));
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

	EndpointConfig getConfig()
	{
		return _config;
	}
};

Endpoint::Endpoint(const EndpointConfig& config, const boost::shared_ptr<TProcessor>& processor) 
		: _pimpl(new impl(config, processor))
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

EndpointConfig Endpoint::getConfig()
{
	return _pimpl->getConfig();
}
