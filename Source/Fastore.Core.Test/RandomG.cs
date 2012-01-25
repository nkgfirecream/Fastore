using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core.Test
{
    public static class RandomG
    {
        private static readonly Random _rng = new Random();
        private static readonly string _chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        public static string RandomString(int size)
        {
            char[] buffer = new char[size];

            for (int i = 0; i < size; i++)
            {
                buffer[i] = _chars[_rng.Next(_chars.Length)];
            }
            return new string(buffer);
        }

        public static int RandomInt(int max)
        {
            return _rng.Next(1, max);
        }

		public static long RandomLong()
		{
			return (long)_rng.Next(int.MinValue, int.MaxValue) << 32 & (long)_rng.Next(int.MinValue, int.MaxValue);
		}
	}
}
