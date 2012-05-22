#pragma once
#include <Windows.h>

void FastoreInit();
int FastoreMain(HANDLE ghSvcStopEvent);
void FastoreCleanup();