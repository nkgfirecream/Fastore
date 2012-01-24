using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Fastore.Core.Test;
using Microsoft.VisualBasic.FileIO;
using System.IO;

namespace Fastore.Core.TestApp
{
    class Program
    {
        static void Main(string[] args)
        {

            //var but = new ColumnHash<string>(Comparer<string>.Default);


            //TextFieldParser parser = new TextFieldParser(@"C:\owt.person.csv");
            //parser.HasFieldsEnclosedInQuotes = false;
            //parser.Delimiters = new string[] { "," };

            //var test = new KeyBTree<string>(Comparer<string>.Default);

            //IKeyLeaf<string> dummy;

            //while (!parser.EndOfData)
            //{
            //    test.Insert(parser.ReadFields()[1], out dummy);
            //}

            //var file = new StreamWriter(@"c:\names.txt");

            //foreach (var item in test.Get(true))
            //{
            //    file.WriteLine(item);
            //}

            //file.Flush();
            //file.Close();

            var file = new StreamReader(@"c:\names.txt");

            var test = new SimplePrefixBTree<string>();

            IBTreeLeaf<string,string> dummy;
            while (!file.EndOfStream)
            {
                var item = file.ReadLine();
                test.Insert(item, item, out dummy);
            }

            file.Close();
            file.Dispose();
            //var outfile = new StreamWriter(@"c:\patriciatree.txt");

            //outfile.WriteLine(test.ToString());
            Console.ReadLine();
        }
    }
}
