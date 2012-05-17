#pragma once
#include "typedefs.h"
#include "Schema\standardtypes.h"
#include "Schema\column.h"
#include "Column\IColumnBuffer.h"
#include <hash_map>

using namespace fs;

typedef std::pair<IColumnBuffer*,ColumnDef> PointerDefPair;

class FastoreHost
{	
	//TODO: Host Information...
	//Address

	//TODO: So... Let's dump the columns into the host.. This is non-bootstrapped, and therefore wrong. The Host factory will eventually need to create a bootstrapped host so columns
	//additions and removals can simply be includes/excludes.
	private:
		//Store pointers to the column buffers..
		std::vector<PointerDefPair> _columns;

		//Map Ids to locations in the vector (should I just point to a pointer instead? No, because I potentially need additional information about the column)
	//Topology should probably store column info
		std::hash_map<int,  PointerDefPair> _columnMap;

		ScalarType GetScalarTypeFromString(fs::wstring tname);

		//Most of these will probably disappear once API work has been done.
		void BootStrap();
		IColumnBuffer* InstantiateColumn(ColumnDef def);
		void AddColumnToSchema(ColumnDef def);
		void RemoveColumnFromSchema(int columnId);
		void CreateColumn(int columnId);
		void CreateColumn(ColumnDef  def);
		void DeleteColumn(const int& columnId);		
		bool ExistsColumn(const int& columnId);
		
		

	public:
		FastoreHost();

		PointerDefPair GetColumn(const int& columnId);
		void SyncToSchema();

};