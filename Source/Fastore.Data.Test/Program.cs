using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Alphora.Fastore.Data.Test
{
	public static class Program
	{
		public static int Main(string[] args)
		{
			var test = new ProviderTests();
			//test.ConnectDisconnectTest();
			test.PrepareCloseTest();
			return 0;
		}
	}
}
