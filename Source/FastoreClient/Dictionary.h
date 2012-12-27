#pragma once
#include <vector>
#include "../FastoreCommon/Communication/Comm_types.h"

using namespace fastore::communication;

namespace fastore { namespace client
{
	/// <summary> The column IDs for the core and the client. </summary>
	/// <remarks> ColumnIDs 0-9,999 are reserved for the core engine.  Column IDs 10,000 through 
	/// 19,999 are reserved for the client. </remarks>
	class Dictionary
	{	
	private:
		static const fastore::communication::ColumnID _GeneratorColumns[];

	public:
		static const fastore::communication::ColumnID GeneratorNextValue;	
		static const ColumnIDs GeneratorColumns;

	};
}}
