using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Fastore.Core;
using System.Diagnostics;
using Fastore.Core.Test;

namespace Fastore.Engine.Test
{
    [TestClass]
    public class PatriciaTrieTests
    {
        [TestMethod]
        public void TestInsert()
        {
            var tree = new PatriciaTrie<string>();

            tree.Insert("romane", "romane");
            tree.Insert("romanus", "romanus");
            tree.Insert("romulus", "romulus");
            tree.Insert("rubens", "rubens");
            tree.Insert("ruber", "ruber");
            tree.Insert("rubicon", "rubicon");
            tree.Insert("rubicundus", "rubicundus");
            tree.Insert("rubber", "rubber");
            tree.Insert("hobo", "hobo");
            tree.Insert("homo", "homo");
            tree.Insert("romane", "romane");
            tree.Insert("roman", "roman");

            Debug.WriteLine(tree.GetValue("roman"));
            Debug.WriteLine(tree.GetValue("romane"));
            Debug.WriteLine(tree.GetValue("bobo"));
            Debug.WriteLine(tree.GetValue("hobo"));
            Debug.WriteLine(tree.GetValue("ro"));

            tree.DisplayAsTree();
        }

        [TestMethod]
        public void TestInsertSpeed()
        {
            int numrows = 1000;
            var test = new PatriciaTrie<Guid>();

            Debug.WriteLine("Inserting Rows...");
            var watch = new Stopwatch();

            for (int j = 0; j < numrows; j++)
            {
                var key = Guid.NewGuid();
                var value = RandomG.RandomString(RandomG.RandomInt(8));
                watch.Start();
                test.Insert(value, key);
                watch.Stop();
            }

            Debug.WriteLine("Inserts Per Second: " + (double)numrows / ((double)watch.ElapsedMilliseconds / 1000));
        }
    }
}
