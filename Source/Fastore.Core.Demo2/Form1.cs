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

using Alphora.Fastore.Client;
using System.Threading.Tasks;
using System.Threading;

namespace Fastore.Core.Demo2
{
	public partial class Form1 : Form
	{
		public Form1()
		{
			InitializeComponent();
		}

        private Database _database;
		private Transaction _transaction;
        private int[] _columns = new int[] { 10000, 10001, 10002, 10003, 10004, 10005 };
        private int[] _schemaColumns = new int[] { 0, 1, 2, 3, 4 };	
        private int _ids = 0;
        private Task _commitTask;
		public bool Canceled { get; set; }

		private void Form1_Shown(object sender, EventArgs e)
		{
			var connect = new ConnectForm();
			if (connect.ShowDialog() != System.Windows.Forms.DialogResult.OK)
			{
				Close();
				return;
			}
			var address = connect.Address;
			var port = connect.Port;
			connect.Dispose();

			_database = Client.Connect(new[] { new ServiceAddress { Name = address, Port = port } });

            if (connect.Detect)
            {
                DetectSchema();
            }
            else
            {
                CreateSchema();
            }

			LoadData();

			StopButton.Visible = false;
			comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 0;
		}

        private void DetectSchema()
        {
            _columns[0] = GetColumnID("ID");
            _columns[1] = GetColumnID("Given");
            _columns[2] = GetColumnID("Surname");
            _columns[3] = GetColumnID("Gender");
            _columns[4] = GetColumnID("BirthDate");
            _columns[5] = GetColumnID("BirthPlace");
        }

        private int GetColumnID(string columnName)
        {
            var nameBound = new RangeBound();
            nameBound.Bound = columnName;
            nameBound.Inclusive = true;

            var nameRange = new Range();
            nameRange.Ascending = true;
            nameRange.ColumnID = 1;
            nameRange.Start = nameBound;
            nameRange.End = nameBound;

            var result = _database.GetRange(new int[] { 1, 0 }, nameRange, 1);

            if (result.Data.Count == 0)
                throw new Exception(String.Format("Column {0} not found in hive", columnName));

            return (int)result.Data[0].Values[1];
        }

		private void CreateSchema()
		{		
			_database.Include(_schemaColumns, _columns[0], new object[] { _columns[0], "ID", "Int", "Int", true });
			_database.Include(_schemaColumns, _columns[1], new object[] { _columns[1], "Given", "String", "Int", false });
			_database.Include(_schemaColumns, _columns[2], new object[] { _columns[2], "Surname", "String", "Int", false });
			_database.Include(_schemaColumns, _columns[3], new object[] { _columns[3], "Gender", "Bool", "Int", false });
			_database.Include(_schemaColumns, _columns[4], new object[] { _columns[4], "BirthDate", "String", "Int", false });
			_database.Include(_schemaColumns, _columns[5], new object[] { _columns[5], "BirthPlace", "String", "Int", false });

			int[] _podIdColumn = new int[] { 300 };
			Range podIdRange = new Range();
			podIdRange.Ascending = true;
			podIdRange.ColumnID = _podIdColumn[0];

			var podIds = _database.GetRange(_podIdColumn, podIdRange, 500);

			int[] _podColumnColumns = new int[] { 400, 401 };
			for (int i = 0; i < _columns.Length; i++)
			{
				_database.Include(_podColumnColumns, i, new object[] { podIds.Data[i % podIds.Data.Count].Values[0], _columns[i] });
			}
		}

		private void LoadData()
		{
			var fileName = @"e:\Ancestry\owt\owt.xml.gz";
			using (var fileStream = new FileStream(fileName, FileMode.Open, FileAccess.Read))
			{
				var deflated = Path.GetExtension(fileName) == ".gz"
					? (Stream)new GZipStream(fileStream, CompressionMode.Decompress)
					: fileStream;

				var xrs = new XmlReaderSettings { ConformanceLevel = ConformanceLevel.Fragment, CheckCharacters = true };
				var xmlReader = XmlReader.Create(deflated, xrs);

				_transaction = _database.Begin(true, true);

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

					if (count % 1000 == 0)
					{

						//Wait until task is done.
						if (_commitTask != null)
							_commitTask.Wait();
						_commitTask = Task.Factory.StartNew
							(
								(t) =>
								{
									((Transaction)t).Commit();
								},
								_transaction
							);
						//_transaction.Commit();
						_transaction = _database.Begin(true, true);
						Application.DoEvents();
					}
					if (count % 1000 == 0)
					{
						StatusBox.AppendText(String.Format("\r\nLoaded: {0}  Last Rate: {1} rows/sec", count, 1000 / ((double)(watch.ElapsedMilliseconds - lastMilliseconds) / 1000)));
						lastMilliseconds = watch.ElapsedMilliseconds;
					}
				}

				//Wait until task is done.
				if (_commitTask != null)
					_commitTask.Wait();

				watch.Stop();

				string result = GetStats();
				StatusBox.AppendText("\r\n" + result);
				StatusBox.AppendText("\r\nRow per second : " + (count / (watch.ElapsedMilliseconds / 1000.0)));
				StatusBox.AppendText("\r\nLoad time: " + watch.Elapsed.ToString());
			}
		}

        private string GetStats()
        {
            string results = "";

            //var stats = _database.GetStatistics(_columns);
            //for (var i = 0; i < stats.Length; i++)
            //    results += "Column: " + _columns[i].ToString() + " Unique: " + stats[i].Unique + " Total: " + stats[i].Total + " Avg Density: " + (double)stats[i].Total / (double)stats[i].Unique + "\n";

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

				_transaction.Include(_columns, _ids, record);
				//_database.Include(_columns, _ids, record);
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
            RangeBound? start = null;
            if (!String.IsNullOrWhiteSpace(Search.Text))
            {
                object value = null;

                switch (comboBox1.SelectedIndex)
                {
                    case 0:
                        bool result = true;
                        bool.TryParse(Search.Text, out result);
                        value = result;
                        break;
                    case 3:
                        int id = 0;
                        int.TryParse(Search.Text, out id);
                        value = id;
                        break;
                    default:
                        value = Search.Text;
                        break;
                }

                start = new RangeBound { Bound = value, Inclusive = true };
            }

			var orderColumn = _columns[comboBox1.SelectedIndex];
            Range range = new Range();
            range.ColumnID = orderColumn;
            range.Ascending = comboBox2.SelectedIndex == 0;
            if (range.Ascending)
                range.Start = start;
            else
                range.End = start;

            var set = 
				_database.GetRange
				(
					_columns,
					range,
					50
				);

            return ParseDataSet(set);
        }

		// Convert each column to a string
		private IEnumerable<string[]> ParseDataSet(Alphora.Fastore.Client.RangeSet set)
		{
			foreach (var item in set.Data)
			{
                yield return (from c in item.Values select (c == null ? "" : c.ToString())).ToArray();
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

        private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
        {
            RefreshItems();
        }


	}
}
