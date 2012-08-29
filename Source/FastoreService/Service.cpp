#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <tchar.h>
#include <string>
#include <strsafe.h>


#include "Service.h"
#include "errors.h"
#include "../FastoreCore/Endpoint.h"
#include "../FastoreCore/ServiceHandler.h"
#include "../FastoreCore/EndpointConfig.h"
#include "../FastoreCore/StartupConfig.h"


using namespace std;
using namespace ::fastore::communication;
using namespace ::fastore::server;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

boost::shared_ptr<Endpoint> endpoint;

typedef void (*ServiceEventCallback)();

void RunService(ServiceEventCallback started, ServiceEventCallback stopping, const EndpointConfig& endpointConfig, const StartupConfig& startupConfig)
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
		cout << "WSAStartup failed with error: " << err << "\n";
		return;
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
		cout << "Could not find a usable version (2.2) of Winsock.dll\n";
		return;
	}

	try
	{
		ServiceStartup startup;
		startup.__set_path(startupConfig.dataPath);
		startup.__set_port(endpointConfig.port);

		auto handler = boost::shared_ptr<ServiceHandler>(new ServiceHandler(startup));
		auto processor = boost::shared_ptr<TProcessor>(new ServiceProcessor(handler));

		processor->setEventHandler(handler);

		endpoint = boost::shared_ptr<Endpoint>(new Endpoint(endpointConfig, processor));
	}
	catch (const exception& e)
	{
		cout << "Error starting service: " << e.what();
		return;
	}

	if (started != NULL)
		started();

	// Start main execution
	try
	{
		endpoint->Run();
	}
	catch (const exception& e)
	{
		cout << "Error during service execution: " << e.what();
	}

	if (stopping != NULL)
		stopping();

	// Stop the service
	try
	{
		endpoint.reset();			
	}
	catch (const exception& e)
	{
		cout << "Error shutting down service: " << e.what();
		return;
	}

	// Cleanup winsock
	WSACleanup();
}

//Shuts the server down gracefully.
void ShutdownEndpoint()
{
	try
	{
		boost::shared_ptr<TSocket> socket(new TSocket("localhost", endpoint->getConfig().port));
		boost::shared_ptr<TTransport> transport(new TFramedTransport(socket));
		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

		ServiceClient client(protocol);
		transport->open();
		client.shutdown();
		transport->close();
	}
	catch(...)
	{
		// for now we expect a transport exception upon shutting down, since the server will immediately
		//close and terminate all connections.
	}

	if (endpoint != NULL)
			endpoint->Stop();
}

//
// Purpose: 
//   Handles Ctrl-C
//
BOOL CtrlCHandler(DWORD fdwCtrlType) 
{ 
	switch (fdwCtrlType) 
	{ 
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT: 
		cout << "Stop Request Received.\n";
		ShutdownEndpoint();
		return( TRUE );

	default: 
		return FALSE; 
	} 
}

void ConsoleStarted()
{
	// Report running status when initialization is complete.
	cout << "Service started.\nPress Ctrl-C to stop...\n";

	// Ctrl-C handling
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlCHandler, TRUE) ) 
		cout << "ERROR: Could not set control handler.\n";
}

void ConsoleStopping()
{
	// Report running status when initialization is complete.
	cout << "Stopping Service...\n";
}

void ConsoleError(string message)
{
	cout << message;
}

vector<string> argsToVector(int argc, wchar_t* argv[])
{
	vector<string> args;
	for (int i = 1; i < argc; i++) 
	{
		wstring current(argv[i]);
		args.push_back(string(current.begin(), current.end()));
	}
	return args;
}

struct CombinedConfig
{
	EndpointConfig endpointConfig;
	StartupConfig startupConfig;
};

CombinedConfig getConfig(vector<string>& args)
{
	CombinedConfig config;

	// TODO: set configuration from arguments and/or from file...
	for (int i = 0; i < args.size(); i++)
	{
		if (args[i] == "-p" && args.size() - 1 > i)
			istringstream(args[i + 1]) >> config.endpointConfig.port;
		else if (args[i] == "-dp" && args.size() - 1 > i)
			config.startupConfig.dataPath = args[i + 1];
	}

	return config;
}

