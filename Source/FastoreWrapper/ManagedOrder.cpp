#include "stdafx.h"
#include "ManagedOrder.h"

using namespace Wrapper;

fs::Order* Wrapper::ManagedOrder::GetNativePointer()
{
	return _nativeOrder;
}
