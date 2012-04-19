#pragma once
#include "typedefs.h"

using namespace fs;

struct ColumnDef
{
	fs::wstring Name;
	fs::wstring KeyType;
	bool IsUnique;
};

//TODO: This is wrong
typedef eastl::vector<ColumnDef> Topology;

//class Topology
//{
//	//Topology Information  -- Stores, Buffers, Revisors, etc.
//};