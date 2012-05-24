#include "Service.h"
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include "errors.h"
#include "FastoreService.h"
#include <exception>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <sstream>

using namespace std;

boost::shared_ptr<FastoreService> service;

typedef void (*ServiceEventCallback)();

void RunService(ServiceEventCallback started, ServiceEventCallback stopping)
{
	try
	{
		service = boost::shared_ptr<FastoreService>(new FastoreService());
	}
	catch (exception& e)
	{
		cout << "Error starting service: " << e.what();
		return;
	}

	if (started != NULL)
		started();

	// Start main execution
	try
	{
		service->Run();
	}
	catch (exception& e)
	{
		cout << "Error during service execution: " << e.what();
	}

	if (stopping != NULL)
		stopping();

	// Stop the service
	try
	{
		service.reset();			
	}
	catch (exception& e)
	{
		cout << "Error shutting down service: " << e.what();
		return;
	}
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
		if (service != NULL)
			service->Stop();
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

void __cdecl _tmain(int argc, _TCHAR* argv[])
{
	// If command-line parameter is "install", install the service. 
	// Otherwise, the service is probably being started by the SCM.

	if (lstrcmpi( argv[1], TEXT("-run")) == 0 || lstrcmpi( argv[1], TEXT("-r")) == 0)
	{
		cout << "Service starting....\n";
		RunService(&ConsoleStarted, &ConsoleStopping);
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
VOID WINAPI SvcMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
	// Register the handler function for the service
	gSvcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);

	if (!gSvcStatusHandle)
	{ 
		SvcReportEvent(TEXT("SvcMain"), TEXT("Unable to obtain status handle.")); 
		return; 
	} 

	// These SERVICE_STATUS members remain as set here
	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
	gSvcStatus.dwServiceSpecificExitCode = 0;    

	// Report starting status to the SCM
	ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	// Run the service
	RunService(&ServiceStarted, &ServiceStopping);

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

		if (service != NULL)
			service->Stop();
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