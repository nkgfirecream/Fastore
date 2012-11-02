#include "standardtypes.h"
#include <map>
using namespace std;

StringType standardtypes::String;
WStringType standardtypes::WString;
IntType standardtypes::Int;
LongType standardtypes::Long;
BoolType standardtypes::Bool;
DoubleType standardtypes::Double;
	
// Internal Types
HashSetType standardtypes::StandardHashSet;
BTreeType standardtypes::StandardBTreeType;

NodeType standardtypes::StandardNodeType;
NoOpNodeType standardtypes::StandardNoOpNodeType;

ScalarType standardtypes::GetTypeFromName(std::string typeName)
{
	static std::map<std::string, ScalarType> output;

	if (output.size() == 0)
	{
		output[standardtypes::String.Name] = standardtypes::String;
		output[standardtypes::WString.Name] = standardtypes::WString;
		output[standardtypes::Int.Name] = standardtypes::Int;
		output[standardtypes::Long.Name] = standardtypes::Long;
		output[standardtypes::Bool.Name] = standardtypes::Bool;
		output[standardtypes::Double.Name] = standardtypes::Double;
	}

	return output[typeName];
}