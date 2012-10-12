#pragma once
#include "scalar.h"

struct NodeType : public ScalarType
{
	NodeType();
};

struct NoOpNodeType : public ScalarType
{
	NoOpNodeType();
};