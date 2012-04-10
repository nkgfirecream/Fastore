#include "stdafx.h"
#include "ManagedHost.h"

using namespace Wrapper;

Host* Wrapper::ManagedHost::GetNativePointer()
{
	return _nativeHost;
}
