#pragma once

#include "BaseTypes.h"
#include "StringTypes.h"
#include "InternalTypes.h"

namespace standardtypes
{
	extern StringType String;
	extern WStringType WString;
	extern IntType Int;
	extern LongType Long;
	extern BoolType Bool;
	
	// Internal Types
	extern HashSetType StandardHashSet;
	extern KeyVectorType StandardKeyVector;
	extern KeyTreeType StandardKeyTree;
	extern BTreeType StandardBTreeType;
}