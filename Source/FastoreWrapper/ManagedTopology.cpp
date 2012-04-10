#include "stdafx.h"
#include "ManagedTopology.h"

using namespace Wrapper;

Topology* Wrapper::ManagedTopology::GetNativePointer()
{
	return _nativeTopology;
}