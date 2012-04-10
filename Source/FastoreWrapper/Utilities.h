#pragma once
#include "stdafx.h"

using namespace System;
using namespace fs;

namespace Wrapper
{
	void* ConvertObjectToVoidPointer(System::Object^ object);

	//TODO: template this function? What type do we need to go to?
	System::Object^ ConvertVoidPointerToObject(void* pointer);

	void* MarshalString (String ^ s);
}
