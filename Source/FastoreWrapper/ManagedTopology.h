#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Topology.h"
#pragma managed(pop)

#include "ManagedColumnDef.h"

namespace Wrapper
{
	public ref class ManagedTopology
	{
		private:
			Topology* _nativeTopology;

		public:
			ManagedTopology()
			{
				_nativeTopology = new Topology();
			}

			ManagedTopology(Topology* nativeTopology) : _nativeTopology(nativeTopology) {};
			Topology* GetNativePointer();

			void Add(ManagedColumnDef^ columndef);

	};
}