using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Alphora.Fastore.Client;

namespace Fastore.Client.Test
{
    [TestClass]
    public class TransactionTest : TestSetup
    {
        [TestMethod]
        public void TestTransaction()
        {
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };
            createTable(_columns);
            addData(_database, _columns);

            Transaction _transaction = _database.Begin(false, false);

            _database.Include(_columns, 8, new object[] { 8, "Scott", "Pilgrim", true, "10/13/1979", "Moscow" });
            var data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            _transaction = _database.Begin(false, false);
            var tData = _transaction.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            Assert.AreEqual(data.Count, 8);
            Assert.AreEqual(tData.Count, 8);

            _database.Exclude(_columns, 8);
            data = _database.GetRange(
              _columns,
              new[] { new Order { ColumnID = 1000, Ascending = true } },
              new[] { new Range { ColumnID = 1000, Limit = 8 } }
              );
            _transaction = _database.Begin(false, false);
            tData = _transaction.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            Assert.AreEqual(data.Count, 7);
            Assert.AreEqual(tData.Count, 7);
        }

        [TestMethod]
        public void TestTransactionCommit()
        {
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };
            createTable(_columns);
            addData(_database, _columns);

            Transaction _transaction = _database.Begin(false, false);

            var data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );    
            var tData = _transaction.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            Assert.AreEqual(data.Count, 7);
            Assert.AreEqual(tData.Count, 7);

            _transaction.Commit();
            bool flag = false;
            try
            {
                tData = _transaction.GetRange(
                   _columns,
                   new[] { new Order { ColumnID = 1000, Ascending = true } },
                   new[] { new Range { ColumnID = 1000, Limit = 8 } }
                   );
            }
            catch
            {
                flag = true;
            }
            Assert.AreEqual(flag, false);

            _transaction = _database.Begin(false, false);
            _transaction.Include(_columns, 8, new object[] { 8, "Martha", "Stewart", false, "4/10/1967", "San Jose" });
            data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            tData = _transaction.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            Assert.AreEqual(data.Count, 7);
            Assert.AreEqual(tData.Count, 8);

            _transaction.Commit();
            data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            _transaction = _database.Begin(false, false);
            tData = _transaction.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            Assert.AreEqual(data.Count, 8);
            Assert.AreEqual(tData.Count, 8);

            _transaction.Exclude(_columns, 8);
            data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            tData = _transaction.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            Assert.AreEqual(data.Count, 8);
            Assert.AreEqual(tData.Count, 7);

            _transaction.Commit();
            data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            _transaction = _database.Begin(false, false);
            tData = _transaction.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            Assert.AreEqual(data.Count, 7);
            Assert.AreEqual(tData.Count, 7);
        }

        [TestMethod]
        public void TestTransactionConflict()
        {
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };
            createTable(_columns);
            addData(_database, _columns);

            Transaction _transaction = _database.Begin(false, false);

            _transaction.Include(_columns, 8, new object[] { 8, "Scott", "Pilgrim", true, "10/13/1979", "Moscow" });
            _database.Include(_columns, 8, new object[] { 8, "Martha", "Stewart", false, "4/10/1967", "San Jose" });
            bool flag = false;
            try
            {
                _transaction.Commit();
            }
            catch
            {
                flag = true;
            }
            Assert.AreEqual(flag, true);

            _transaction = _database.Begin(false, false);
            _database.Exclude(_columns, 8);
            _database.Exclude(_columns, 7);
            _transaction.Commit();
            var data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 8 } }
                );
            Assert.AreEqual(data.Count, 6);

            _transaction = _database.Begin(false, false);
            _database.Include(_columns, 7, new object[] { 7, "Carl", "Sagan", true, "4/1/1957", "Las Vegas" });
            _database.Include(_columns, 8, new object[] { 8, "Scott", "Pilgrim", true, "10/13/1979", "Moscow" });
            _transaction.Include(_columns, 9, new object[] { 9, "Martha", "Stewart", false, "4/10/1967", "San Jose" });
            _transaction.Commit();
            data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 9 } }
                );
            Assert.AreEqual(data.Count, 9);
        }

        [TestMethod]
        public void TestMultipleTransactions()
        {
            _columns = new int[] { 1000, 1001, 1002, 1003, 1004, 1005 };
            createTable(_columns);
            addData(_database, _columns);

            Transaction _transaction1 = _database.Begin(false, false);
            Transaction _transaction2 = _database.Begin(false, false);

            _transaction1.Include(_columns, 8, new object[] { 8, "Scott", "Pilgrim", true, "10/13/1979", "Moscow" });
            _transaction2.Include(_columns, 9, new object[] { 9, "Martha", "Stewart", false, "4/10/1967", "San Jose" });

            _transaction1.Commit();
            var data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 9 } }
                );
            Assert.AreEqual(data.Count, 8);

            _transaction2.Commit();
            data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 9 } }
                );
            Assert.AreEqual(data.Count, 9);

            _database.Exclude(_columns, 8);
            _database.Exclude(_columns, 9);

            _transaction1 = _database.Begin(false, false);
            _transaction2 = _database.Begin(false, false);

            _transaction1.Include(_columns, 8, new object[] { 8, "Scott", "Pilgrim", true, "10/13/1979", "Moscow" });
            _transaction2.Include(_columns, 8, new object[] { 8, "Martha", "Stewart", false, "4/10/1967", "San Jose" });
            _transaction1.Commit();
            bool flag = false;
            try
            {
                _transaction2.Commit();
            }
            catch
            {
                flag = true;
            }
            Assert.AreEqual(flag, true);

            _database.Exclude(_columns, 8);

            _transaction1 = _database.Begin(false, false);
            _database.Include(_columns, 8, new object[] { 8, "Scott", "Pilgrim", true, "10/13/1979", "Moscow" });
            _transaction2 = _database.Begin(false, false);
            _database.Include(_columns, 9, new object[] { 9, "Martha", "Stewart", false, "4/10/1967", "San Jose" });

            _transaction1.Commit();
            _transaction2.Commit();

            data = _database.GetRange(
                _columns,
                new[] { new Order { ColumnID = 1000, Ascending = true } },
                new[] { new Range { ColumnID = 1000, Limit = 9 } }
                );
            Assert.AreEqual(data.Count, 9);

        }
        
        [TestMethod]
        public void TestDispose()
        {

        }

        [TestMethod]
        public void TestRollback()
        {

        }

        [TestMethod]
        public void TestEnsureColumnLog()
        {

        }
    }
}
