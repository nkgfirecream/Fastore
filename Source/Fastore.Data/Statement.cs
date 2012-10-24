using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Data
{
	public class Statement : IDisposable
	{
		public const int MaxStringSize = 512;

		private IntPtr _statement;

		internal Statement(IntPtr connection, string batch)
		{
			var result = Provider.Prepare(connection, batch);
			Provider.CheckResult(result.Result);
			_statement = result.Statement;
		}

		public void Dispose()
		{
			if (_statement != IntPtr.Zero)
			{
				try
				{
					var result = Provider.Close(_statement);
					Provider.CheckResult(result.Result);
				}
				finally
				{
					_statement = IntPtr.Zero;
				}
			}
		}

		public bool Next()
		{
			var result = Provider.Next(_statement);
			Provider.CheckResult(result.Result);
			return result.Eof != 0;
		}

		public int GetInt32(int index)
		{
			int size = sizeof(int);
			int value;
			var getResult = Provider.ColumnValue(_statement, index, ref size, out value);
			return value;
		}

		public long GetInt64(int index)
		{
			int size = sizeof(long);
			long value;
			var getResult = Provider.ColumnValue(_statement, index, ref size, out value);
			return value;
		}

		public double GetDouble(int index)
		{
			int size = sizeof(double);
			double value;
			var getResult = Provider.ColumnValue(_statement, index, ref size, out value);
			return value;
		}

		public string GetString(int index)
		{
			int size = MaxStringSize;
			var value = new StringBuilder(MaxStringSize);
			value.Length = size;
			var getResult = Provider.ColumnValueA(_statement, index, ref size, value);
			value.Length = size;
			return value.ToString();;
		}
	}
}
