#include "stdafx.h"
#include <vector>
#include <boost\assign\list_of.hpp>
#include "TestSetup.h"
#include "CppUnitTest.h"
#include "Extensions.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace fastore::client;
using namespace boost::assign;
using namespace std;

TEST_CLASS(DataAccessTest), public TestSetup
{
public:
        TEST_METHOD(TestTableSetup)
		{
			createTableWithData();

			std::vector<Statistic> statsVect = _database->GetStatistics(_columns);
			//6 columns
			Assert::AreEqual<size_t>(statsVect.size(), 6);
			//7 rows
			Assert::AreEqual<int64_t>(statsVect[0].total, 7);
		}

        TEST_METHOD(TestGetRangeCols)
        {
            createTableWithData();

			Range range;
			range.ColumnID = 1000;
			range.Ascending = true;
            RangeSet rangeData = _database->GetRange(_columns, range, 50);
			DataSet data = rangeData.Data;

            //Assert::AreEqual<int>(data.getColumnCount, 1);
            //Assert::AreEqual<>(data.Data[0].Values[1], "Shmoe");
            //Assert::AreEqual<>(data.Data[0].Values[2], "Antarctica");
            //Assert::AreEqual<>(data.Data[1].Values[0], 2);
            //Assert::AreEqual<>(data.Data[1].Values[1], "Shmoe");
            //Assert::AreEqual<>(data.Data[1].Values[2], "Denver");
            //Assert::AreEqual<>(data.Data[2].Values[0], 3);
            //Assert::AreEqual<>(data.Data[2].Values[1], "Silverman");
            //Assert::AreEqual<>(data.Data[2].Values[2], "Chicago");   
        }

         TEST_METHOD(TestGetRangeLimit)
        {
            createTableWithData();

           /* var data = _database.GetRange(_columns, new Range{ ColumnID = 1000, Ascending = true}, 3);
            Assert.AreEqual(data.Data.Count, 3);

            var data2 = _database.GetRange(_columns, new Range { ColumnID = 1000, Ascending = true }, 9);
			Assert.AreEqual(data2.Data.Count, 7); */
        }

         TEST_METHOD(TestGetRangeOrder)
        {
            createTableWithData();

            /*var data = _database.GetRange(_columns, new Range { ColumnID = 1000, Ascending = false}, 3);
			Assert.AreEqual((int)data.Data[0].Values[0], 7);
			Assert.AreEqual((int)data.Data[1].Values[0], 6);
			Assert.AreEqual((int)data.Data[2].Values[0], 5);*/ 
        }

		  TEST_METHOD(TestGetRangeStart)
        {
            createTableWithData();

   //         var data = _database.GetRange(_columns, new Range { ColumnID = 1003, Ascending = true }, 7);

			//var data2 = _database.GetRange(_columns, new Range { ColumnID = 1003, Ascending = true, Start = new RangeBound { Bound = data.Data[1].Values[3], Inclusive = true } }, 7);

   //         //never concludes debugging process, or gives "invalid null pointer" error and closes service
			//Assert.Equals(data2.Data.Count, 5);
			//Assert.Equals(data2.Data[0].Values[0], data.Data[1].Values[0]);
        }

         TEST_METHOD(TestInclude)
        {
            createTableWithData();

            /*_database.Include(_columns, 8, new object[] { 8, "Martha", "Stewart", false, "4/10/1967", "San Jose" });
            var data = _database.GetRange(_columns, new Range { ColumnID = 1000, Ascending = true }, 8);

			Assert.AreEqual(data.Data.Count, 8);
			Assert.AreEqual(data.Data[7].Values[0], 8);
			Assert.AreEqual(data.Data[7].Values[1], "Martha");
			Assert.AreEqual(data.Data[7].Values[2], "Stewart");
			Assert.AreEqual(data.Data[7].Values[3], false);
			Assert.AreEqual(data.Data[7].Values[4], "4/10/1967");
			Assert.AreEqual(data.Data[7].Values[5], "San Jose");*/
        }

         TEST_METHOD(TestExclude)
        {
            createTableWithData();

           /* _database.Exclude(_columns, 7);
            var data = _database.GetRange(_columns, new Range { ColumnID = 1000, Ascending = true }, 8);

			Assert.Equals(data.Data.Count, 6);
			Assert.Equals(data.Data[0].Values[0], 1);
			Assert.Equals(data.Data[1].Values[0], 2);
			Assert.Equals(data.Data[2].Values[0], 3);
			Assert.Equals(data.Data[3].Values[0], 4);
			Assert.Equals(data.Data[4].Values[0], 5);
			Assert.Equals(data.Data[5].Values[0], 6);*/
        }

         TEST_METHOD(TestGetStatistics)
        {
            createTableWithData();
            
            /*var data = _database.GetStatistics(_columns);
            Assert.AreEqual(data.Length, _columns.Length);
            Assert.AreEqual(data[0].Total, 7);
            Assert.AreEqual(data[0].Unique, 7);
            Assert.AreEqual(data[2].Total, 7);
            Assert.AreEqual(data[2].Unique, 6);
            Assert.AreEqual(data[3].Total, 7);
            Assert.AreEqual(data[3].Unique, 2);*/
        }
};//end class

