#include "FastoreService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

class FastoreService::impl
{
	boost::shared_ptr<TProcessor> _processor;
	boost::shared_ptr<TServerTransport> _serverTransport;
	boost::shared_ptr<TTransportFactory> _transportFactory;
	boost::shared_ptr<TProtocolFactory> _protocolFactory;
	boost::shared_ptr<TServer> _server;

public:

	impl(const ServiceConfig& config, const boost::shared_ptr<TProcessor> processor)	
	{
		_processor = processor;
		_serverTransport = boost::shared_ptr<TServerTransport>(new TServerSocket(config.port));
		_transportFactory = boost::shared_ptr<TTransportFactory>(new TBufferedTransportFactory());
		_protocolFactory = boost::shared_ptr<TProtocolFactory>(new TBinaryProtocolFactory());
		_server = boost::shared_ptr<TSimpleServer>(new TSimpleServer(_processor, _serverTransport, _transportFactory, _protocolFactory));
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
};

FastoreService::FastoreService(const ServiceConfig& config, const boost::shared_ptr<TProcessor>& processor) : _pimpl(new impl(config, processor))
{ }

void FastoreService::Run()
{
	_pimpl->Run();
}

void FastoreService::Stop()
{
	_pimpl->Stop();
}
