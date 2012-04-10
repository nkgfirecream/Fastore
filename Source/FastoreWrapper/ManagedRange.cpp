#include "stdafx.h"
#include "ManagedRange.h"

using namespace Wrapper;

fs::Range* Wrapper::ManagedRange::GetNativePointer()
{
	return _nativeRange;
}

fs::RangeBound* Wrapper::ManagedRangeBound::GetNativePointer()
{
	return _nativeRangeBound;
}