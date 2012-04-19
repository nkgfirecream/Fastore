#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Database.h"
#include "../FastoreCore/Host.h"
#pragma managed(pop)

#include "ManagedSession.h"
#include "ManagedHost.h"

namespace Wrapper
{
	public ref class ManagedDatabase
	{
		private:
			Database* _nativeDatabase;
			Host* _nativeHost;

		public:
			Database* GetNativePointer();

			ManagedDatabase(ManagedHost^ host) : _nativeHost(host->GetNativePointer())
			{
				_nativeDatabase = new Database(*(_nativeHost));
			}
			ManagedDatabase(Database* nativeDatabase) : _nativeDatabase(nativeDatabase) {};
			ManagedSession^ Start();
			TransactionID GetID(/* Token */);
			//Low priority
			//void Topology();
			//ILock GetLock(/*name, mode */);
	};
}