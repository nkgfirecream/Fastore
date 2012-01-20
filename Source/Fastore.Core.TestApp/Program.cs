using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Fastore.Core.Test;

namespace Fastore.Core.TestApp
{
    class Program
    {
        static void Main(string[] args)
        {
            var test = new MemoryUsageTests();

            test.TestMethod2();
        }
    }
}
