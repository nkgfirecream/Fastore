#include "stdafx.h"
#include <msclr\marshal_cppstd.h>

#include "Utilities.h"

using namespace msclr::interop;

void* Wrapper::Utilities::ConvertObjectToVoidPointer(System::Object^ object)
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

	}

	return NULL;
}

System::Object^ Wrapper::Utilities::ConvertVoidPointerToObject(void* pointer)
{
	//TODO: What if we are get the actual bytes for an int? Then this won't work...
	if (pointer == NULL)
		return nullptr;

	return nullptr;
}

eastl::vector<std::wstring> Wrapper::Utilities::ConvertStringArray(array<System::String^>^ managed)
{
	eastl::vector<std::wstring> strings(managed->Length);

	for (int i = 0; i < managed->Length; i++)
	{
		strings[i] = Utilities::ConvertString(managed[i]);
	}

	return strings;
}

array<System::String^>^ Wrapper::Utilities::ConvertStringArray(eastl::vector<std::wstring> native)
{
	array<System::String^>^ strings = gcnew array<System::String^>(native.size());

	for (int i = 0; i < native.size(); i++)
	{
		strings[i] = Utilities::ConvertString(native[i]);
	}

	return strings;
}

std::wstring Wrapper::Utilities::ConvertString(System::String^ managed)
{
	return marshal_as<std::wstring>(managed);
}

System::String^ Wrapper::Utilities::ConvertString(std::wstring native)
{
	return marshal_as<System::String^>(native);
}