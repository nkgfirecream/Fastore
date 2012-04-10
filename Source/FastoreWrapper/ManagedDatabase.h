#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Database.h"
#pragma managed(pop)

#include "ManagedSession.h"

using namespace System;

namespace Wrapper
{
	public ref class ManagedDatabase
	{
		private:
			Database* _nativeDatabase;

		public:
			Database* GetNativePointer();

			ManagedDatabase(Database* nativeDatabase) : _nativeDatabase(nativeDatabase) {};
			ManagedSession^ Start();
			TransactionID GetID(/* Token */);
			//Low priority
			//void Topology();
			//ILock GetLock(/*name, mode */);
	};
}