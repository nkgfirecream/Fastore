#include <cstdio>

#include <sys/stat.h>

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

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

void RunService(ServiceEventCallback started, 
		ServiceEventCallback stopping, 
		const EndpointConfig& endpointConfig, 
		const StartupConfig& startupConfig)
{
    int err;

    try	{
	ServiceStartup startup;
	startup.__set_path(startupConfig.dataPath);
	startup.__set_port(endpointConfig.port);

	boost::shared_ptr<ServiceHandler> 
	    handler(new ServiceHandler(startup));

	boost::shared_ptr<TProcessor> 
	    processor(new ServiceProcessor(handler));

	processor->setEventHandler(handler);

	endpoint = boost::shared_ptr<Endpoint>(
				 new Endpoint(endpointConfig, processor));
    } 
    catch (const exception& e) {
	cout << "Error starting service: " << e.what();
	return;
    }

    if (started != NULL)
	started();

    // Start main execution
    try {
	endpoint->Run();
    }
    catch (const exception& e) {
	cout << "Error during service execution: " << e.what();
    }

    if (stopping != NULL)
	stopping();

    // Stop the service
    try {
	endpoint.reset();			
    }
    catch (const exception& e) {
	cout << "Error shutting down service: " << e.what();
	return;
    }
}

//Shuts the server down gracefully.
void ShutdownEndpoint()
{
    try {
	boost::shared_ptr<TSocket> 
	    socket(new TSocket("localhost", endpoint->getConfig().port));
	boost::shared_ptr<TTransport> 
	    transport(new TFramedTransport(socket));
	boost::shared_ptr<TProtocol> 
	    protocol(new TBinaryProtocol(transport));

	ServiceClient client(protocol);
	transport->open();
	client.shutdown();
	transport->close();
    }
    catch(...) {
	// For now we expect a transport exception upon shutting down, 
	// since the server will immediately
	// close and terminate all connections.
    }

    if (endpoint != NULL)
	endpoint->Stop();
}

#if defined(_WIN32)
//
// Purpose: 
//   Handles Ctrl-C
//
BOOL CtrlCHandler(DWORD fdwCtrlType) 
{ 
    switch (fdwCtrlType) { 
	// Handle the CTRL-C signal. 
    case CTRL_C_EVENT: 
	cout << "Stop Request Received.\n";
	ShutdownEndpoint();
	return( TRUE );

    default: 
	return FALSE; 
    } 
}
#endif

void ConsoleStarted()
{
	// Report running status when initialization is complete.
	cout << "Service started.\nPress Ctrl-C to stop...\n";
#if defined(_WIN32)
	// Ctrl-C handling
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlCHandler, TRUE) ) 
		cout << "ERROR: Could not set control handler.\n";
#endif
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

struct CombinedConfig
{
	EndpointConfig endpointConfig;
	StartupConfig startupConfig;
};

extern char *optarg;

int
main(int argc, char* argv[])
{
    CombinedConfig config;
    int ch;

    while ((ch = getopt(argc, argv, "d:p:")) != -1) {
	switch (ch) {
	case 'd': {
	    struct stat sb;
	    if( -1 == stat(optarg, &sb) ) {
		ostringstream oops;
		oops << optarg << " is not a valid path";
		perror(oops.str().c_str());
		exit(EXIT_FAILURE);
	    }
	    config.startupConfig.dataPath = optarg;
	    } break;
	case 'p': {
	    istringstream is(optarg);
	    is >> config.endpointConfig.port;
	    if( is.tellg() == 0 || !is.good() ) {
		cerr << "error: " << optarg << " is not a good port number\n";
		exit(EXIT_FAILURE);
	    }
	    } break;
	case '?':
	default:
	    cout << "syntax: fastored -p port -d datapath\n";
	    break;
	}
    }

    cout << "Configuration: port = " << config.endpointConfig.port 
	 << "	data path = '"       << config.startupConfig.dataPath 
	 << "'\n";

    cout << "Service starting....\n";

    RunService(&ConsoleStarted, 
	       &ConsoleStopping, 
	       config.endpointConfig, 
	       config.startupConfig);

    cout << "Service stopped.\n";

    return EXIT_SUCCESS;
}

#if defined(_WIN32)
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
#endif
