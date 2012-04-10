#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Range.h"
#pragma managed(pop)

using namespace System;

namespace Wrapper
{
		public ref class ManagedRange
	{
		private:
			fs::Range* _nativeRange;

		public:
			ManagedRange(fs::Range* nativeRange) : _nativeRange(nativeRange) {};
			fs::Range* GetNativePointer();
	};
}