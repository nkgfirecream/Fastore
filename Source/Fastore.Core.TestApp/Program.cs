using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Fastore.Core.Test;
using Microsoft.VisualBasic.FileIO;
using System.IO;
using System.Diagnostics;

namespace Fastore.Core.TestApp
{
    class Program
    {
        static void Main(string[] args)
        {

            var test = new BTree<int, int>();
            var watch = new Stopwatch();
            int numrows = 1000000;
            watch.Start();
            IKeyValueLeaf<int, int> dummy;
            for (int i = 0; i < numrows; i++)
            {
                test.Insert(i, i, out dummy);
            }
            watch.Stop();

            var time = (watch.ElapsedMilliseconds / 1000.0);
            Console.WriteLine("BTree Test");
            Console.WriteLine("Total Seconds: " + time);
            Console.WriteLine("Total  Rows: " + numrows);
            Console.WriteLine("Rows per second: " + (numrows / time));


            Console.ReadLine();
        }
    }
}
