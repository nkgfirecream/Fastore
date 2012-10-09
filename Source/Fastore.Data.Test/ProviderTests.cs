using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Text;

namespace Fastore.Data.Test
{
	[TestClass]
	public class ProviderTests
	{
		[TestMethod]
		public void ConnectDisconnectTest()
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

			var disResult = Provider.Disconnect(conResult.Connection);
			CheckResult(disResult.Result);
		}

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
	}
}
