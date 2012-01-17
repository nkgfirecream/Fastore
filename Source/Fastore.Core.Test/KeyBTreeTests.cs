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
    public class KeyBTreeTests
    {
        [TestMethod]
        public void TestOrdering()
        {
            var btree = new KeyBTree<long>(Comparer<long>.Default);

            IKeyLeaf<long> dummy;
            long i = 0;
            for (i = 0; i <= 10000; i++)
            {
                btree.Insert(i, out dummy);
            }

            i = 0;
            foreach (var item in btree.Get(true))
            {
                if (item != i)
                    throw new Exception("Out of order items in tree");

                i++;
            }

            if (i != 10001)
                throw new Exception("Missing Items!");

            i = 10000;
            foreach (var item in btree.Get(false))
            {
                if (item != i)
                    throw new Exception("Out of order items in tree");

                i--;
            }

            if (i != -1)
                throw new Exception("Missing Items!");

            btree = new KeyBTree<long>(Comparer<long>.Default);
            for (i = 10000; i >=0; i--)
            {
                btree.Insert(i, out dummy);
            }

            if (i != -1)
                throw new Exception("Missing Items!");

            i = 0;
            foreach (var item in btree.Get(true))
            {
                if (item != i)
                    throw new Exception("Out of order items in tree");

                i++;
            }


            if (i != 10001)
                throw new Exception("Missing Items!");


            i = 10000;
            foreach (var item in btree.Get(false))
            {
                if (item != i)
                    throw new Exception("Out of order items in tree");

                i--;
            }

            if (i != -1)
                throw new Exception("Missing Items!");

        }
    }
}
