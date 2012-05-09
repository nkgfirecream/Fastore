using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.IO.Compression;
using System.Xml;
using System.Diagnostics;
using Microsoft.VisualBasic.FileIO;
using Wrapper;

namespace Fastore.Core.Demo2
{
	public partial class Form1 : Form
	{
		public Form1()
		{
			InitializeComponent();
		}

        private ManagedSession _session;
        private int[] _columns;
        private int _ids = 0;
		public bool Canceled { get; set; }

		private void Form1_Shown(object sender, EventArgs e)
		{
           // Microsoft.VisualBasic.FileIO.TextFieldParser parser = new Microsoft.VisualBasic.FileIO.TextFieldParser(@"C:\owt.txt");
           // parser.Delimiters = new string[] { "^" };            

            ManagedColumnDef c1 = new ManagedColumnDef();
            c1.IsUnique = true;
            c1.ValueType = "Int";
            c1.RowIDType = "Int";
            c1.Name = "ID";
            c1.ColumnID = 0;

            ManagedColumnDef c2 = new ManagedColumnDef();
            c2.IsUnique = false;
            c2.ValueType = "String";
            c2.RowIDType = "Int";
            c2.Name = "Given";
            c2.ColumnID = 1;

            ManagedColumnDef c3 = new ManagedColumnDef();
            c3.IsUnique = false;
            c3.ValueType = "String";
            c3.RowIDType = "Int";
            c3.Name = "Surname";
            c3.ColumnID = 2;

            ManagedColumnDef c4 = new ManagedColumnDef();
            c4.IsUnique = false;
            c4.ValueType = "Bool";
            c4.RowIDType = "Int";
            c4.Name = "Gender";
            c4.ColumnID = 3;

            ManagedColumnDef c5 = new ManagedColumnDef();
            c5.IsUnique = false;
            c5.ValueType = "String";
            c5.RowIDType = "Int";
            c5.Name = "BirthDate";
            c5.ColumnID = 4;

            ManagedColumnDef c6 = new ManagedColumnDef();
            c6.IsUnique = false;
            c6.ValueType = "String";
            c6.RowIDType = "Int";
            c6.Name = "BirthPlace";
            c6.ColumnID = 5;

            ManagedTopology topo = new ManagedTopology();

            topo.Add(c1);
            topo.Add(c2);
            topo.Add(c3);
            topo.Add(c4);
            topo.Add(c5);
            topo.Add(c6);

            ManagedHostFactory hf = new ManagedHostFactory();
            var host = hf.Create(topo);

            var db = new ManagedDatabase(host);

            _session = db.Start();
            _columns = new int[] {0, 1, 2, 3, 4, 5};


			var fileName = @"e:\owt.xml";
			using (var fileStream = new FileStream(fileName, FileMode.Open, FileAccess.Read))
            {
				var deflated = Path.GetExtension(fileName) == ".gz" 
					? (Stream)new GZipStream(fileStream, CompressionMode.Decompress)
					: fileStream;

                var xrs = new XmlReaderSettings { ConformanceLevel = ConformanceLevel.Fragment, CheckCharacters = true };
                var xmlReader = XmlReader.Create(deflated, xrs);

               
                var count = 0;
				long lastMilliseconds = 0;
                Stopwatch watch = new Stopwatch();
                watch.Start();
                while (!Canceled)
                { 
                    xmlReader.MoveToContent();
                    if (xmlReader.EOF)
                        break;

					count++;

                    var subReader = xmlReader.ReadSubtree();
                    object[] record = null;
                    while (subReader.Read())
                    {
                        if (subReader.NodeType == XmlNodeType.Element)
                        {
                            if (subReader.Name == "d")
                            {
                                InsertRecord(record);

                                record = new object[_columns.Length];

                                if (subReader.MoveToAttribute("p"))
                                    record[0] = int.Parse(subReader.Value);
                            }
                            else if (subReader.Name == "f" && subReader.MoveToAttribute("i"))
                            {
                                var code = subReader.Value;
                                subReader.MoveToContent();
                                switch (code)
                                {
                                    case "80004002": record[1] = subReader.ReadString(); break;
                                    case "80004003": record[2] = subReader.ReadString(); break;
                                    case "83004003": record[3] = subReader.ReadString().StartsWith("M", StringComparison.OrdinalIgnoreCase); break;
                                    case "81004010": record[4] = subReader.ReadString(); break;
                                    case "82004010": record[5] = subReader.ReadString(); break;
                                }
                            }
                        }
					}

                    InsertRecord(record);

                    xmlReader.Read();

					if (count % 100000 == 0)
					{
						StatusBox.AppendText(String.Format("\r\nLoaded: {0}  Last Rate: {1} rows/sec", count, 100000 / ((double)(watch.ElapsedMilliseconds - lastMilliseconds) / 1000)));
						lastMilliseconds = watch.ElapsedMilliseconds;
					}
					if (count % 1000 == 0)
						Application.DoEvents();
				}
                watch.Stop();

				StatusBox.AppendText("\r\nRow per second : " + (count / (watch.ElapsedMilliseconds / 1000.0)));

                string result = GetStats();
				StatusBox.AppendText("\r\n" + result);

				StatusBox.AppendText("\r\nLoad time: " + watch.Elapsed.ToString());

				StopButton.Visible = false;
            }

            //Stopwatch watch = new Stopwatch();
            //Stopwatch watchInner = new Stopwatch();
            //watch.Start();
            //int numrows = 10000;
            //for (int i = 0; i < numrows; i++)
            //{
            //    var strings = parser.ReadFields();
            //    object[] objects = new object[6];

            //    int id = int.Parse(strings[0]);
            //    bool gender = strings[3].StartsWith("T");

            //    objects[0] = id;
            //    objects[3] = gender;
            //    objects[1] = strings[1];
            //    objects[2] = strings[2];
            //    objects[4] = strings[4];
            //    objects[5] = strings[5];

            //    watchInner.Start();
            //    _session.Include(objects, _columns, false);
            //    watchInner.Stop();
            //}
            //watch.Stop();

            ////Stopwatch is not accurate...
            
        

            comboBox1.SelectedIndex = 0;
		}

