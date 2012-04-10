﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics;
using Wrapper;

namespace WrapperTest
{
    class Program
    {
        static void Main(string[] args)
        {
            ManagedBTree b = new ManagedBTree();
            var watch = new Stopwatch();
            int numrows = 1000000;
            watch.Start();
            for (int i = 0; i < numrows; i++)
            {            
                b.Insert(i, i);     
            }
            watch.Stop();

            for (int i = 0; i < 1000; i++)
            {
                Console.WriteLine(b.Get(i));
            }

            var time = (watch.ElapsedMilliseconds / 1000.0);
            Console.WriteLine("BTree Test");
            Console.WriteLine("Total Seconds: " + time);
            Console.WriteLine("Total  Rows: " + numrows );
            Console.WriteLine("Rows per second: " + (numrows / time));

            Dictionary<int, int> test = new Dictionary<int, int>();

            watch.Reset();
            watch.Start();
            for (int i = 0; i < numrows; i++)
            {
                test.Add(i, i);
            }
            watch.Stop();

            Console.WriteLine("Dictionary Test");
            time = (watch.ElapsedMilliseconds / 1000.0);
            Console.WriteLine("Total Seconds: " + time);
            Console.WriteLine("Total  Rows: " + numrows);
            Console.WriteLine("Rows per second: " + (numrows / time));

            SortedList<int, int> list = new SortedList<int, int>();

            watch.Reset();
            watch.Start();
            for (int i = 0; i < numrows; i++)
            {
                list.Add(i, i);
            }
            watch.Stop();

            Console.WriteLine("Sorted List Test");
            time = (watch.ElapsedMilliseconds / 1000.0);
            Console.WriteLine("Total Seconds: " + time);
            Console.WriteLine("Total  Rows: " + numrows);
            Console.WriteLine("Rows per second: " + (numrows / time));
            
            Console.ReadLine();
        }
    }
}
