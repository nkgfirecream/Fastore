#include "stdafx.h"
#include "ManagedHostFactory.h"

using namespace Wrapper;

Wrapper::ManagedHost^ Wrapper::ManagedHostFactory::Create(ManagedTopology^ topo)
{
	Host* host = new Host();
	//host -> set up topology
	//wrap host
	ManagedHost^ wrapper = gcnew ManagedHost(host);
	return wrapper;
}

HostFactory* Wrapper::ManagedHostFactory::GetNativePointer()
{
	return _nativeHostFactory;
}
