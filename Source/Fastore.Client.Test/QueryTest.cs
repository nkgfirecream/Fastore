using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Alphora.Fastore.Client;

namespace Fastore.Client.Test
{
    [TestClass]
    public class QueryTest
    {
        private Database _database;
        private int[] _columns;

        public void createTable()
        {
            string hostname = "localhost";
            const int port = 8064;
            _database = Alphora.Fastore.Client.Client.Connect(hostname, port);

            int[] _schemaColumns = new int[] { 0, 1, 2, 3, 4 };
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };

            _database.Include(_schemaColumns, _columns[0], new object[] { _columns[0], "ID", "Int", "Int", true });
            _database.Include(_schemaColumns, _columns[1], new object[] { _columns[1], "Given", "String", "Int", false });
            _database.Include(_schemaColumns, _columns[2], new object[] { _columns[2], "Surname", "String", "Int", false });
            _database.Include(_schemaColumns, _columns[3], new object[] { _columns[3], "Gender", "Bool", "Int", false });
            _database.Include(_schemaColumns, _columns[4], new object[] { _columns[4], "BirthDate", "String", "Int", false });
            _database.Include(_schemaColumns, _columns[5], new object[] { _columns[5], "BirthPlace", "String", "Int", false });
        }

        public void addData(Database db)
        {
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };

            db.Include(_columns, 1, new object[] { 1, "Joe", "Shmoe", true, "5/26/1980", "Antarctica" });
            db.Include(_columns, 2, new object[] { 2, "Ann", "Shmoe", false, "4/20/1981", "Denver" });
            db.Include(_columns, 3, new object[] { 3, "Sarah", "Silverman", false, "11/10/1976", "Chicago" });
            db.Include(_columns, 4, new object[] { 4, "Bob", "Newhart", true, "12/2/1970", "Paris" });
            db.Include(_columns, 5, new object[] { 5, "Samantha", "Smith", false, "1/13/1984", "Tokyo" });
            db.Include(_columns, 6, new object[] { 6, "Andy", "Warhol", true, "9/14/1987", "New York" });
            db.Include(_columns, 7, new object[] { 7, "Carl", "Sagan", true, "4/1/1957", "Las Vegas" });
        }

        [TestMethod]
        public void TestCreateTable(){
        string hostname1 = "localhost";
            const int port1 = 8064; 
            Database _db = Alphora.Fastore.Client.Client.Connect(hostname1, port1);

            int[] _schemaColumns = new int[] { 0, 1, 2, 3, 4 };
            int[] _cols = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };

            _db.Include(_schemaColumns, _cols[0], new object[] { _cols[0], "ID", "Int", "Int", true });
            _db.Include(_schemaColumns, _cols[1], new object[] { _cols[1], "Given", "String", "Int", false });
            _db.Include(_schemaColumns, _cols[2], new object[] { _cols[2], "Surname", "String", "Int", false });
            _db.Include(_schemaColumns, _cols[3], new object[] { _cols[3], "Gender", "Bool", "Int", false });
            _db.Include(_schemaColumns, _cols[4], new object[] { _cols[4], "BirthDate", "String", "Int", false });
            _db.Include(_schemaColumns, _cols[5], new object[] { _cols[5], "BirthPlace", "String", "Int", false });

            var stats = _db.GetStatistics(_cols);
            Assert.AreEqual(stats[0].Total, 0);

            _db.Include(_columns, 0, new object[] { 0, "Joe", "Shmoe", true, "5/26/1980", "Antarctica" });
            var stats2 = _db.GetStatistics(_cols);
            Assert.AreEqual(stats2[0].Total, 1);
        }

        [TestMethod]
        public void TestGetRangeCols()
        {
            createTable();
            addData(_database);
            int[] cols = new int[] { 1000, 1002, 1005 };
            var data = _database.GetRange(
                cols,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 3 } }
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
            createTable();
            addData(_database);

            var data = _database.GetRange(
                _columns, 
                new []{ new Order{ ColumnID = 1000, Ascending = true} }, 
                new []{ new Range{ ColumnID = 1000, Limit = 3} }
                );
            Assert.AreEqual(data.Count, 3);

            var data2 = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 9 } }
                );
            Assert.AreEqual(data2.Count, 7); //bug - expects 8
        }

        [TestMethod]
        public void TestGetRangeOrder()
        {
            createTable();
            addData(_database);
            var data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = false } },
                new[] { new Range { ColumnID = 1000, Limit = 3 } }
                );
            Assert.AreEqual((int)data[0].Values[0], 7); 
            Assert.AreEqual((int)data[1].Values[0], 6); 
            Assert.AreEqual((int)data[2].Values[0], 5); 
        }

        [TestMethod]
        public void TestGetRangeStartId()
        {
            createTable();
            addData(_database);
            var data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1003, Ascending = true } },
                new[] { new Range { ColumnID = 1003, Limit = 7 } }
                );
            var data2 = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1003, Ascending = true } },
                new[] { new Range { ColumnID = 1003, Limit = 7, Start = new RangeBound { Bound = data[1].Values[3], Inclusive = true } } },
                data[1].Values[0]
                );
            //never concludes debugging process, or gives "invalid null pointer" error and closes service
            Assert.Equals(data2.Count, 5);
            Assert.Equals(data2[0].Values[0], data[1].Values[0]);
        }

        [TestMethod]
        public void TestInclude()
        {
            createTable();
            addData(_database);
            _database.Include(_columns, 8, new object[] { 8, "Martha", "Stewart", false, "4/10/1967", "San Jose" });
            var data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
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
            createTable();
            addData(_database);
            _database.Exclude(_columns, 7);
            var data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
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
            createTable();
            addData(_database);
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
