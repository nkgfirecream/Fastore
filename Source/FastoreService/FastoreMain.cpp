#define WIN32_LEAN_AND_MEAN
#include "FastoreMain.h"
#include "ServiceHandler.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>



void FastoreInit()
{
	//Open windows sockets
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        throw exception("WSAStartup failed with error: %d\n", err);
    }

	/* Confirm that the WinSock DLL supports 2.2.*/
	/* Note that if the DLL supports versions greater    */
	/* than 2.2 in addition to 2.2, it will still return */
	/* 2.2 in wVersion since that is the version we      */
	/* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        WSACleanup();
        throw exception("Could not find a usable version of Winsock.dll\n");
    }
    else
	{
        printf("The Winsock 2.2 dll was found okay\n");
		return;
	}
}

int FastoreMain(HANDLE ghSvcStopEvent)
{
	int port = 8064;
	boost::shared_ptr<ServiceHandler> handler(new ServiceHandler());
	boost::shared_ptr<TProcessor> processor(new ServiceProcessor(handler));
	boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
	boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
	boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

	TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
	server.serve();

	//Return non-zero if threadstart fails
	return 0;
}

void FastoreCleanup()
{
	WSACleanup();
}