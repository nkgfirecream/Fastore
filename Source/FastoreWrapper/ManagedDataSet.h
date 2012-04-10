#pragma once
#include "stdafx.h"
#include "../FastoreCore/Table/dataset.h"

using namespace System;

namespace Wrapper
{
	//TODO: These next few items are basically POD -- it may be more efficient to map them to memory and copy rather than wrap. 
	public ref class ManagedDataSet
	{
		private:
			DataSet* _nativeDataSet;

		public:
			ManagedDataSet(DataSet* nativeSet) : _nativeDataSet(nativeSet) {};
			DataSet* GetNativePointer();
	};
}