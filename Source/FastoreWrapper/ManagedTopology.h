#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Topology.h"
#pragma managed(pop)

using namespace System;

namespace Wrapper
{
	public ref class ManagedTopology
	{
		private:
			Topology* _nativeTopology;

		public:
			ManagedTopology(Topology* nativeTopology) : _nativeTopology(nativeTopology) {};
			Topology* GetNativePointer();

	};
}