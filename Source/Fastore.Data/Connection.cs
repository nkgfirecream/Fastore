using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Fastore.Data
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
			return new Statement(_connection, batch);
		}
	}
}
