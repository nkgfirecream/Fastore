#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/HostFactory.h"
#pragma managed(pop)

#include "ManagedTopology.h"
#include "ManagedHost.h"


namespace Wrapper
{
	public ref class ManagedHostFactory
	{
		private:
			HostFactory* _nativeHostFactory;

		public:
			HostFactory* GetNativePointer();

			ManagedHostFactory()
			{
				_nativeHostFactory = new HostFactory();
			}

			ManagedHostFactory(HostFactory* nativeHostFactory) : _nativeHostFactory(nativeHostFactory) {}
			ManagedHost^ Create(ManagedTopology^ topo);
			//Lower Priority
			//IHost Connect(address);		
	};
}