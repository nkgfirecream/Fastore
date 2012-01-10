using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Fastore.Core
{
    //surrogate key object. Since our links are just references, this works fine.
    class KeyObject {}

    public class RowList
       // : ILeafSubscriber<string,KeyObject>
        : ILeafSubscriber<string, Guid>
    {
        //private Dictionary<KeyObject, Leaf<string,KeyObject>[]> rows = new Dictionary<KeyObject,Leaf<string,KeyObject>[]>();
       // private List<BTree<string, KeyObject>> columns = new List<BTree<string, KeyObject>>();

        private Dictionary<Guid, Leaf<string, Guid>[]> rows = new Dictionary<Guid, Leaf<string, Guid>[]>();
        private List<BTree<string, Guid>> columns = new List<BTree<string, Guid>>();


        private int _numcolumns;
        public RowList(int numcolumns)
        {
            _numcolumns = numcolumns;
            for (int i = 0; i < numcolumns; i++)
            {
                columns.Add(new BTree<string, Guid>(32, 16, this, i));
            }
        }

        //public void Add(params string[] values)
        public void Add(Guid key, params string[] values)
        {
            if (values.Length != _numcolumns)
                throw new ArgumentException("Hey! We need the same number of values as columns!");

           // var list = new Leaf<string, KeyObject>[_numcolumns];

            var list = new Leaf<string, Guid>[_numcolumns];

            //var key = new KeyObject();
            //rows.Add(key, list);
            rows.Add(key, list);
            for (int i = 0; i < _numcolumns; i++)
            {
                //Leaf<string, KeyObject> leaf;
                Leaf<string, Guid> leaf;
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

        //private string ReconstructRow(KeyObject row)
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

        //public void UpdateLink(KeyObject row, int column, Leaf<string, KeyObject> leaf)
        public void UpdateLink(Guid row, int column, Leaf<string, Guid> leaf)
        {
            rows[row][column] = leaf;
        }
    }
}