void __cdecl _tmain(int argc, wchar_t* argv[])
{
	// If command-line parameter is "install", install the service. 
	// Otherwise, the service is probably being started by the SCM.

	if (lstrcmpi( argv[1], TEXT("-run")) == 0 || lstrcmpi( argv[1], TEXT("-r")) == 0)
	{
		auto config = getConfig(argsToVector(argc, argv));

		cout << "Configuration: port = " << config.endpointConfig.port << "	data path = '" << config.startupConfig.dataPath << "'\n";

		cout << "Service starting....\n";
		RunService(&ConsoleStarted, &ConsoleStopping, config.endpointConfig, config.startupConfig);
		cout << "Service stopped.\n";
		return;
	}
	else if (lstrcmpi( argv[1], TEXT("-install")) == 0 || lstrcmpi( argv[1], TEXT("-i")) == 0)
	{
		SvcInstall();
		return;
	}

	// The dispatcher returns when the service has stopped. 
	// The process should simply terminate when the call returns.
	SERVICE_TABLE_ENTRY DispatchTable[] = 
	{ 
		{ SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain }, 
		{ NULL, NULL } 
	}; 
	if (!StartServiceCtrlDispatcher( DispatchTable )) 
	{ 
		SvcReportEvent(TEXT("StartServiceCtrlDispatcher"), NULL); 
	} 
}

//
// Purpose: 
//   Installs a service in the SCM database
//
// Parameters:
//   None
// 
// Return value:
//   None
//
VOID SvcInstall()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];

	if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
	{
		cout << "Cannot install service (" << GetLastError() << ")\n";
		return;
	}

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager( 
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager) 
	{
		cout << "OpenSCManager failed (" << GetLastError() << "%d)\n";
		return;
	}

	// Create the service

	schService = CreateService( 
		schSCManager,              // SCM database 
		SVCNAME,                   // name of service 
		SVCNAME,                   // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_DEMAND_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		szPath,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 

	if (schService == NULL) 
	{
		cout << "CreateService failed (" << GetLastError() << ")\n";
		CloseServiceHandle(schSCManager);
		return;
	}
	else
		cout << "Service installed successfully\n"; 

	CloseServiceHandle(schService); 
	CloseServiceHandle(schSCManager);
}

void ServiceStarted()
{
	ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
}

void ServiceStopping()
{
	ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
}

//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain( DWORD dwArgc, LPTSTR* lpszArgv )
{
	// Register the handler function for the service
	gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);

	if (!gSvcStatusHandle)
	{ 
		SvcReportEvent(TEXT("SvcMain"), TEXT("Unable to obtain status handle.")); 
		return; 
	} 

	//TODO: redirect stdout to log

	// These SERVICE_STATUS members remain as set here
	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
	gSvcStatus.dwServiceSpecificExitCode = 0;    

	// Report starting status to the SCM
	ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	// Get configuration
	auto config = getConfig(argsToVector(dwArgc, lpszArgv));

	// Run the service
	RunService(&ServiceStarted, &ServiceStopping, config.endpointConfig, config.startupConfig);

	// Report stopped status to the SCM
	ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus( DWORD dwCurrentState,
	DWORD dwWin32ExitCode,
	DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure.

	gSvcStatus.dwCurrentState = dwCurrentState;
	gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
	gSvcStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		gSvcStatus.dwControlsAccepted = 0;
	else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if ( (dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED) )
		gSvcStatus.dwCheckPoint = 0;
	else gSvcStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the SCM.
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
	// Handle the requested control code. 

	switch(dwCtrl) 
	{  
	case SERVICE_CONTROL_STOP: 
		ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

		// Signal the service to stop.

		if (endpoint != NULL)
			endpoint->Stop();
		ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);

		return;

	case SERVICE_CONTROL_INTERROGATE: 
		break; 

	default: 
		break;
	} 

}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction, LPTSTR szMessage) 
{ 
	HANDLE hEventSource;
	LPCTSTR lpszStrings[2];
	TCHAR Buffer[80];

	hEventSource = RegisterEventSource(NULL, SVCNAME);

	if( NULL != hEventSource )
	{
		if (szMessage != NULL)
			StringCchCopy(Buffer, 80, szMessage);
		else
			StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

		lpszStrings[0] = SVCNAME;
		lpszStrings[1] = Buffer;

		ReportEvent(hEventSource,        // event log handle
			EVENTLOG_ERROR_TYPE, // event type
			0,                   // event category
			SVC_ERROR,           // event identifier
			NULL,                // no security identifier
			2,                   // size of lpszStrings array
			0,                   // no binary data
			lpszStrings,         // array of strings
			NULL);               // no binary data

		DeregisterEventSource(hEventSource);
	}
}