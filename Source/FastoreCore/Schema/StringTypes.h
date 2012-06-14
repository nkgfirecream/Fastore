#pragma once

#include "scalar.h"
#include <string>

struct StringType : public ScalarType
{
	StringType();
};

struct WStringType : public ScalarType
{
	WStringType();
};