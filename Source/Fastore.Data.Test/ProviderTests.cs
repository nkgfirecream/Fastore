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
						Assert.IsTrue(statement.Next(), "No row.");

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
						Assert.IsTrue(statement.Next(), "No row.");

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
						Assert.IsTrue(statement.Next(), "No row.");

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
						Assert.IsTrue(statement.Next(), "No row.");

						Assert.IsFalse(statement.GetInt64(0).HasValue);
					}
				}
			);
		}

		[TestMethod]
		public void ResetTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					var statement = c.Prepare("select sqlite_version()");

					Assert.IsTrue(statement.Next(), "No row.");
					Assert.IsFalse(statement.Next(), "Invalid row.");

					statement.Reset();

					Assert.IsTrue(statement.Next(), "No row.");
					Assert.IsFalse(statement.Next(), "Invalid row.");
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
		public void BasicBindInt64Test()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select ?"))
					{
						statement.Bind(1, 1234);

						Assert.IsTrue(statement.Next(), "No row.");

						Assert.AreEqual(1234, statement.GetInt64(0));
					}
				}
			);
		}

		[TestMethod]
		public void BasicBindDoubleTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select ?"))
					{
						statement.Bind(1, 12.34);

						Assert.IsTrue(statement.Next(), "No row.");

						Assert.AreEqual(12.34, statement.GetDouble(0));
					}
				}
			);
		}

		[TestMethod]
		public void BasicBindAStringTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select ?"))
					{
						statement.BindAString(1, "Blah");

						Assert.IsTrue(statement.Next(), "No row.");

						Assert.AreEqual("Blah", statement.GetAString(0));
					}
				}
			);
		}

		[TestMethod]
		public void BasicBindWStringTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select ?"))
					{
						statement.BindWString(1, "BlahW");

						Assert.IsTrue(statement.Next(), "No row.");

						Assert.AreEqual("BlahW", statement.GetWString(0));
					}
				}
			);
		}

		[TestMethod]
		public void BasicBindAgainTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select ?"))
					{
						statement.Bind(1, 1234);

						Assert.IsTrue(statement.Next(), "No row.");

						Assert.AreEqual(1234, statement.GetInt64(0));

						statement.Bind(1, 2468);

						Assert.IsTrue(statement.Next(), "No row.");

						Assert.AreEqual(2468, statement.GetInt64(0));
					}
				}
			);
		}

		[TestMethod]
		public void GetColumnInfoTest()
		{
			InternalConnectDisconnect
			(
				(c) =>
				{
					using (var statement = c.Prepare("select 'Blah'"))
					{
						Assert.AreEqual(1, statement.ColumnCount, "Incorrect number of columns in result.");

						var info = statement.GetColumnInfo(0);
						Assert.AreEqual("varchar", info.Name);
						Assert.AreEqual(Provider.ArgumentType.FASTORE_ARGUMENT_STRING8, info.Type);
					}
				}
			);
		}
	}
}
