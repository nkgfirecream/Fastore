#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Table/dataset.h"
#pragma managed(pop)

namespace Wrapper
{
	//TODO: These next few items are basically POD -- it may be more efficient to map them to memory and copy rather than wrap. 
	public ref class ManagedDataSet
	{
		private:
			DataSet* _nativeDataSet;

		public:
			ManagedDataSet() {}
			ManagedDataSet(DataSet* nativeSet) : _nativeDataSet(nativeSet) {}
			DataSet* GetNativePointer();

			int Size();
			array<System::String^>^ Row(int row);
			void Dump();
	};
}