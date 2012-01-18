using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Fastore.Core;
using System.Diagnostics;

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

            tree.DisplayAsTree();
        }
    }
}
