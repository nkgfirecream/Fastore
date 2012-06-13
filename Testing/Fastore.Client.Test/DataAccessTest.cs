using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Alphora.Fastore.Client;

namespace Fastore.Client.Test
{
    [TestClass]
    public class DataAccessTest : TestSetup
    {
        [TestMethod]
        public void TestCreateTable(){
        string hostname1 = "localhost";
            const int port1 = 8064; 
            Database _db = Alphora.Fastore.Client.Client.Connect(hostname1, port1);

            int[] _schemaColumns = new int[] { 0, 1, 2, 3, 4 };
            int[] _cols = new int[] { 2000, 2001, 2002, 2003, 2004, 2005 };

            _db.Include(_schemaColumns, _cols[0], new object[] { _cols[0], "ID", "Int", "Int", true });
            _db.Include(_schemaColumns, _cols[1], new object[] { _cols[1], "Given", "String", "Int", false });
            _db.Include(_schemaColumns, _cols[2], new object[] { _cols[2], "Surname", "String", "Int", false });
            _db.Include(_schemaColumns, _cols[3], new object[] { _cols[3], "Gender", "Bool", "Int", false });
            _db.Include(_schemaColumns, _cols[4], new object[] { _cols[4], "BirthDate", "String", "Int", false });
            _db.Include(_schemaColumns, _cols[5], new object[] { _cols[5], "BirthPlace", "String", "Int", false });

            var stats = _db.GetStatistics(_cols);
            Assert.AreEqual(stats[0].Total, 0);

            _db.Include(_cols, 0, new object[] { 0, "Joe", "Shmoe", true, "5/26/1980", "Antarctica" });
            var stats2 = _db.GetStatistics(_cols);
            Assert.AreEqual(stats2[0].Total, 1);
        }

        [TestMethod]
        public void TestGetRangeCols()
        {
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };
            createTable(_columns);
            addData(_database, _columns);

            int[] cols = new int[] { 1000, 1002, 1005 };
            var data = _database.GetRange(
                cols,
                new Range { ColumnID = 1000, Ascending = true },
                3
                );
            Assert.AreEqual(data[0].Values[0], 1);
            Assert.AreEqual(data[0].Values[1], "Shmoe");
            Assert.AreEqual(data[0].Values[2], "Antarctica");
            Assert.AreEqual(data[1].Values[0], 2);
            Assert.AreEqual(data[1].Values[1], "Shmoe");
            Assert.AreEqual(data[1].Values[2], "Denver");
            Assert.AreEqual(data[2].Values[0], 3);
            Assert.AreEqual(data[2].Values[1], "Silverman");
            Assert.AreEqual(data[2].Values[2], "Chicago");   
        }

        [TestMethod]
        public void TestGetRangeLimit()
        {
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };
            createTable(_columns);
            addData(_database, _columns);

            var data = _database.GetRange(
                _columns, 
                new Range{ ColumnID = 1000, Ascending = true},
                3
                );
            Assert.AreEqual(data.Count, 3);

            var data2 = _database.GetRange(
                _columns,
                new Range { ColumnID = 1000, Ascending = true },
                9
                );
            Assert.AreEqual(data2.Count, 7); 
        }

        [TestMethod]
        public void TestGetRangeOrder()
        {
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };
            createTable(_columns);
            addData(_database, _columns);
            var data = _database.GetRange(
                _columns,
                new Range { ColumnID = 1000, Ascending = false},
                3
                );
            Assert.AreEqual((int)data[0].Values[0], 7); 
            Assert.AreEqual((int)data[1].Values[0], 6); 
            Assert.AreEqual((int)data[2].Values[0], 5); 
        }

        [TestMethod]
        public void TestGetRangeStart()
        {
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };
            createTable(_columns);
            addData(_database, _columns);
            var data = _database.GetRange(
                _columns,
                new Range { ColumnID = 1003, Ascending = true },
                7
                );
            //var data2 = _database.GetRange(
            //    _columns,
            //    new[] { new Order { ColumnID = 1003, Ascending = true } },
            //    new[] { new Range { ColumnID = 1003, Limit = 7, Start = new RangeBound { Bound = data[1].Values[3], Inclusive = true } } },
            //    data[1].Values[0]
            //    );
            var data2 = _database.GetRange(
                _columns,
                new Range { ColumnID = 1003, Ascending = true, Start = new RangeBound { Bound = data[1].Values[3], Inclusive = true } },
                7
                );

            //never concludes debugging process, or gives "invalid null pointer" error and closes service
            Assert.Equals(data2.Count, 5);
            Assert.Equals(data2[0].Values[0], data[1].Values[0]);
        }

        [TestMethod]
        public void TestInclude()
        {
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };
            createTable(_columns);
            addData(_database, _columns);
            _database.Include(_columns, 8, new object[] { 8, "Martha", "Stewart", false, "4/10/1967", "San Jose" });
            var data = _database.GetRange(
                _columns,
                new Range { ColumnID = 1000, Ascending = true },
                8
                );
            Assert.AreEqual(data.Count, 8);
            Assert.AreEqual(data[7].Values[0], 8);
            Assert.AreEqual(data[7].Values[1], "Martha");
            Assert.AreEqual(data[7].Values[2], "Stewart");
            Assert.AreEqual(data[7].Values[3], false);
            Assert.AreEqual(data[7].Values[4], "4/10/1967");
            Assert.AreEqual(data[7].Values[5], "San Jose");
        }

        [TestMethod]
        public void TestExclude()
        {
            //service stops running before it can get to this test
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };
            createTable(_columns);
            addData(_database, _columns);
            _database.Exclude(_columns, 7);
            var data = _database.GetRange(
                _columns,
                new Range { ColumnID = 1000, Ascending = true }, 
                8
                );
            Assert.Equals(data.Count, 6);
            Assert.Equals(data[0].Values[0], 1);
            Assert.Equals(data[1].Values[0], 2);
            Assert.Equals(data[2].Values[0], 3);
            Assert.Equals(data[3].Values[0], 4);
            Assert.Equals(data[4].Values[0], 5);
            Assert.Equals(data[5].Values[0], 6);
        }

        [TestMethod]
        public void TestGetStatistics()
        {
            _columns = new int[] { 3000, 3001, 3002, 3003, 3004, 3005 };
            createTable(_columns);
            addData(_database, _columns);
            
            var data = _database.GetStatistics(_columns);
            Assert.AreEqual(data.Length, _columns.Length);
            Assert.AreEqual(data[0].Total, 7);
            Assert.AreEqual(data[0].Unique, 7);
            Assert.AreEqual(data[2].Total, 7);
            Assert.AreEqual(data[2].Unique, 6);
            Assert.AreEqual(data[3].Total, 7);
            Assert.AreEqual(data[3].Unique, 2);
        }
    }
}
