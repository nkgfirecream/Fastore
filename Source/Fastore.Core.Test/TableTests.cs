using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Diagnostics;
using Fastore.Core;

namespace Fastore.Core.Test
{
	[TestClass]
	public class EngineTests
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


			var test = new BTree<int, int>() { BranchingFactor = 4, LeafSize = 4 };

			IBTreeLeaf<int, int> dummy;
			for (int i = 0; i < 100; i++)
			{
				test.Insert(i, i, out dummy);
			}

			Console.WriteLine(test.ToString());

			test = new BTree<int, int>() { BranchingFactor = 32, LeafSize = 16 };

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
			IBTreeLeaf<Guid, string> dummy;
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
					var tree = new BTree<Guid, string>(null) { BranchingFactor = factor, LeafSize = 8 };

					watch.Reset();
					watch.Start();
					for (int j = 0; j <= 500000; j++)
					{
						tree.Insert(Guid.NewGuid(), RandomG.RandomString(RandomG.RandomInt(8)), out dummy);
					}

					watch.Stop();

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
					var tree = new BTree<Guid, string>(null) { BranchingFactor = bestFactor, LeafSize = factor };

					watch.Reset();
					watch.Start();
					for (int j = 0; j <= 500000; j++)
					{
						tree.Insert(Guid.NewGuid(), RandomG.RandomString(RandomG.RandomInt(8)), out dummy);
					}

					watch.Stop();

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
			for (int x = 0; x <= 5; x++)
			{
				test = new BTree<Guid, string>(null) { BranchingFactor = bestFactor, LeafSize = bestSize };

				watch.Reset();
				watch.Start();
				for (int j = 0; j <= 100000; j++)
				{
					test.Insert(Guid.NewGuid(), RandomG.RandomString(RandomG.RandomInt(8)), out dummy);
				}

				watch.Stop();

				totaltime += watch.ElapsedMilliseconds;

				Debug.WriteLine("Load Test: {0}", x);
			}

			Debug.WriteLine("Total Number of Seconds Elapsed: " + ((double)totaltime / 1000));
			Debug.WriteLine("Inserts Per Second: " + (double)500000 / ((double)totaltime / 1000));


			Debug.WriteLine("Sorted List Test");
			SortedList<Guid, string> test2;
			totaltime = 0;
			for (int x = 0; x <= 5; x++)
			{
				test2 = new SortedList<Guid, string>();

				watch.Reset();
				watch.Start();
				for (int j = 0; j <= 10000; j++)
				{
					test2.Add(Guid.NewGuid(), RandomG.RandomString(RandomG.RandomInt(8)));
				}

				watch.Stop();

				totaltime += watch.ElapsedMilliseconds;

				Debug.WriteLine("Load Test: {0}", x);
			}

			Debug.WriteLine("Total Number of Seconds Elapsed: " + ((double)totaltime / 1000));
			Debug.WriteLine("Inserts Per Second: " + (double)50000 / ((double)totaltime / 1000));

			Debug.WriteLine("Dictionary Test");
			Dictionary<Guid, string> test3;
			totaltime = 0;
			for (int x = 0; x <= 5; x++)
			{
				test3 = new Dictionary<Guid, string>();

				watch.Reset();
				watch.Start();
				for (int j = 0; j <= 100000; j++)
				{
					test3.Add(Guid.NewGuid(), RandomG.RandomString(RandomG.RandomInt(8)));
				}

				watch.Stop();

				totaltime += watch.ElapsedMilliseconds;

				Debug.WriteLine("Load Test: {0}", x);
			}

			Debug.WriteLine("SortedDictionary Test");
			SortedDictionary<Guid, string> test4;
			totaltime = 0;
			for (int x = 0; x <= 5; x++)
			{
				test4 = new SortedDictionary<Guid, string>();

				watch.Reset();
				watch.Start();
				for (int j = 0; j <= 100000; j++)
				{
					test4.Add(Guid.NewGuid(), RandomG.RandomString(RandomG.RandomInt(8)));
				}

				watch.Stop();

				totaltime += watch.ElapsedMilliseconds;

				Debug.WriteLine("Load Test: {0}", x);
			}

			Debug.WriteLine("Total Number of Seconds Elapsed: " + ((double)totaltime / 1000));
			Debug.WriteLine("Inserts Per Second: " + (double)500000 / ((double)totaltime / 1000));
		}

		[TestMethod]
		public void TableTest1()
		{
			int numrows = 1000000;
			var table = new Table();
			table.AddColumn(0, new ColumnDef("ID", typeof(Guid)));
			table.AddColumn(1, new ColumnDef("Name", typeof(string)));
			table.AddColumn(2, new ColumnDef("Age", typeof(int)));
			table.AddColumn(3, new ColumnDef("Comments", typeof(string)));

			Debug.WriteLine("Inserting Rows...");
			var watch = new Stopwatch();
			for (int j = 0; j < numrows; j++)
			{
				var row = new object[] { Guid.NewGuid(), RandomG.RandomString(RandomG.RandomInt(8)), RandomG.RandomInt(99), RandomG.RandomString(RandomG.RandomInt(16)) };
				watch.Start();
				table.Insert(row);
				watch.Stop();
			}

			Debug.WriteLine("Inserts Per Second: " + (double)numrows / ((double)watch.ElapsedMilliseconds / 1000));

			Debug.WriteLine("Reconstructing Rows...");

			watch.Reset();
			var selection = table.Select(0, true, null);
			int i = 0;
			watch.Start();
			foreach (var item in selection)
			{
				//Debug.WriteLine(item);
				i++;
			}
			watch.Stop();
			Debug.WriteLine(i);

			Debug.WriteLine("Rows Reconstructed per Second: " + (double)numrows / ((double)watch.ElapsedMilliseconds / 1000));
		}

		[TestMethod]
		public void BTreeTest4()
		{
			int numrows = 10;
			var test = new BTree<Guid, string>(null) { BranchingFactor = 32, LeafSize = 32 };

			IBTreeLeaf<Guid, string> dummy;
			Debug.WriteLine("Inserting Rows...");
			var watch = new Stopwatch();
			watch.Start();
			for (int j = 0; j < numrows; j++)
			{
				test.Insert(Guid.NewGuid(), RandomG.RandomString(RandomG.RandomInt(8)), out dummy);
			}
			watch.Stop();

			Debug.WriteLine("Inserts Per Second: " + (double)numrows / ((double)watch.ElapsedMilliseconds / 1000));

			Debug.WriteLine("Reconstructing Rows...");

			Debug.WriteLine(test.ToString());
		}
	}
}
