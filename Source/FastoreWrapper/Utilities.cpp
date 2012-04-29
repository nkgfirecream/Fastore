#include "stdafx.h"
#include <msclr\marshal_cppstd.h>

#include "Utilities.h"

using namespace msclr::interop;

void* Wrapper::Utilities::ConvertObjectToNative(System::Object^ object)
{
	if (object == nullptr)
		return NULL;

	//TODO: Consider storing the conversions as functions in a hash to avoid branching.
	auto type = object->GetType();
	if (type == System::Int32::typeid)
	{
		return new int((int)object);
	}
	else if (type == System::Int64::typeid)
	{
		return new long long((long long)object);
	}
	else if (type == System::Boolean::typeid)
	{
		return new bool((bool)object);
	}
	else if (type == System::String::typeid)
	{
		//TODO: Make string/wstring conversion dependent on encoding of string
		return new std::wstring(ConvertToNativeWString((System::String^)object));
	}
	else
	{
		//Unsupported Type.
		throw;
	}

	return NULL;
}

System::Object^ Wrapper::Utilities::ConvertNativeToObject(void* pointer)
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
		strings[i] = Utilities::ConvertToNativeWString(managed[i]);
	}

	return strings;
}

array<System::String^>^ Wrapper::Utilities::ConvertStringArray(eastl::vector<std::wstring> native)
{
	array<System::String^>^ strings = gcnew array<System::String^>(native.size());

	for (unsigned int i = 0; i < native.size(); i++)
	{
		strings[i] = Utilities::ConvertToManagedString(native[i]);
	}

	return strings;
}

std::wstring Wrapper::Utilities::ConvertToNativeWString(System::String^ managed)
{
	return marshal_as<std::wstring>(managed);
}

System::String^ Wrapper::Utilities::ConvertToManagedString(std::wstring native)
{
	return marshal_as<System::String^>(native);
}

std::string Wrapper::Utilities::ConvertToNativeString(System::String^ managed)
{
	return marshal_as<std::string>(managed);
}

System::String^ Wrapper::Utilities::ConvertToManagedString(std::string native)
{
	return marshal_as<System::String^>(native);
}

ScalarType Wrapper::Utilities::ConvertStringToScalarType(System::String^ typestring)
{
	//TODO: Consider putting this into a hash to avoid branches.
	if (typestring == L"WString")
	{
		return standardtypes::GetWStringType();
	}
	else if (typestring == L"String")
	{
		return standardtypes::GetStringType();
	}
	else if (typestring == L"Int")
	{
		return standardtypes::GetIntType();
	}
	else if (typestring == L"Long")
	{
		return standardtypes::GetLongType();
	}
	else if (typestring == L"Bool")
	{
		return standardtypes::GetBoolType();
	}
	else
	{
		throw;
	}
}

System::String^ Wrapper::Utilities::ConvertScalarTypeToString(ScalarType type)
{
	return ConvertToManagedString(type.Name);
}