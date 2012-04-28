#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Column/IColumnBuffer.h"
#pragma managed(pop)

namespace Wrapper
{
	public ref class ManagedStatistics
	{
		private: 
			Statistics* _nativeStatistics;

		public:
			Statistics* GetNativePointer();

			ManagedStatistics(Statistics* nativeStatistics) : _nativeStatistics(nativeStatistics) {};

			System::Int64 Total()
			{
				return _nativeStatistics->Total;
			}

			System::Int64 Unique()
			{
				return _nativeStatistics->Unique;
			}
	};
}