        private string GetStats()
        {
            string results = "";
            foreach (var item in _columns)
            {
                var stats = _session.GetStatistics(item);
                results += "Column: " + item + " Unique: " + stats.Unique() + " Total: " + stats.Total() + " Avg Density: " + (double)stats.Total() / (double)stats.Unique() + "\n";
            }

            return results;
        }

        private void InsertRecord(object[] record)
        {
            if (record != null && record[0] != null) //Filter out junk data..
            {
                record[1] = record[1] ?? "";
                record[2] = record[2] ?? "";
                record[3] = record[3] ?? false;
                record[4] = record[4] ?? "";
                record[5] = record[5] ?? "";

                _session.Include(_ids, record, _columns);
                _ids++;
            }
        }

		private void RefreshItems()
		{
			listView1.Items.Clear();
			foreach (var item in SelectData())
			{
				listView1.Items.Add
				(
					new ListViewItem
					(
						item
					)
				);
			}
		}

        private IEnumerable<string[]> SelectData()
        {
            List<ManagedRange> ranges = new List<ManagedRange>();

            ManagedRangeBound start = null;
            if (!String.IsNullOrWhiteSpace(Search.Text))
            {
                start = new ManagedRangeBound(Search.Text, null, true);
            }
         
            ranges.Add(new ManagedRange(comboBox1.SelectedIndex, 100, start, null));

            List<ManagedOrder> orders = new List<ManagedOrder>();

            //foreach (var item in _columns)
            //{
            //    if (item == comboBox1.SelectedItem.ToString())
            //    {
            //        //Force the selected Item into the range as the first item
            //        //So that the data is ordered by it/
            //        ranges.Insert(0, CreateRange(item, true));
            //    }
            //    else
            //    {
            //        var range = CreateRange(item, false);
            //        if (range != null)
            //            ranges.Add(range);
            //    }
            //}
            
            var set = _session.GetRange(_columns, orders.ToArray(), ranges.ToArray());

            return ParseDataSet(set);
        }

        //Force just means create a range even even we aren't searching on it. There's a better way to do this I'm sure,
        //But time crunch time.
        //private ManagedRange CreateRange(string item, bool force)
        //{
        //    //How do you search for controls by name on winforms...
        //    ManagedRange range = null;
        //    int limit = 100;
        //    switch (item)
        //    {
        //        case "ID":
        //            if (force || !String.IsNullOrEmpty(IDSearch.Text))
        //            {
        //                int id = 0;
        //                int.TryParse(IDSearch.Text, out id);

        //                ManagedRangeBound start = new ManagedRangeBound(id, null, true);
        //                range = new ManagedRange("ID", limit, true, start, null);
        //            }
        //            break;
        //        case "Given":
        //            if (force || !String.IsNullOrEmpty(GivenSearch.Text))
        //            {
        //                ManagedRangeBound start = new ManagedRangeBound(GivenSearch.Text, null, true);
        //                range = new ManagedRange("Given", limit, true, start, null);
        //            }
        //            break;
        //        case "Surname":
        //            if (force || !String.IsNullOrEmpty(SurnameSearch.Text))
        //            {
        //                ManagedRangeBound start = new ManagedRangeBound(SurnameSearch.Text, null, true);
        //                range = new ManagedRange("Surname", limit, true, start, null);
        //            }
        //            break;
        //        case "Gender" :
        //            if (force || !String.IsNullOrEmpty(GenderSearch.Text))
        //            {
        //                bool gender;
        //                bool.TryParse(GenderSearch.Text, out gender);
        //                ManagedRangeBound start = new ManagedRangeBound(gender, null, true);
        //                range = new ManagedRange("Gender", limit, true, start, null);
        //            }
        //            break;
        //        case "BirthPlace" :
        //            if (force || !String.IsNullOrEmpty(BirthPlaceSearch.Text))
        //            {
        //                ManagedRangeBound start = new ManagedRangeBound(BirthPlaceSearch.Text, null, true);
        //                range = new ManagedRange("BirthPlace", limit, true, start, null);
        //            }
        //            break;
        //        case "BirthDate" :
        //            if (force || !String.IsNullOrEmpty(BirthDateSearch.Text))
        //            {
        //                ManagedRangeBound start = new ManagedRangeBound(BirthDateSearch.Text, null, true);
        //                range = new ManagedRange("BirthDate", limit, true, start, null);
        //            }
        //            break;
        //        default: break;
        //    }

        //    return range;
        //}

		private IEnumerable<string[]> ParseDataSet(ManagedDataSet set)
		{
			for (int i = 0; i < set.Size(); i++)
			{
                yield return (from c in set.Row(i) select c.ToString()).ToArray();
			}
		}

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
		{
			RefreshItems();
		}

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            RefreshItems();
        }

		private void StopButton_Click(object sender, EventArgs e)
		{
			Canceled = true;
		}


	}
}
