using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Alphora.Fastore.Client;

namespace Fastore.Client.Test
{
    class TestSetup
    {
        internal Database _database;
        internal int[] _columns;

        public void createTable(int[] cols)
        {
            string hostname = "localhost";
            const int port = 8064;
            _database = Alphora.Fastore.Client.Client.Connect(hostname, port);

            int[] _schemaColumns = new int[] { 0, 1, 2, 3, 4 };

            _database.Include(_schemaColumns, cols[0], new object[] { cols[0], "ID", "Int", "Int", true });
            _database.Include(_schemaColumns, cols[1], new object[] { cols[1], "Given", "String", "Int", false });
            _database.Include(_schemaColumns, cols[2], new object[] { cols[2], "Surname", "String", "Int", false });
            _database.Include(_schemaColumns, cols[3], new object[] { cols[3], "Gender", "Bool", "Int", false });
            _database.Include(_schemaColumns, cols[4], new object[] { cols[4], "BirthDate", "String", "Int", false });
            _database.Include(_schemaColumns, cols[5], new object[] { cols[5], "BirthPlace", "String", "Int", false });
        }

        public void addData(Database db, int[] cols)
        {
            db.Include(cols, 1, new object[] { 1, "Joe", "Shmoe", true, "5/26/1980", "Antarctica" });
            db.Include(cols, 2, new object[] { 2, "Ann", "Shmoe", false, "4/20/1981", "Denver" });
            db.Include(cols, 3, new object[] { 3, "Sarah", "Silverman", false, "11/10/1976", "Chicago" });
            db.Include(cols, 4, new object[] { 4, "Bob", "Newhart", true, "12/2/1970", "Paris" });
            db.Include(cols, 5, new object[] { 5, "Samantha", "Smith", false, "1/13/1984", "Tokyo" });
            db.Include(cols, 6, new object[] { 6, "Andy", "Warhol", true, "9/14/1987", "New York" });
            db.Include(cols, 7, new object[] { 7, "Carl", "Sagan", true, "4/1/1957", "Las Vegas" });
        }
    }
}
