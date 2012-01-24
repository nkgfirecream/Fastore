using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Fastore.Core;
using System.Diagnostics;
using Fastore.Core.Test;

namespace Fastore.Core.Test
{
    [TestClass]
    public class PatriciaTrieTests
    {
        [TestMethod]
        public void PatriciaInsert()
        {
            var tree = new PatriciaTrie<string>();

            tree.Insert("romane", "romane");
            tree.Insert("romanus", "romanus");
            tree.Insert("romulus", "romulus");
            tree.Insert("rubens", "rubens");
            tree.Insert("rubensandwich", "rubensandwhich");
            tree.Insert("ruber", "ruber");
            tree.Insert("rubicon", "rubicon");
            tree.Insert("rubicundus", "rubicundus");
            tree.Insert("rubber", "rubber");
            tree.Insert("hobo", "hobo");
            tree.Insert("homo", "homo");
            tree.Insert("romane", "romane");
            tree.Insert("roman", "roman");
            tree.Insert("water", "water");
            tree.Insert("waterproof", "waterproof");
            tree.Insert("waiter", "waiter");
            tree.Insert("wait", "wait");
            tree.Insert("waiting", "waiting");
            tree.Insert("watertight", "watertight");
            tree.Insert("waterfall", "waterfall");

            Debug.WriteLine(tree.GetValue("roman"));
            Debug.WriteLine(tree.GetValue("romane"));
            Debug.WriteLine(tree.GetValue("bobo"));
            Debug.WriteLine(tree.GetValue("hobo"));
            Debug.WriteLine(tree.GetValue("ro"));
            Debug.WriteLine(tree.GetValue("rubensandwich"));
            Debug.WriteLine(tree.GetValue("waiter"));
            Debug.WriteLine(tree.GetValue("water"));
            Debug.WriteLine(tree.GetValue("waterproof"));
            Debug.WriteLine(tree.GetValue("rubber"));

            Debug.WriteLine(tree.ToString());
            Debug.WriteLine(tree.Count);
        }

        [TestMethod]
		public void PatriciaDelete()
        {
            var tree = new PatriciaTrie<string>();

            tree.Insert("romane", "romane");
            tree.Insert("romanus", "romanus");
            tree.Insert("romulus", "romulus");          
            tree.Insert("ruber", "ruber");
            tree.Insert("rubicon", "rubicon");
            tree.Insert("rubicundus", "rubicundus");
            tree.Insert("rubber", "rubber");

            Debug.WriteLine(tree.ToString());
            Debug.WriteLine(tree.Count);

            tree.Insert("rubens", "rubens");
            tree.Insert("rubensandwich", "rubensandwhich");
            tree.Insert("hobo", "hobo");
            tree.Insert("homo", "homo");
            tree.Insert("roman", "roman");
            tree.Insert("water", "water");
            tree.Insert("waterproof", "waterproof");
            tree.Insert("waiter", "waiter");
            tree.Insert("wait", "wait");
            tree.Insert("waiting", "waiting");
            tree.Insert("watertight", "watertight");
            tree.Insert("waterfall", "waterfall");

            Debug.WriteLine(tree.ToString());
            Debug.WriteLine(tree.Count);

            tree.Delete("watertight");
            tree.Delete("waterfall");
            tree.Delete("hobo");
            tree.Delete("homo");
            tree.Delete("roman");
            tree.Delete("water");
            tree.Delete("waterproof");
            tree.Delete("waiter");
            tree.Delete("wait");
            tree.Delete("rubens");
            tree.Delete("rubensandwich");
            tree.Delete("waiting");
            tree.Delete("rubens");


            //Trees one and three should be identical
            //TODO: Visual check for now, add enumeration and ensure that it enumerates the same way.
            Debug.WriteLine(tree.ToString());
            Debug.WriteLine(tree.Count);

            tree.Delete("romane");
            tree.Delete("romanus");
            tree.Delete("romulus");
            tree.Delete("ruber");
            tree.Delete("rubicon");
            tree.Delete("rubicundus");
            tree.Delete("rubber");


            //Should be empty
            Debug.WriteLine(tree.ToString());
            Debug.WriteLine(tree.Count);
        }

        [TestMethod]
		public void PatriciaSplit()
        {
            var tree = new PatriciaTrie<string>();

            tree.Insert("romane", "romane");
            tree.Insert("romanus", "romanus");
            tree.Insert("romulus", "romulus");
            tree.Insert("ruber", "ruber");
            tree.Insert("rubicon", "rubicon");
            tree.Insert("rubicundus", "rubicundus");
            tree.Insert("rubber", "rubber");
            tree.Insert("rubens", "rubens");
            tree.Insert("rubensandwich", "rubensandwhich");
            tree.Insert("hobo", "hobo");
            tree.Insert("homo", "homo");
            tree.Insert("roman", "roman");
            tree.Insert("water", "water");
            tree.Insert("waterproof", "waterproof");
            tree.Insert("waiter", "waiter");
            tree.Insert("wait", "wait");
            tree.Insert("waiting", "waiting");
            tree.Insert("watertight", "watertight");
            tree.Insert("waterfall", "waterfall");

            //tree.DisplayAsTree();
            //Debug.WriteLine("");
            //var tree2 = tree.SplitTreeAtKey("water");

            //tree.DisplayAsTree();
            //Debug.WriteLine("");
            //tree2.DisplayAsTree();
            //Debug.WriteLine("");

            //var tree3 = tree.SplitTreeAtKey("roman");
            //tree.DisplayAsTree();
            //Debug.WriteLine("");
            //tree3.DisplayAsTree();
        }

        [TestMethod]
		public void PatriciaInsertSpeed()
        {
            int numrows = 1000000;
            //var test = new BTree<string,Guid>();
            var test = new PatriciaTrie<Guid>();

            IKeyValueLeaf<string, Guid> dummy;
            Debug.WriteLine("Inserting Rows...");
            var watch = new Stopwatch();

            for (int j = 0; j < numrows; j++)
            {
                var value = Guid.NewGuid();
                var key = RandomG.RandomString(RandomG.RandomInt(8));
                watch.Start();
                //test.Insert(key, value, out dummy);
                test.Insert(key, value);
                watch.Stop();
            }

            Debug.WriteLine("Inserts Per Second: " + (double)numrows / ((double)watch.ElapsedMilliseconds / 1000));
        }

        [TestMethod]
		public void PatriciaGetValues()
        {
            var tree = new PatriciaTrie<string>();
            tree.Insert("romane", "romane");
            tree.Insert("romanus", "romanus");
            tree.Insert("romulus", "romulus");
            tree.Insert("ruber", "ruber");
            tree.Insert("rubicon", "rubicon");
            tree.Insert("rubicundus", "rubicundus");
            tree.Insert("rubber", "rubber");
            tree.Insert("rubens", "rubens");
            tree.Insert("rubensandwich", "rubensandwhich");
            tree.Insert("hobo", "hobo");
            tree.Insert("homo", "homo");
            tree.Insert("roman", "roman");
            tree.Insert("water", "water");
            tree.Insert("waterproof", "waterproof");
            tree.Insert("waiter", "waiter");
            tree.Insert("wait", "wait");
            tree.Insert("waiting", "waiting");
            tree.Insert("watertight", "watertight");
            tree.Insert("waterfall", "waterfall");

            //foreach (var item in tree.GetValues())
            //    Debug.WriteLine(item);

            //Debug.WriteLine("");
            //Debug.WriteLine(tree.GetValue("roman"));
            //Debug.WriteLine(tree.GetValue("hobo"));
            //Debug.WriteLine(tree.GetValue("romulus"));
        }
    }
}
