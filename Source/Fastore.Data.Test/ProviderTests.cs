using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Text;

namespace Fastore.Data.Test
{
	[TestClass]
	public class ProviderTests
	{
		private static void CheckResult(int result)
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

		private static void InternalConnectDisconnect(Action<IntPtr> callback)
		{
			var conResult =
				Provider.Connect
				(
					1,
					new Provider.FastoreAddress[] 
					{ 
						new Provider.FastoreAddress { HostName = "localhost", Port = 8765 }
					}
				);
			CheckResult(conResult.Result);
			try
			{
				callback(conResult.Connection);
			}
			finally
			{
				var disResult = Provider.Disconnect(conResult.Connection);
				CheckResult(disResult.Result);
			}
		}

		[TestMethod]
		public void ConnectDisconnectTest()
		{
			InternalConnectDisconnect((c) => {});
		}

		[TestMethod]
		public void PrepareCloseTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					var prepareResult = Provider.Prepare(c, "select 5");
					CheckResult(prepareResult.Result);
					try
					{
						var nextResult = Provider.Next(prepareResult.Statement);
						CheckResult(nextResult.Result);
					}
					finally
					{
						var closeResult = Provider.Close(prepareResult.Statement);
						CheckResult(closeResult.Result);
					}
				}
			);
		}
	}
}
