#include "stdafx.h"
#include "ManagedTopology.h"

using namespace Wrapper;

Topology* Wrapper::ManagedTopology::GetNativePointer()
{
	return _nativeTopology;
}

void Wrapper::ManagedTopology::Add(ManagedColumnDef^ def)
{
	_nativeTopology->push_back(*(def->GetNativePointer()));
}

