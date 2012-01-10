using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    public class RowList
        : ILeafSubscriber<string, Guid>
    {
        private Dictionary<Guid, ILeaf<string, Guid>[]> rows = new Dictionary<Guid, ILeaf<string, Guid>[]>();
        private List<IKeyValueTree<string, Guid>> columns = new List<IKeyValueTree<string, Guid>>();


        private int _numcolumns;
        public RowList(int numcolumns)
        {
            _numcolumns = numcolumns;
            for (int i = 0; i < numcolumns; i++)
            {
                columns.Add(new BTree<string, Guid>(32, 16, this, i));
            }
        }

        public void Add(Guid key, params string[] values)
        {
            if (values.Length != _numcolumns)
                throw new ArgumentException("Hey! We need the same number of values as columns!");

            var list = new ILeaf<string, Guid>[_numcolumns];

            rows.Add(key, list);
            for (int i = 0; i < _numcolumns; i++)
            {
                ILeaf<string, Guid> leaf;
                columns[i].Insert(values[i], key, out leaf);
                list[i] = leaf;
            }            
        }

        public IEnumerable<string> OrderBy(int column)
        {
            foreach (var rowID in columns[column].OrderedValues())
            {
                yield return ReconstructRow(rowID);
            }
        }

        private string ReconstructRow(Guid row)
        {
            var list = rows[row];

            StringBuilder builder = new StringBuilder();
            for (int i = 0; i < list.Length; i++)
            {
                builder.Append(list[i].GetKey(row));
                builder.Append("\t");
            }

            return builder.ToString();
        }

        public void UpdateLink(Guid row, int column, ILeaf<string, Guid> leaf)
        {
            rows[row][column] = leaf;
        }
    }
}
