#pragma once
#include "stdafx.h"
#include "../FastoreCore/Range.h"

using namespace System;

namespace Wrapper
{
		public ref class ManagedRange
	{
		private:
			Range* _nativeRange;

		public:
			ManagedRange(Range* nativeRange) : _nativeRange(nativeRange) {};
			Range* GetNativePointer();
	};
}