#pragma once
#include "stdafx.h"
#include "../FastoreCore/Host.h"

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