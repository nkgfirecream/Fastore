#pragma once

#include <boost/shared_ptr.hpp>


namespace fastore { namespace client
{
	/// <summary> The column IDs for the core and the client. </summary>
	/// <remarks> ColumnIDs 0-9,999 are reserved for the core engine.  Column IDs 10,000 through 
	/// 19,999 are reserved for the client. </remarks>
	class Dictionary
	{
	public:
		static const int MaxSystemColumnID = 9999;
		static const int MaxClientColumnID = 19999;

		static const int ColumnID = 0;
		static const int ColumnName = 1;
		static const int ColumnValueType = 2;
		static const int ColumnRowIDType = 3;
		static const int ColumnBufferType = 4;

		static const int ColumnColumns[];

		static const int TopologyID = 100;

		static const int TopologyColumns[];

		static const int HostID = 200;

		static const int HostColumns[];

		static const int PodID = 300;
		static const int PodHostID = 301;

		static const int TablePodColumns[];

		static const int PodColumnPodID = 400;
		static const int PodColumnColumnID = 401;

		static const int PodColumnColumns[];

		static const int GeneratorNextValue = 10000;

		static const int GeneratorColumns[]; = {GeneratorNextValue};
	};
}}
