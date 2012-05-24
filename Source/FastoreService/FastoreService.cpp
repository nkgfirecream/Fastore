#define WIN32_LEAN_AND_MEAN
#include "FastoreService.h"
#include "ServiceHandler.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadPoolServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PlatformThreadFactory.h>

using namespace std;

class FastoreService::impl
{
	boost::shared_ptr<ServiceHandler> _handler;
	boost::shared_ptr<TProcessor> _processor;
	boost::shared_ptr<TServerTransport> _serverTransport;
	boost::shared_ptr<TTransportFactory> _transportFactory;
	boost::shared_ptr<TProtocolFactory> _protocolFactory;
	boost::shared_ptr<ThreadManager> _threadManager;
	boost::shared_ptr<concurrency::PlatformThreadFactory> _threadFactory;
	boost::shared_ptr<TServer> _server;

public:

	impl()	
	{
		//Open windows sockets
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;

		/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
		wVersionRequested = MAKEWORD(2, 2);

		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) 
		{
			/* Tell the user that we could not find a usable */
			/* Winsock DLL.                                  */
			throw exception("WSAStartup failed with error: %d\n", err);
		}

		/* Confirm that the WinSock DLL supports 2.2.*/
		/* Note that if the DLL supports versions greater    */
		/* than 2.2 in addition to 2.2, it will still return */
		/* 2.2 in wVersion since that is the version we      */
		/* requested.                                        */

		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
		{
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.                                  */
			WSACleanup();
			throw exception("Could not find a usable version (2.2) of Winsock.dll\n");
		}

		int port = 8064;		// TODO: read this from configuration, overridable from command-line
		int threadCount = 8;	// TODO: dynamically set this to the number of cores
		int pendingCount = 64;	// TODO: tune this

		_handler = boost::shared_ptr<ServiceHandler>(new ServiceHandler());
		_processor = boost::shared_ptr<TProcessor>(new ServiceProcessor(_handler));
		_serverTransport = boost::shared_ptr<TServerTransport>(new TServerSocket(port));
		_transportFactory = boost::shared_ptr<TTransportFactory>(new TBufferedTransportFactory());
		_protocolFactory = boost::shared_ptr<TProtocolFactory>(new TBinaryProtocolFactory());
	
		_threadManager = ThreadManager::newSimpleThreadManager(threadCount, pendingCount);
		_threadFactory = boost::shared_ptr<concurrency::PlatformThreadFactory>(new concurrency::PlatformThreadFactory());
		_threadManager->threadFactory(_threadFactory);
		_threadManager->start();

		_server = boost::shared_ptr<TThreadPoolServer>(new TThreadPoolServer(_processor, _serverTransport, _transportFactory, _protocolFactory, _threadManager));
	}

	~impl()
	{
		Stop();
		WSACleanup();
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

FastoreService::FastoreService() : _pimpl(new impl())
{
}

void FastoreService::Run()
{
	_pimpl->Run();
}

void FastoreService::Stop()
{
	_pimpl->Stop();
}
