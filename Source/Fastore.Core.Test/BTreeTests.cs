﻿using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Fastore.Core;
using System.Diagnostics;
using Fastore.Core.Test;

namespace Fastore.Core.Test
{
    [TestClass]
    public class BTreeTests
    {
        [TestMethod]
        public void BTreeTest1()
        {
            //var test = new BTree<string, string>(8, 8, null, 0, StringComparer.CurrentCulture);

            //Leaf<string, string> dummy;
            //var alpha = 'a';
            //for (int i = 0; i < 26; i++)
            //{
            //    test.Insert(alpha.ToString(), alpha.ToString(), out dummy);
            //    alpha++;
            //}

            //alpha = 'A';
            //for (int i = 0; i < 26; i++)
            //{
            //    test.Insert(alpha.ToString(), alpha.ToString(), out dummy);
            //    alpha++;
            //}

            //test.Dump();

            //Debug.WriteLine();
            //test = new BTree<string, string>(4, 4, null, 0);

            //alpha = 'z';
            //for (int i = 26; i > 0; i--)
            //{
            //    test.Insert(alpha.ToString(), alpha.ToString(), out dummy);
            //    alpha--;
            //}

            //alpha = 'Z';
            //for (int i = 26; i > 0; i--)
            //{
            //    test.Insert(alpha.ToString(), alpha.ToString(), out dummy);
            //    alpha--;
            //}

            //test.Dump();


            var test = new BTree<int, int>(fanout: 4, leafSize: 4);

            IKeyValueLeaf<int, int> dummy;
            for (int i = 0; i < 100; i++)
            {
                test.Insert(i, i, out dummy);
            }

            Debug.WriteLine(test.ToString());

            test = new BTree<int, int>(fanout: 32, leafSize: 16);

            for (int i = 99; i >= 0; i--)
            {
                test.Insert(i, i, out dummy);
            }

            Debug.WriteLine(test.ToString());


            //var test2 = new BTree<Guid, int>(4, 4, null, 0, Comparer<Guid>.Default);
            //Leaf<Guid, int> dummy2;
            //for (int i = 1; i < 20000; i++)
            //{
            //    test2.Insert(Guid.NewGuid(), i, out dummy2);
            //}

            //test2.Dump();
        }

