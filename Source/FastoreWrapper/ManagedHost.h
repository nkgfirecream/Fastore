#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Host.h"
#pragma managed(pop)

#include "ManagedColumnDef.h"

namespace Wrapper
{
	public ref class ManagedHost
	{
		private:
			Host* _nativeHost;

		public:
			Host* GetNativePointer();
			ManagedHost(Host* nativeHost) : _nativeHost(nativeHost) {};

			void CreateColumn(ManagedColumnDef^  def);
			void DeleteColumn(System::Int32 columnId);
			System::Boolean ExistsColumn(System::Int32 columnId);			
	};
}