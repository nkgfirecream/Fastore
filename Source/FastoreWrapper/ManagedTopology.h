#pragma once
#include "stdafx.h"
#include "../FastoreCore/Topology.h"

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