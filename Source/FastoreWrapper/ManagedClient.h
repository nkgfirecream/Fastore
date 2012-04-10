#pragma once
#include "stdafx.h"
#include "../FastoreCore/Client.h"
#include "ManagedDatabase.h"
#include "ManagedHost.h"

using namespace System;

namespace Wrapper
{
	public ref class ManagedClient
	{
		private: 
			Client* _nativeClient;

		public:
			Client* GetNativePointer();

			ManagedClient(Client* nativeClient) : _nativeClient(nativeClient) {};
			ManagedDatabase^ Connect(ManagedHost^ host);
	};
}