        [TestMethod]
        public void BTreeTest2()
        {
            IKeyValueLeaf<Guid, string> dummy;
            Debug.WriteLine("BTree tests");

            Debug.WriteLine("Branching Factor Tests");

            int bestFactor = 0;
            long bestTime = long.MaxValue;

            var watch = new Stopwatch();
            for (int i = 2; i < 8; i++)
            {
                int factor = (int)Math.Pow(2, i);
                for (int x = 0; x < 1; x++)
                {
                    var tree = new BTree<Guid, string>(fanout: factor, leafSize: 8);

                    watch.Reset();
                    for (int j = 0; j <= 500000; j++)
                    {
                        var key = Guid.NewGuid();
                        var value = RandomG.RandomString(RandomG.RandomInt(8));
                        watch.Start();
                        tree.Insert(key, value, out dummy);
                        watch.Stop();
                    }

                    var time = watch.ElapsedTicks;

                    if (time < bestTime)
                    {
                        bestFactor = factor;
                        bestTime = time;
                    }

                    Debug.WriteLine("Factor:  {0}, Time: {1}", factor, time);
                }
            }

            int bestSize = 0;
            for (int i = 2; i < 10; i++)
            {
                int factor = (int)Math.Pow(2, i);
                for (int x = 0; x < 1; x++)
                {
                    var tree = new BTree<Guid, string>(fanout: bestFactor, leafSize: factor);

                    watch.Reset();
                    for (int j = 0; j <= 500000; j++)
                    {
                        var key = Guid.NewGuid();
                        var value = RandomG.RandomString(RandomG.RandomInt(8));
                        watch.Start();
                        tree.Insert(key, value, out dummy);
                        watch.Stop();
                    }

                    var time = watch.ElapsedTicks;

                    if (time < bestTime)
                    {
                        bestSize = factor;
                        bestTime = time;
                    }

                    Debug.WriteLine("Size:  {0}, Time: {1}", factor, time);
                }
            }

            Debug.WriteLine("Best Factor: {0}, Best Size: {0}, Best Time {1}", bestFactor, bestSize, bestTime);

            BTree<Guid, string> test;
            long totaltime = 0;
            watch.Reset();
            for (int x = 0; x <= 5; x++)
            {
                test = new BTree<Guid, string>(fanout: bestFactor, leafSize: bestSize);

                for (int j = 0; j <= 100000; j++)
                {
                    var key = Guid.NewGuid();
                    var value = RandomG.RandomString(RandomG.RandomInt(8));
                    watch.Start();
                    test.Insert(key, value, out dummy);
                    watch.Stop();
                }

                Debug.WriteLine("Load Test: {0}", x);
            }
            totaltime = watch.ElapsedMilliseconds;

            Debug.WriteLine("Total Number of Seconds Elapsed: " + ((double)totaltime / 1000));
            Debug.WriteLine("Inserts Per Second: " + (double)500000 / ((double)totaltime / 1000));


            Debug.WriteLine("Sorted List Test");
            SortedList<Guid, string> test2;
            totaltime = 0;
            watch.Reset();
            for (int x = 0; x <= 5; x++)
            {
                test2 = new SortedList<Guid, string>();

                for (int j = 0; j <= 10000; j++)
                {
                    var key = Guid.NewGuid();
                    var value = RandomG.RandomString(RandomG.RandomInt(8));
                    watch.Start();
                    test2.Add(key, value);
                    watch.Stop();
                }


                Debug.WriteLine("Load Test: {0}", x);
            }
            totaltime = watch.ElapsedMilliseconds;

            Debug.WriteLine("Total Number of Seconds Elapsed: " + ((double)totaltime / 1000));
            Debug.WriteLine("Inserts Per Second: " + (double)50000 / ((double)totaltime / 1000));

            Debug.WriteLine("Dictionary Test");
            Dictionary<Guid, string> test3;
            totaltime = 0;
            watch.Reset();
            for (int x = 0; x <= 5; x++)
            {
                test3 = new Dictionary<Guid, string>();

                for (int j = 0; j <= 100000; j++)
                {
                    var key = Guid.NewGuid();
                    var value = RandomG.RandomString(RandomG.RandomInt(8));
                    watch.Start();
                    test3.Add(key, value);
                    watch.Stop();
                }

                Debug.WriteLine("Load Test: {0}", x);
            }
            totaltime = watch.ElapsedMilliseconds;

            Debug.WriteLine("Total Number of Seconds Elapsed: " + ((double)totaltime / 1000));
            Debug.WriteLine("Inserts Per Second: " + (double)500000 / ((double)totaltime / 1000));

            Debug.WriteLine("SortedDictionary Test");
            SortedDictionary<Guid, string> test4;
            totaltime = 0;
            watch.Reset();
            for (int x = 0; x <= 5; x++)
            {
                test4 = new SortedDictionary<Guid, string>();

                for (int j = 0; j <= 100000; j++)
                {
                    var key = Guid.NewGuid();
                    var value = RandomG.RandomString(RandomG.RandomInt(8));
                    watch.Start();
                    test4.Add(key, value);
                    watch.Stop();
                }

                Debug.WriteLine("Load Test: {0}", x);
            }
            totaltime = watch.ElapsedMilliseconds;

            Debug.WriteLine("Total Number of Seconds Elapsed: " + ((double)totaltime / 1000));
            Debug.WriteLine("Inserts Per Second: " + (double)500000 / ((double)totaltime / 1000));

            Debug.WriteLine("List Test");
            List<string> test5;
            totaltime = 0;
            watch.Reset();
            for (int x = 0; x <= 5; x++)
            {
                test5 = new List<string>();

                for (int j = 0; j <= 100000; j++)
                {
                    
                    var value = RandomG.RandomString(RandomG.RandomInt(8));
                    watch.Start();
                    test5.Add(value);
                    watch.Stop();
                }

                Debug.WriteLine("Load Test: {0}", x);
            }
            totaltime = watch.ElapsedMilliseconds;

            Debug.WriteLine("Total Number of Seconds Elapsed: " + ((double)totaltime / 1000));
            Debug.WriteLine("Inserts Per Second: " + (double)500000 / ((double)totaltime / 1000));
        }


        [TestMethod]
        public void BTreeRandomStringTimings()
        {
            using (var outStream = new System.IO.FileStream("results.txt", System.IO.FileMode.Create, System.IO.FileAccess.Write))
            {
                Trace.Listeners.Add(new TextWriterTraceListener(outStream));
                int numrows = 1000000;
                var test = new SimplePrefixBTree<string>(fanout: 128, leafSize: 128);
                var test3 = new BTree<string, string>(fanout: 128, leafSize: 128);
                var test4 = new KeyBTree<string>(null);

                IKeyValueLeaf<string, string> dummy;
                IKeyLeaf<string> dummy2;
                Trace.WriteLine("Inserting Rows...");
                var watch = new Stopwatch();
                var watch3 = new Stopwatch();
                var watch4 = new Stopwatch();
                for (int j = 0; j < numrows; j++)
                {
                    var value = RandomG.RandomString(RandomG.RandomInt(8));
                    watch.Start();
                    test.Insert(value, value, out dummy);
                    watch.Stop();

                    watch3.Start();
                    test3.Insert(value, value, out dummy);
                    watch3.Stop();

                    watch4.Start();
                    test4.Insert(value, out dummy2);
                    watch4.Stop();
                }

                Trace.WriteLine("Inserts Per Second SimplePrefix: " + (double)numrows / ((double)watch.ElapsedMilliseconds / 1000));
                Trace.WriteLine("Inserts Per Second Btree: " + (double)numrows / ((double)watch3.ElapsedMilliseconds / 1000));
                Trace.WriteLine("Inserts Per Second KeyBtree: " + (double)numrows / ((double)watch4.ElapsedMilliseconds / 1000));

                Trace.Flush();
            }
        }



