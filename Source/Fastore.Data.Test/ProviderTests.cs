using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Text;

namespace Alphora.Fastore.Data.Test
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

						Assert.AreEqual(5523123232, statement.GetInt64(0).Value);
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

						Assert.AreEqual(1.234, statement.GetDouble(0).Value);
					}
				}
			);
		}

		[TestMethod]
		public void GetAStringTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select 'Hello World'"))
					{
						if (!statement.Next())
							Assert.Fail("No row.");

						Assert.AreEqual("Hello World", statement.GetAString(0));
					}
				}
			);
		}

		[TestMethod]
		public void GetMultiColumnRowTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select 'Hello World', 5, 1.234 union select 'Hey There', 10, 2.345"))
					{
						if (!statement.Next())
							Assert.Fail("No row.");

						Assert.AreEqual("Hello World", statement.GetAString(0));
						Assert.AreEqual(5, statement.GetInt64(1));
						Assert.AreEqual(1.234, statement.GetDouble(2));

						if (!statement.Next())
							Assert.Fail("No 2nd row.");

						Assert.AreEqual("Hey There", statement.GetAString(0));
						Assert.AreEqual(10, statement.GetInt64(1));
						Assert.AreEqual(2.345, statement.GetDouble(2));

						if (statement.Next())
							Assert.Fail("Invalid 3rd row.");
					}
				}
			);
		}

		[TestMethod]
		public void GetNullTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select null"))
					{
						if (!statement.Next())
							Assert.Fail("No row.");

						Assert.IsFalse(statement.GetInt64(0).HasValue);
					}
				}
			);
		}

		[TestMethod]
		public void ExecuteFunctionTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					var statement = c.Execute("select sqlite_version()");

					Assert.IsNull(statement);
				}
			);
		}

		[TestMethod]
		public void BasicBindTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select ?"))
					{
						statement.Bind(1, 1234);

						if (!statement.Next())
							Assert.Fail("No row.");

						Assert.AreEqual(1234, statement.GetInt64(0));
					}
				}
			);
		}
	}
}
