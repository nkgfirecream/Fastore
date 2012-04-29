#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Range.h"
#include "../FastoreCore/Util/utilities.h"
#pragma managed(pop)

using namespace fs;

namespace Wrapper
{
	public ref class Utilities
	{
		public:
			static void* ConvertObjectToNative(System::Object^ object);

			//TODO: template this function? What type do we need to go to?
			static System::Object^ ConvertNativeToObject(void* pointer);

			static array<System::String^>^ ConvertStringArray(eastl::vector<std::wstring>);
			static eastl::vector<std::wstring> ConvertStringArray(array<System::String^>^);
			static System::String^ ConvertToManagedString(std::wstring);
			static System::String^ ConvertToManagedString(std::string);
			static std::wstring ConvertToNativeWString(System::String^);
			static std::string ConvertToNativeString(System::String^);

			static ScalarType ConvertStringToScalarType(System::String^);
			static System::String^ ConvertScalarTypeToString(ScalarType);
	};
}
