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
	public class TableTests
	{
		[TestMethod]
		public void TableTest1()
		{
			int numrows = 100000;
			var table = 
				new Table
				(
					new ColumnDef("ID", typeof(Guid), true),
					new ColumnDef("Name", typeof(string), false),
					new ColumnDef("Age", typeof(int), false),
					new ColumnDef("Comments", typeof(string), false)
				);

			Debug.WriteLine("Inserting Rows...");
			var watch = new Stopwatch();
			for (int j = 0; j < numrows; j++)
			{
				var row = new object[] { Guid.NewGuid(), RandomG.RandomString(RandomG.RandomInt(8)), RandomG.RandomInt(99), RandomG.RandomString(RandomG.RandomInt(16)) };
				watch.Start();
				table.Insert(row);
				watch.Stop();
			}
			table.WaitForWorkers();

			Debug.WriteLine("Inserts Per Second: " + (double)numrows / ((double)watch.ElapsedMilliseconds / 1000));

			Debug.WriteLine("Reconstructing Rows...");

			watch.Reset();
			var selection = table.Select(0, null, null, true, null);
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

	}
}
