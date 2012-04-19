#include "stdafx.h"
#include "ManagedColumnDef.h"

using namespace Wrapper;

ColumnDef* Wrapper::ManagedColumnDef::GetNativePointer()
{
	return _nativeColumnDef;
}
