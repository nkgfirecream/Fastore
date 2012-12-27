#pragma once
#include <vector>
#include "../Communication/Comm_types.h"

using namespace fastore::communication;

namespace fastore { namespace common
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
		static const fastore::communication::ColumnID _StashColumns[];
		static const fastore::communication::ColumnID _StashColumnColumns[];

	public:
		static const fastore::communication::ColumnID MaxSystemColumnID;
		static const fastore::communication::ColumnID MaxClientColumnID;

		static const fastore::communication::ColumnID ColumnID;
		static const fastore::communication::ColumnID ColumnName;
		static const fastore::communication::ColumnID ColumnValueType;
		static const fastore::communication::ColumnID ColumnRowIDType;
		static const fastore::communication::ColumnID ColumnBufferType;	
		static const fastore::communication::ColumnID ColumnRequired;
		static const ColumnIDs ColumnColumns;

		static const fastore::communication::ColumnID TopologyID;
		static const ColumnIDs TopologyColumns;

		static const fastore::communication::ColumnID HostID;
		static const ColumnIDs HostColumns;

		static const fastore::communication::ColumnID PodID;
		static const fastore::communication::ColumnID PodHostID;
		static const ColumnIDs TablePodColumns;

		static const fastore::communication::ColumnID PodColumnPodID;
		static const fastore::communication::ColumnID PodColumnColumnID;
		static const ColumnIDs PodColumnColumns;

		static const fastore::communication::ColumnID StashID;
		static const fastore::communication::ColumnID StashHostID;
		static const ColumnIDs StashColumns;

		static const fastore::communication::ColumnID StashColumnStashID;
		static const fastore::communication::ColumnID StashColumnColumnID;
		static const ColumnIDs StashColumnColumns;

	};
}}