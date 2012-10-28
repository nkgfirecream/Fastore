using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Alphora.Fastore.Data
{
	public class Statement : IDisposable
	{
		public const int MaxStringSize = 512;

		private IntPtr _statement;
		private int _columnCount;

		internal Statement(IntPtr statement, int columnCount)
		{
			_statement = statement;
			_columnCount = columnCount;
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

		public int ColumnCount { get { return _columnCount; } }

		public bool Next()
		{
			var result = Provider.Next(_statement);
			Provider.CheckResult(result.Result);
			return result.Eof != 0;
		}

		public void Bind(int index, long value)
		{
			var result = Provider.Bind(_statement, index, value);
			Provider.CheckResult(result.Result);
		}

		public void Bind(int index, double value)
		{
			var result = Provider.Bind(_statement, index, value);
			Provider.CheckResult(result.Result);
		}

		public void BindAString(int index, string value)
		{
			var result = Provider.BindAString(_statement, index, value);
			Provider.CheckResult(result.Result);
		}

		public void BindWString(int index, string value)
		{
			var result = Provider.BindWString(_statement, index, value);
			Provider.CheckResult(result.Result);
		}

		public long? GetInt64(int index)
		{
			var getResult = Provider.ColumnValueInt64(_statement, index);
			return getResult.IsNull == 0 ? (long?)getResult.Value : null;
		}

		public double? GetDouble(int index)
		{
			var getResult = Provider.ColumnValueDouble(_statement, index);
			return getResult.IsNull == 0 ? (double?)getResult.Value : null;
		}

		public string GetAString(int index)
		{
			int size = MaxStringSize;
			var value = new StringBuilder(MaxStringSize);
			value.Length = MaxStringSize;
			var getResult = Provider.ColumnValueAString(_statement, index, ref size, value);
			if (getResult.IsNull != 0)
				return null;
			value.Length = size;
			return value.ToString();
		}

		public string GetWString(int index)
		{
			int size = MaxStringSize * 2;
			var value = new StringBuilder(MaxStringSize);
			value.Length = MaxStringSize;
			var getResult = Provider.ColumnValueWString(_statement, index, ref size, value);
			if (getResult.IsNull != 0)
				return null;
			value.Length = size / 2;
			return value.ToString();
		}
	}
}
