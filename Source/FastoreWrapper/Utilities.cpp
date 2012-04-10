#include "stdafx.h"
#include "Utilities.h"

void* Wrapper::ConvertObjectToVoidPointer(System::Object^ object)
{
	if (object == nullptr)
		return NULL;

	auto type = object->GetType();
	if (type->IsValueType)
	{
		//pin and return
	}
	else if (type == System::String::typeid)
	{
		return MarshalString((System::String ^)object);
	}

	return NULL;
}

System::Object^ Wrapper::ConvertVoidPointerToObject(void* pointer)
{
	//TODO: What if we are get the actual bytes for an int? Then this won't work...
	if (pointer == NULL)
		return nullptr;

	return nullptr;
}

void* Wrapper::MarshalString (System::String ^ s)
{
	//TODO: Where is this unmanaged memory allocated? Can I free it with delete? (initial investigation says no...)
   return (System::Runtime::InteropServices::Marshal::StringToHGlobalUni(s)).ToPointer();
}