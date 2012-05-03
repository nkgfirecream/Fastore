#include "stdafx.h"
#include "ManagedHost.h"

using namespace Wrapper;

Host* Wrapper::ManagedHost::GetNativePointer()
{
	return _nativeHost;
}

void Wrapper::ManagedHost::CreateColumn(ManagedColumnDef^  def)
{
	_nativeHost->CreateColumn(*def->GetNativePointer());
}

void Wrapper::ManagedHost::DeleteColumn(System::Int32 columnId)
{
	_nativeHost->DeleteColumn(columnId);
}

System::Boolean Wrapper::ManagedHost::ExistsColumn(System::Int32 columnId)
{
	return _nativeHost->ExistsColumn(columnId);
}
