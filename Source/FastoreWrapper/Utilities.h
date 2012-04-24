#pragma once
#include "stdafx.h"

#pragma managed(push, off)
#include "../FastoreCore/Range.h"
#pragma managed(pop)

using namespace fs;

namespace Wrapper
{
	public ref class Utilities
	{
		public:
			static void* ConvertObjectToVoidPointer(System::Object^ object);

			//TODO: template this function? What type do we need to go to?
			static System::Object^ ConvertVoidPointerToObject(void* pointer);

			static array<System::String^>^ ConvertStringArray(eastl::vector<std::wstring>);
			static eastl::vector<std::wstring> ConvertStringArray(array<System::String^>^);
			static System::String^ ConvertString(std::wstring);
			static std::wstring ConvertString(System::String^);
	};
}
