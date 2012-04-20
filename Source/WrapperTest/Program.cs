using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using System.IO;
using Microsoft.VisualBasic;
using Wrapper;

namespace WrapperTest
{
    class Program
    {
        static void Main(string[] args)
        {
           
            Microsoft.VisualBasic.FileIO.TextFieldParser parser = new Microsoft.VisualBasic.FileIO.TextFieldParser(@"C:\owt.txt");
            parser.Delimiters = new string[] { "^" };

            ManagedColumnDef c1 = new ManagedColumnDef();
	        c1.IsUnique = true;
	        c1.KeyType = "Int";
	        c1.Name = "ID";

            ManagedColumnDef c2 = new ManagedColumnDef();
	        c2.IsUnique = false;
	        c2.KeyType = "String";
	        c2.Name = "Given";

            ManagedColumnDef c3 = new ManagedColumnDef();
	        c3.IsUnique = false;
	        c3.KeyType = "String";
	        c3.Name = "Surname";

            ManagedColumnDef c4 = new ManagedColumnDef();
	        c4.IsUnique = false;
	        c4.KeyType = "Bool";
	        c4.Name = "Gender";

            ManagedColumnDef c5 = new ManagedColumnDef();
	        c5.IsUnique = false;
	        c5.KeyType = "String";
	        c5.Name = "BirthDate";

            ManagedColumnDef c6 = new ManagedColumnDef();
	        c6.IsUnique = false;
	        c6.KeyType = "String";
	        c6.Name = "BirthPlace";

            ManagedTopology topo = new ManagedTopology();

            topo.Add(c1);
            topo.Add(c2);
            topo.Add(c3);
            topo.Add(c4);
            topo.Add(c5);
            topo.Add(c6);

            ManagedHostFactory hf = new ManagedHostFactory();
            var host = hf.Create(topo);

            var db = new ManagedDatabase(host);

            var session = db.Start();

            string[] columns = { "ID", "Given", "Surname", "Gender", "BirthDate", "BirthPlace" };

            Stopwatch watch = new Stopwatch();
            Stopwatch watchInner = new Stopwatch();
            watch.Start();
            int numrows = 100000;
            for (int i = 0; i < numrows; i++)
            {
                var strings = parser.ReadFields();
                object[] objects = new object[6];

                int id = int.Parse(strings[0]);
                bool gender = strings[3].StartsWith("T");

                objects[0] = id;
                objects[3] = gender;
                objects[1] = strings[1];
                objects[2] = strings[2];
                objects[4] = strings[4];
                objects[5] = strings[5];

                watchInner.Start();
                session.Include(objects, columns, false);
                watchInner.Stop();
            }
            watch.Stop();

            //Stopwatch is not accurate...
            Console.WriteLine("Row per second (excluding parsing): " + (watchInner.ElapsedMilliseconds / 1000.0) / numrows);
            Console.WriteLine("Row per second (including parsing): " + (watch.ElapsedMilliseconds / 1000.0) / numrows);

            int numpull = 50;
            object[] rowIds = new object[numpull];
            for(int i = 0; i < numpull; i++)
            {
                rowIds[i] = i;
            }

            ManagedRangeBound bound1 = new ManagedRangeBound(100, null, true);
            ManagedRangeBound bound2 = new ManagedRangeBound(1000, null, true);

            ManagedRange range = new ManagedRange(100, false, bound1, bound2);

            var result = session.GetRange(columns, range, 0);

            result.Dump();
            
            Console.ReadLine();
        }
    }
}
