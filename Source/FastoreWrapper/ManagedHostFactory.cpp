#include "stdafx.h"
#include "ManagedHostFactory.h"

using namespace Wrapper;

Wrapper::ManagedHost^ Wrapper::ManagedHostFactory::Create(ManagedTopology^ topo)
{
	return gcnew ManagedHost(new Host(_nativeHostFactory->Create(*(topo->GetNativePointer()))));
}

HostFactory* Wrapper::ManagedHostFactory::GetNativePointer()
{
	return _nativeHostFactory;
}
