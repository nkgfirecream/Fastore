using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public class Engine
    {
        string[][] fieldValues =
        {
            new string[] { "S1", "Adams", "10", "Athens"},
            new string[] { "S2", "Blake", "20", "London"},
            new string[] { "S3", "Clark", "20", "London"},
            new string[] { "S4", "Jones", "30", "Paris"},
            new string[] { "S5", "Smith", "30", "Paris"}
        };

        int[][] reconstruction = 
        {
            new int[] { 5, 5, 4, 5 },
            new int[] { 4, 4, 2, 1 },
            new int[] { 2, 3, 3, 4 },
            new int[] { 3, 1, 5, 2 },
            new int[] { 1, 2, 1, 3 }
        };


        public void OrderBy(int column)
        {
            string[][] result = new string[5][];
            for (int row = 0; row < 5; row++)
            {
                result[row] = new string[4];

                int currentc = column;
                int currentr = row;
                for (int c = 0; c < 4; c++)
                {
                    int nextcolumn = currentc + 1;
                    if (nextcolumn >= 4)
                        nextcolumn = nextcolumn - 4;

                    if (currentc >= 4)
                        currentc = currentc - 4;

                    int nextrow = reconstruction[currentr][currentc] - 1;
                    result[row][nextcolumn] = fieldValues[nextrow][nextcolumn];
                    currentc++;
                    currentr = nextrow;
                }
            }

            Output(result);
        }


        private void Output(string[][] values)
        {
            foreach (var row in values)
            {
                foreach (var column in row)
                {
                    Console.Write(column + " ");
                }
                Console.WriteLine();
            }
        }
    }
}
