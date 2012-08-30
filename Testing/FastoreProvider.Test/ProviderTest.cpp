#include "stdafx.h"
#include "string.h"
#include "CppUnitTest.h"
#include "..\..\Source\FastoreProvider\fastore.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace FastoreProviderTest
{		
	TEST_CLASS(ProviderTest)
	{
	public:
		
		TEST_METHOD(TestConnect)
		{
			FastoreAddress addresses[1];
			strcpy(addresses[0].hostName, "localhost");
			addresses[0].port = 8064;
			ConnectResult result = fastoreConnect(1, addresses);

			Assert::IsTrue(result.success);
		}

		TEST_METHOD(TestDisconnect)
		{
			//no transactions
			FastoreAddress addresses;
			strcpy(addresses.hostName, "localhost");
			addresses.port = 8064;
			ConnectResult result = fastoreConnect(1, addresses);

			ConnectionHandle connection;
			fastoreDisconnect(connection);

			//transactions have been committed

			//transactions still exist - db connection should remain open
		}

		TEST_METHOD(TestPrepare)
		{
			// TODO: Your test code here
		}

		TEST_METHOD(TestBind)
		{
			// TODO: Your test code here
		}

		TEST_METHOD(TestNext)
		{
			// TODO: Your test code here
		}

		TEST_METHOD(TestColumnInfo)
		{
			// TODO: Your test code here
		}

		TEST_METHOD(TestColumnValue)
		{
			// TODO: Your test code here
		}

		TEST_METHOD(TestClose)
		{
			// TODO: Your test code here
		}

		TEST_METHOD(TestExecute)
		{
			// TODO: Your test code here
		}

	};
}