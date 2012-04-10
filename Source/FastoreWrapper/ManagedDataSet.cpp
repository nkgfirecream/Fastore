#include "stdafx.h"
#include "ManagedDataSet.h"

DataSet* Wrapper::ManagedDataSet::GetNativePointer()
{
	return _nativeDataSet;
}