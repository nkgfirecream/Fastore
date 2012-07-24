using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Client
{
	/// <summary> The column IDs for the core and the client. </summary>
	/// <remarks> ColumnIDs 0-9,999 are reserved for the core engine.  Column IDs 10,000 through 
	/// 19,999 are reserved for the client. </remarks>
	public static class Dictionary
	{
		public const int MaxSystemColumnID = 9999;
		public const int MaxClientColumnID = 19999;

		public const int ColumnID = 0;
		public const int ColumnName = 1;
		public const int ColumnValueType = 2;
		public const int ColumnRowIDType = 3;
		public const int ColumnBufferType = 4;

		public static readonly int[] ColumnColumns = 
		{ 
			ColumnID,
			ColumnName,
			ColumnValueType,
			ColumnRowIDType,
			ColumnBufferType
		};

		public const int TopologyID = 100;

		public static readonly int[] TopologyColumns = 
		{ 
			TopologyID
		};

		public const int HostID = 200;

		public static readonly int[] HostColumns =
		{
			HostID
		};

		public const int PodID = 300;
		public const int PodHostID = 301;

		public static readonly int[] TablePodColumns = 
		{ 
			PodID,
			PodHostID
		};

		public const int PodColumnPodID = 400;
		public const int PodColumnColumnID = 401;

		public static readonly int[] PodColumnColumns = 
		{ 
			PodColumnPodID,
			PodColumnColumnID
		};

		public const int GeneratorNextValue = 10000;

		public static readonly int[] GeneratorColumns = 
		{ 
			GeneratorNextValue 
		};
	}
}
