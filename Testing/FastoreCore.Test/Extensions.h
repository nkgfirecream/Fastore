#pragma once
#include <string>
#include <CppUnitTestAssert.h>

namespace Microsoft{ namespace VisualStudio {namespace CppUnitTestFramework
{

	template<> static std::wstring ToString<int64_t>                  (const int64_t& t)				    { RETURN_WIDE_STRING(t);}

}}}