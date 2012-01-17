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

	}
}
