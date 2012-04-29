#pragma once
#include "typedefs.h"
#include "Topology.h"
#include "Schema\standardtypes.h"
#include "Column\IColumnBuffer.h"
#include <EASTL\hash_set.h>
#include <EASTL\hash_map.h>

using namespace fs;

typedef eastl::pair<IColumnBuffer*,ColumnDef> PointerDefPair;

class Host
{
	
	//TODO: Host Information...
	//Address

	//TODO: So... Let's dump the columns into the host.. This is non-bootstrapped, and therfore wrong. The Host factory will evenutally need to create a bootstrapped host so columns
	//addtions and removals can simply be includes/excludes.
	private:
		//Store pointers to the column buffers..
		eastl::vector<PointerDefPair> _columns;

		//Map Ids to locations in the vector (should I just point to a pointer instead? No, because I potentially need additional information about the column)
	//Topology should probably store column info
		eastl::hash_map<fs::wstring, int, eastl::string_hash<fs::wstring>> _columnMap;

		ScalarType GetScalarTypeFromString(fs::wstring tname);
		

	public:

		//TODO: kill these..
		void CreateColumn(ColumnDef  def);
		void DeleteColumn(const fs::wstring name);
		PointerDefPair GetColumn(fs::wstring name);

};