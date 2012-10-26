using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Alphora.Fastore.Data
{
	public class Connection : IDisposable
	{
		private IntPtr _connection;

		public Connection(Provider.FastoreAddress[] addresses)
		{
			var result = Provider.Connect(addresses.Length, addresses);
			Provider.CheckResult(result.Result);
			_connection = result.Connection;
		}

		public void Dispose()
		{
			if (_connection != IntPtr.Zero)
			{
				try
				{
					var result = Provider.Disconnect(_connection);
					Provider.CheckResult(result.Result);
				}
				finally
				{
					_connection = IntPtr.Zero;
				}
			}
		}

		public Statement Prepare(string batch)
		{
			var result = Provider.Prepare(_connection, batch);
			Provider.CheckResult(result.Result);

			return new Statement(result.Statement, result.ColumnCount);
		}

		public Statement Execute(string batch)
		{
			var result = Provider.Execute(_connection, batch);
			Provider.CheckResult(result.Result);

			if (result.Statement != IntPtr.Zero)
				return new Statement(result.Statement, result.ColumnCount);
			else
				return null;
		}
	}
}
