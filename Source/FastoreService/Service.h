#pragma once
//#include <vld.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define SVCNAME TEXT("Fastore")

SERVICE_STATUS          gSvcStatus; 
SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
HANDLE                  ghSvcStopEvent = NULL;
#else
#define BOOL int
#define VOID void
#define WINAPI
#define LPTSTR char*
#define DWORD uint32_t
#endif

BOOL CtrlCHandler(DWORD);
VOID SvcInstall(void);
VOID WINAPI SvcCtrlHandler(DWORD); 
VOID WINAPI SvcMain(DWORD, LPTSTR *); 
VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR *); 
VOID SvcReportEvent(LPTSTR, LPTSTR);
