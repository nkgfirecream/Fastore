#pragma once
#include "scalar.h"

struct HashSetType : public ScalarType
{
	HashSetType();
};

struct KeyVectorType : public ScalarType
{
	KeyVectorType();
};

struct KeyTreeType : public ScalarType
{
	KeyTreeType();
};

struct BTreeType : public ScalarType
{
	BTreeType();
};