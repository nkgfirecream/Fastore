#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Host.h"
#pragma managed(pop)

using namespace System;

namespace Wrapper
{
	public ref class ManagedHost
	{
		private:
			Host* _nativeHost;

		public:
			Host* GetNativePointer();

			ManagedHost(Host* nativeHost) : _nativeHost(nativeHost) {};
	};
}