#pragma once
#include <vector>
#include "..\FastoreCommunication\Comm_types.h"

using namespace fastore::communication;

namespace fastore { namespace client
{
	/// <summary> The column IDs for the core and the client. </summary>
	/// <remarks> ColumnIDs 0-9,999 are reserved for the core engine.  Column IDs 10,000 through 
	/// 19,999 are reserved for the client. </remarks>
	class Dictionary
	{	
	private:
		static const fastore::communication::ColumnID _ColumnColumns[];
		static const fastore::communication::ColumnID _TopologyColumns[];
		static const fastore::communication::ColumnID _HostColumns[];		
		static const fastore::communication::ColumnID _TablePodColumns[];		
		static const fastore::communication::ColumnID _PodColumnColumns[];
		static const fastore::communication::ColumnID _GeneratorColumns[];

	public:
		static const fastore::communication::ColumnID MaxSystemColumnID = 9999;
		static const fastore::communication::ColumnID MaxClientColumnID = 19999;

		static const fastore::communication::ColumnID ColumnID = 0;
		static const fastore::communication::ColumnID ColumnName = 1;
		static const fastore::communication::ColumnID ColumnValueType = 2;
		static const fastore::communication::ColumnID ColumnRowIDType = 3;
		static const fastore::communication::ColumnID ColumnBufferType = 4;	
		static const ColumnIDs ColumnColumns;

		static const fastore::communication::ColumnID TopologyID = 100;
		static const ColumnIDs TopologyColumns;

		static const fastore::communication::ColumnID HostID = 200;
		static const ColumnIDs HostColumns;

		static const fastore::communication::ColumnID PodID = 300;
		static const fastore::communication::ColumnID PodHostID = 301;
		static const ColumnIDs TablePodColumns;

		static const fastore::communication::ColumnID PodColumnPodID = 400;
		static const fastore::communication::ColumnID PodColumnColumnID = 401;
		static const ColumnIDs PodColumnColumns;

		static const fastore::communication::ColumnID GeneratorNextValue = 10000;	
		static const ColumnIDs GeneratorColumns;

	};
}}
