using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Text;

namespace Fastore.Data.Test
{
	[TestClass]
	public class ProviderTests
	{
		private static void InternalConnectDisconnect(Action<Connection> callback)
		{
			using (var connection = new Connection(new Provider.FastoreAddress[] { new Provider.FastoreAddress { HostName = "localhost", Port = 8765 } }))
			{
				callback(connection);
			}
		}

		[TestMethod]
		public void ConnectDisconnectTest()
		{
			InternalConnectDisconnect
			(
				(c) => { }
			);
		}

		[TestMethod]
		public void PrepareCloseTest()
		{
			InternalConnectDisconnect
			(
				(c) => 
				{ 
					using (var statement = c.Prepare("select 5"))
					{
					}
				}
			);
		}

		[TestMethod]
		public void GetInt32Test()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select 5"))
					{
						if (!statement.Next())
							Assert.Fail("No row.");

						Assert.AreEqual(5, statement.GetInt32(0));
					}
				}
			);
		}

		[TestMethod]
		public void GetInt64Test()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select 5523123232"))
					{
						if (!statement.Next())
							Assert.Fail("No row.");

						Assert.AreEqual(5523123232, statement.GetInt64(0));
					}
				}
			);
		}

		[TestMethod]
		public void GetDoubleTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select 1.234"))
					{
						if (!statement.Next())
							Assert.Fail("No row.");

						Assert.AreEqual(1.234, statement.GetDouble(0));
					}
				}
			);
		}
	}
}
