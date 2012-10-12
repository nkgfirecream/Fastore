#pragma once

#include "BaseTypes.h"
#include "StringTypes.h"
#include "InternalTypes.h"
#include "NodeTypes.h"

namespace standardtypes
{
	extern StringType String;
	extern WStringType WString;
	extern IntType Int;
	extern LongType Long;
	extern BoolType Bool;
	extern DoubleType Double;
	
	// Internal Types
	extern HashSetType StandardHashSet;
	extern BTreeType StandardBTreeType;

	extern NodeType StandardNodeType;
	extern NoOpNodeType StandardNoOpNodeType;
}