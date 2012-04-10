#pragma once
#include "stdafx.h"
#include "../FastoreCore/HostFactory.h"
#include "ManagedTopology.h"
#include "ManagedHost.h"


using namespace System;

namespace Wrapper
{
	public ref class ManagedHostFactory
	{
		private:
			HostFactory* _nativeHostFactory;

		public:
			HostFactory* GetNativePointer();

			ManagedHostFactory(HostFactory* nativeHostFactory) : _nativeHostFactory(nativeHostFactory) {};
			ManagedHost^ Create(ManagedTopology^ topo);
			//Lower Priority
			//IHost Connect(address);		
	};
}