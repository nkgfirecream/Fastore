using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Alphora.Fastore.Data
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

		[StructLayout(LayoutKind.Sequential)]
		public struct PrepareResult
		{
			public int Result;
			public IntPtr Statement;
			public int ColumnCount;
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct ExecuteResult
		{
			public int Result;
			public IntPtr Statement;
			public int ColumnCount;
			// TODO: get this to marshal as a bool (tried all the obvious things)
			public byte Eof;
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct NextResult
		{
			public int Result;
			// TODO: get this to marshal as a bool (tried all the obvious things)
			public byte Eof;
		};

		[StructLayout(LayoutKind.Sequential)]
		public struct ColumnValueInt64Result
		{
			public int Result;
			// TODO: get this to marshal as a bool (tried all the obvious things)
			public byte IsNull;
			public long Value;
		};

		[StructLayout(LayoutKind.Sequential)]
		public struct ColumnValueDoubleResult
		{
			public int Result;
			// TODO: get this to marshal as a bool (tried all the obvious things)
			public byte IsNull;
			public double Value;
		};

		[StructLayout(LayoutKind.Sequential)]
		public struct ColumnValueStringResult
		{
			public int Result;
			// TODO: get this to marshal as a bool (tried all the obvious things)
			public byte IsNull;
		};

		// Retrieves the message and code of the last error
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreGetLastError", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
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
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreConnect", CallingConvention = CallingConvention.Cdecl)]
		public extern static ConnectResult Connect
		(
			int addressCount, 
			[MarshalAs(UnmanagedType.LPArray)]
			FastoreAddress[] addresses
		);

		// Dereferences the given database connection; the connection may remain open if any transactions are still open on it
		//FASTOREAPI GeneralResult fastoreDisconnect(ConnectionHandle connection);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreDisconnect", CallingConvention = CallingConvention.Cdecl)]
		public extern static GeneralResult Disconnect(IntPtr connection);

		// Prepares a given query or statement statement and returns a cursor
		//FASTOREAPI PrepareResult fastorePrepare(ConnectionHandle connection, const char *batch);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastorePrepare", CallingConvention = CallingConvention.Cdecl)]
		public extern static PrepareResult Prepare(IntPtr connection, [param: MarshalAs(UnmanagedType.LPStr)]string batch);

		//// Provides values for any parameters included in the prepared statement and resets the cursor
		//FASTOREAPI GeneralResult fastoreBindInt64(StatementHandle statement, int32_t argumentIndex, int64_t value);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreBindInt64", CallingConvention = CallingConvention.Cdecl)]
		public extern static PrepareResult Bind(IntPtr statement, int argumentIndex, long value);

		//FASTOREAPI GeneralResult fastoreBindDouble(StatementHandle statement, int32_t argumentIndex, double value);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreBindDouble", CallingConvention = CallingConvention.Cdecl)]
		public extern static PrepareResult Bind(IntPtr statement, int argumentIndex, double value);

		//FASTOREAPI GeneralResult fastoreBindAString(StatementHandle statement, int32_t argumentIndex, const char *value);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreBindAString", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
		public extern static PrepareResult BindAString(IntPtr statement, int argumentIndex, string value);

		//FASTOREAPI GeneralResult fastoreBindWString(StatementHandle statement, int32_t argumentIndex, const wchar_t *value);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreBindWString", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
		public extern static PrepareResult BindWString(IntPtr statement, int argumentIndex, string value);

		// Executes the statement, or navigates to the first or next row
		//FASTOREAPI NextResult fastoreNext(StatementHandle statement);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreNext", CallingConvention = CallingConvention.Cdecl)]
		public extern static NextResult Next(IntPtr statement);

		
		//// Gets the column name for the given column index
		//FASTOREAPI ColumnInfoResult fastoreColumnInfo(StatementHandle statement, int32_t columnIndex);

		// Gets the column value of the current row given an index
		//FASTOREAPI ColumnValueInt64Result fastoreColumnValueInt64(StatementHandle statement, int32_t columnIndex);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreColumnValueInt64", CallingConvention = CallingConvention.Cdecl)]
		public extern static ColumnValueInt64Result ColumnValueInt64(IntPtr statement, int columnIndex);

		//FASTOREAPI ColumnValueDoubleResult fastoreColumnValueDouble(StatementHandle statement, int32_t columnIndex);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreColumnValueDouble", CallingConvention = CallingConvention.Cdecl)]
		public extern static ColumnValueDoubleResult ColumnValueDouble(IntPtr statement, int columnIndex);

		//FASTOREAPI ColumnValueAStringResult fastoreColumnValueAString(StatementHandle statement, int32_t columnIndex, int32_t *targetMaxBytes, char *valueTarget);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreColumnValueAString", CallingConvention = CallingConvention.Cdecl)]
		public extern static ColumnValueStringResult ColumnValueAString(IntPtr statement, int columnIndex, ref int targetMaxBytes, [param: MarshalAs(UnmanagedType.LPStr), Out()] StringBuilder valueTarget);

		//FASTOREAPI ColumnValueAStringResult fastoreColumnValueWString(StatementHandle statement, int32_t columnIndex, int32_t *targetMaxBytes, wchar_t *valueTarget);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreColumnValueWString", CallingConvention = CallingConvention.Cdecl)]
		public extern static ColumnValueStringResult ColumnValueWString(IntPtr statement, int columnIndex, ref int targetMaxBytes, [param: MarshalAs(UnmanagedType.LPWStr), Out()] StringBuilder valueTarget);

		// Closes the given cursor
		//FASTOREAPI GeneralResult fastoreClose(StatementHandle statement);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreClose", CallingConvention = CallingConvention.Cdecl)]
		public extern static GeneralResult Close(IntPtr statement);

		//// Short-hand for Prepare followed by Next... then close if eof.
		//FASTOREAPI ExecuteResult fastoreExecute(ConnectionHandle connection, const char *batch);
		[DllImport("FastoreProvider.dll", EntryPoint = "fastoreExecute", CallingConvention = CallingConvention.Cdecl)]
		public extern static ExecuteResult Execute(IntPtr connection, [param: MarshalAs(UnmanagedType.LPStr)]string batch);

		public static void CheckResult(int result)
		{
			if (result != Provider.FASTORE_OK)
			{
				var message = new StringBuilder(255);
				int code;
				if (!Provider.GetLastError(result, (uint)message.Capacity, message, out code))
					throw new Exception("Unable to retrieve error details.");
				throw new Exception(String.Format("Error {0}: {1}", code, message));
			}
		}
    }
}
