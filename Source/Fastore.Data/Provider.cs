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

		[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
		public struct FastoreAddress
		{
			[MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_HOST_NAME)]
			string message;
			int code;
		};

		[StructLayout(LayoutKind.Sequential)]
		public struct ConnectResult
		{
			int result;
			int connection;
		};

		// Retrieves the message and code of the last error
		[DllImport("FastoreProvider.dll", CharSet = CharSet.Ansi)]
		[return: MarshalAs(UnmanagedType.Bool)]
		public extern static bool fastoreGetLastError
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
		[DllImport("FastoreProvider.dll")]
		public extern static ConnectResult fastoreConnect(int addressCount, [MarshalAs(UnmanagedType.LPArray)]FastoreAddress[] addresses);

		//// Dereferences the given database connection; the connection may remain open if any transactions are still open on it
		//FASTOREAPI FastoreResult APIENTRY fastoreDisconnect(ConnectionHandle connection);

		//// Prepares a given query or statement statement and returns a cursor
		//FASTOREAPI PrepareResult APIENTRY fastorePrepare(ConnectionHandle database, const char *sql);
		//// Provides values for any parameters included in the prepared statement and resets the cursor
		//FASTOREAPI FastoreResult APIENTRY fastoreBind(StatementHandle statement, int argumentCount, void *arguments, const fastore::provider::ArgumentType ArgumentType[]);
		//// Executes the statement, or navigates to the first or next row
		//FASTOREAPI NextResult APIENTRY fastoreNext(StatementHandle statement);
		//// Gets the column name for the given column index
		//FASTOREAPI ColumnInfoResult APIENTRY fastoreColumnInfo(StatementHandle statement, int columnIndex);
		//// Gets the column value of the current row given an index
		//FASTOREAPI FastoreResult APIENTRY fastoreColumnValue(StatementHandle statement, int columnIndex, int targetMaxBytes, void *valueTarget);
		//// Closes the given cursor
		//FASTOREAPI FastoreResult APIENTRY fastoreClose(StatementHandle statement);

		//// Short-hand for Prepare followed by Next... then close if eof.
		//FASTOREAPI ExecuteResult APIENTRY fastoreExecute(ConnectionHandle connection, const char *sql);
    }
}
