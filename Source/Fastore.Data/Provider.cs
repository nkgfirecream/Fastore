using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Fastore.Data
{
    public static class Provider
    {
		public const int MAX_HOST_NAME = 255;
		public const int FASTORE_OK = 0;

		[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
		public struct FastoreAddress
		{
			[MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_HOST_NAME)]
			public string HostName;
			public ulong Port;
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct ConnectResult
		{
			public int Result;
			public IntPtr Connection;
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct GeneralResult
		{
			public int Result;
		}

		// Retrieves the message and code of the last error
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreGetLastError", CharSet = CharSet.Ansi)]
		[return: MarshalAs(UnmanagedType.Bool)]
		public extern static bool GetLastError
		(
			int result, 
			[param: MarshalAs(UnmanagedType.U4)]
			uint messageMaxLength, 
			[param: MarshalAs(UnmanagedType.LPStr), Out()]
			StringBuilder message, 
			out int code
		);

		// Creates a new database connection
		//FASTOREAPI ConnectResult APIENTRY fastoreConnect(size_t addressCount, const struct FastoreAddress addresses[]);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreConnect")]
		public extern static ConnectResult Connect
		(
			int addressCount, 
			[MarshalAs(UnmanagedType.LPArray)]
			FastoreAddress[] addresses
		);

		//// Dereferences the given database connection; the connection may remain open if any transactions are still open on it
		//FASTOREAPI GeneralResult fastoreDisconnect(ConnectionHandle connection);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreDisconnect")]
		public extern static GeneralResult Disconnect(IntPtr connection);

		//// Prepares a given query or statement statement and returns a cursor
		//FASTOREAPI PrepareResult fastorePrepare(ConnectionHandle database, const char *sql);
		//// Provides values for any parameters included in the prepared statement and resets the cursor
		//FASTOREAPI GeneralResult fastoreBind(StatementHandle statement, size_t argumentCount, void *arguments, const ArgumentType ArgumentType[]);
		//// Executes the statement, or navigates to the first or next row
		//FASTOREAPI NextResult fastoreNext(StatementHandle statement);
		//// Gets the column name for the given column index
		//FASTOREAPI ColumnInfoResult fastoreColumnInfo(StatementHandle statement, int columnIndex);
		//// Gets the column value of the current row given an index
		//FASTOREAPI GeneralResult fastoreColumnValue(StatementHandle statement, int columnIndex, int targetMaxBytes, void *valueTarget);
		//// Closes the given cursor
		//FASTOREAPI GeneralResult fastoreClose(StatementHandle statement);

		//// Short-hand for Prepare followed by Next... then close if eof.
		//FASTOREAPI ExecuteResult fastoreExecute(ConnectionHandle connection, const char *sql);
    }
}
