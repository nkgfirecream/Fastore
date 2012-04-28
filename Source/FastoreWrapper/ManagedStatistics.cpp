#include "stdafx.h"
#include "ManagedStatistics.h"

using namespace Wrapper;

Statistics* Wrapper::ManagedStatistics::GetNativePointer()
{
	return _nativeStatistics;
}
