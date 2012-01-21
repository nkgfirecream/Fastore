using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualBasic.FileIO;
using Fastore.Core;
using System.Diagnostics;

namespace Fastore.Core.Test
{
    /// <summary>
    /// Summary description for MemoryUsageTests
    /// </summary>
    [TestClass]
    public class MemoryUsageTests
    {

        [TestMethod]
        public void TestMethod1()
        {
            TextFieldParser parser = new TextFieldParser(@"C:\owt.person.csv");
            parser.Delimiters = new string[] { "," };

            var test = new PatriciaTrie<long>();

            IKeyValueLeaf<string, int> dummy;
            var watch = new Stopwatch();

            for (int i = 0; i < 100000; i++)
            {
                var line = parser.ReadFields()[1];
                watch.Start();
                test.Insert(line, i);
                watch.Stop();
            }

            Debug.WriteLine(100000 / ((double)watch.ElapsedMilliseconds / 1000));
        }

        [TestMethod]
        public void TestMethod2()
        {
            TextFieldParser parser = new TextFieldParser(@"C:\owt.person.csv");
            parser.Delimiters = new string[] { "," };

            var test = new BTree<string, int>();

            //var test = new SimplePrefixBTree<int>();

            IKeyValueLeaf<string, int> dummy;

            var watch = new Stopwatch();

            for (int i = 0; i < 100000; i++)
            {
                var line = parser.ReadFields()[1];
                watch.Start();
                test.Insert(line, i, out dummy);
                watch.Stop();
            }

            Debug.WriteLine(100000 / ((double)watch.ElapsedMilliseconds / 1000));
        }

        [TestMethod]
        public void TestMethod3()
        {
            TextFieldParser parser = new TextFieldParser(@"C:\owt.person.csv");
            parser.Delimiters = new string[] { "," };

            var test = new SimplePrefixBTree<int>();

            IKeyValueLeaf<string, int> dummy;

            var watch = new Stopwatch();

            for (int i = 0; i < 100000; i++)
            {
                var line = parser.ReadFields()[1];
                watch.Start();
                test.Insert(line, i, out dummy);
                watch.Stop();
            }

            Debug.WriteLine(100000 / ((double)watch.ElapsedMilliseconds / 1000));
        }


    }
}