        [TestMethod]
        public void BTreeRandomGUIDTimings()
        {
			using (var outStream = new System.IO.FileStream("results.txt", System.IO.FileMode.Create, System.IO.FileAccess.Write))
			{
				Trace.Listeners.Add(new TextWriterTraceListener(outStream));
				int numrows = 1000000;
				var test = new SimplePrefixBTree<Guid>(fanout: 128, leafSize: 128);
				var test3 = new BTree<string, Guid>(fanout: 128, leafSize: 128);
				var test4 = new KeyBTree<Guid>(null);

				IKeyValueLeaf<string, Guid> dummy;
				IKeyLeaf<Guid> dummy2;
				Trace.WriteLine("Inserting Rows...");
				var watch = new Stopwatch();
				var watch3 = new Stopwatch();
				var watch4 = new Stopwatch();
				for (int j = 0; j < numrows; j++)
				{
					var key = Guid.NewGuid();
					var value = RandomG.RandomString(RandomG.RandomInt(8));
					watch.Start();
					test.Insert(value, key, out dummy);
					watch.Stop();

					watch3.Start();
					test3.Insert(value, key, out dummy);
					watch3.Stop();

					watch4.Start();
					test4.Insert(key, out dummy2);
					watch4.Stop();
				}

				Trace.WriteLine("Inserts Per Second SimplePrefix: " + (double)numrows / ((double)watch.ElapsedMilliseconds / 1000));
				Trace.WriteLine("Inserts Per Second Btree: " + (double)numrows / ((double)watch3.ElapsedMilliseconds / 1000));
				Trace.WriteLine("Inserts Per Second KeyBtree: " + (double)numrows / ((double)watch4.ElapsedMilliseconds / 1000));

				Trace.Flush();
			}
        }

		[TestMethod]
		public void BTreeIntsTimings()
		{
			using (var outStream = new System.IO.FileStream("results.txt", System.IO.FileMode.Create, System.IO.FileAccess.Write))
			{
				Trace.Listeners.Add(new TextWriterTraceListener(outStream));
				int numrows = 1000000;
				var test2 = new BTree<long, long>(fanout: 128, leafSize: 128);
				var test3 = new BTree<int, int>(fanout: 128, leafSize: 128);
				var test4 = new KeyBTree<int>(null);

				IKeyValueLeaf<int, int> dummy;
				IKeyLeaf<int> dummy2;
				IKeyValueLeaf<long, long> dummy3;
				Trace.WriteLine("Inserting Rows...");
				var watch2 = new Stopwatch();
				var watch3 = new Stopwatch();
				var watch4 = new Stopwatch();
				for (int j = 0; j < numrows; j++)
				{
					var v = RandomG.RandomLong();
					watch2.Start();
					test2.Insert(v, v, out dummy3);
					watch2.Stop();

					watch3.Start();
					test3.Insert(j, j, out dummy);
					watch3.Stop();

					watch4.Start();
					test4.Insert(j, out dummy2);
					watch4.Stop();
				}

				Trace.WriteLine("Inserts Per Second Btree (random): " + (double)numrows / ((double)watch2.ElapsedMilliseconds / 1000));
				Trace.WriteLine("Inserts Per Second Btree: " + (double)numrows / ((double)watch3.ElapsedMilliseconds / 1000));
				Trace.WriteLine("Inserts Per Second KeyBtree: " + (double)numrows / ((double)watch4.ElapsedMilliseconds / 1000));

				Trace.Flush();
			}
		}

        [TestMethod]
        public void BTreeTest5()
        {
            int numrows = 1000000;
            var test = new KeyBTree<string>(Comparer<string>.Default);

            IKeyLeaf<string> dummy;
            Debug.WriteLine("Inserting Rows...");
            var watch = new Stopwatch();
      
            for (int j = 0; j < numrows; j++)
            {
                var value = RandomG.RandomString(RandomG.RandomInt(8));
                watch.Start();
                test.Insert(value, out dummy);
                watch.Stop();
            }

            Debug.WriteLine("Inserts Per Second: " + (double)numrows / ((double)watch.ElapsedMilliseconds / 1000));
        }
    }
}
