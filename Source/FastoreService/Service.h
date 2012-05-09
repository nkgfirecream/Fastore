#pragma once
#include <windows.h>

#define SVCNAME TEXT("Fastore")

SERVICE_STATUS          gSvcStatus; 
SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
HANDLE                  ghSvcStopEvent = NULL;

BOOL CtrlCHandler(DWORD);
VOID SvcInstall(void);
VOID WINAPI SvcCtrlHandler(DWORD); 
VOID WINAPI SvcMain(DWORD, LPTSTR *); 
VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPTSTR *); 
VOID SvcReportEvent(LPTSTR, LPTSTR);
