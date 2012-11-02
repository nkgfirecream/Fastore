#pragma once
#include <vector>
#include "../FastoreCommon/Comm_types.h"

using namespace fastore::communication;

namespace fastore { namespace module
{
	/// <summary> The column IDs for the module. </summary>
	/// <remarks> ColumnIDs 0-9,999 are reserved for the core engine.  Column IDs 10,000 through 
	/// 19,999 are reserved for the client. 20,000 through 29,999 are resevered for the modules</remarks>
	class Dictionary
	{	
	
	private:
		static const communication::ColumnID _TableColumns[];
		static const communication::ColumnID _TableColumnColumns[];

	public:
		static const communication::ColumnID MaxModuleColumnID;

		static const communication::ColumnID TableID;
		static const communication::ColumnID TableName;
		static const communication::ColumnID TableDDL;
		static const ColumnIDs TableColumns;

		static const communication::ColumnID TableColumnTableID;
		static const communication::ColumnID TableColumnColumnID;
		static const ColumnIDs TableColumnColumns;

	};
}}
