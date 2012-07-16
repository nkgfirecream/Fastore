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
			var addresses = connect.Addresses;
			connect.Dispose();

			_database = Client.Connect(addresses);

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

            listView1.SelectedIndexChanged += listView1_SelectedIndexChanged;
			//listView1.VirtualListSize = (int)_database.GetStatistics(new[] { 10000 })[0].Total;
		}

        void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {
                GetMoreItems();
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
			_database.Include(_schemaColumns, _columns[0], new object[] { _columns[0], "ID", "Int", "Int", BufferType.Identity });
			_database.Include(_schemaColumns, _columns[1], new object[] { _columns[1], "Given", "String", "Int", BufferType.Multi });
            _database.Include(_schemaColumns, _columns[2], new object[] { _columns[2], "Surname", "String", "Int", BufferType.Multi });
            _database.Include(_schemaColumns, _columns[3], new object[] { _columns[3], "Gender", "Bool", "Int", BufferType.Multi });
            _database.Include(_schemaColumns, _columns[4], new object[] { _columns[4], "BirthDate", "String", "Int", BufferType.Multi });
            _database.Include(_schemaColumns, _columns[5], new object[] { _columns[5], "BirthPlace", "String", "Int", BufferType.Multi });

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
			var fileName = @"e:\Ancestry\owt\owt.csv";
			using (var fileStream = new StreamReader(new FileStream(fileName, FileMode.Open, FileAccess.Read)))
			{
				_transaction = _database.Begin(true, true);

				var count = 0;
				long lastMilliseconds = 0;
				Stopwatch watch = new Stopwatch();
				watch.Start();

				string[] record = new string[8];

				while (!Canceled && !fileStream.EndOfStream && count < 95000000)
				{
					var line = fileStream.ReadLine();
					if (String.IsNullOrWhiteSpace(line))
						break;

					count++;

					LineToRecord(line, record);

					InsertRecord(record);

					if (count % 5000 == 0)
					{

						//Wait until task is done.
						if (_commitTask != null)
							_commitTask.Wait();
						_commitTask = Task.Factory.StartNew
							(
								(t) =>
								{
									((Transaction)t).Commit();
									//((Transaction)t).Ping();
								},
								_transaction
							);
						//_transaction.Commit();
						_transaction = _database.Begin(true, true);
						Application.DoEvents();
					}
					if (count % 5000 == 0)
					{
						StatusBox.AppendText(String.Format("\r\nLoaded: {0}  Last Rate: {1} rows/sec", count, 1000 / ((double)(watch.ElapsedMilliseconds - lastMilliseconds) / 5000)));
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

		private void LineToRecord(string line, string[] record)
		{
			var cell = 0;
			var builder = new StringBuilder();
			var i = 0;
			while (i < line.Length)
			{
				var ch = line[i];
				if (ch == '\"')
				{
					i++;
					while (i < line.Length && (ch = line[i]) != '\"')
					{
						if (ch == '\\')
						{
							i++;
							if (i >= line.Length)
								throw new Exception("Invalid escape sequence.");
							ch = line[i];
							switch (ch)
							{
								case 'n': builder.Append('\n'); break;
								default: builder.Append(ch); break;
							}
						}
						else
							builder.Append(ch);
						i++;
					}
					if (ch != '\"')
						throw new Exception("Unterminated quote.");
				}
				else if (ch == ',')
				{
					record[cell] = builder.ToString();
					cell++;
					builder.Clear();
				}
				i++;
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

        private void InsertRecord(string[] record)
        {
            if (record != null && record[0] != null) //Filter out junk data..
            {
				var data = new object[6];
				data[0] = Int32.Parse(record[0]);
                data[1] = record[1] ?? "";
                data[2] = record[2] ?? "";
                data[3] = (record[3] ?? "0") == "1";
                data[4] = record[4] ?? "";
                data[5] = record[5] ?? "";

				_transaction.Include(_columns, data[0], data);
				//_database.Include(_columns, _ids, record);
            }
        }

		private void RefreshItems()
		{
			listView1.Items.Clear();
            GetMoreItems();
			
		}

        private void GetMoreItems()
        {
            object startId = null;
            if (listView1.Items.Count > 0)
            {
                int id;
                string stringid = listView1.Items[listView1.Items.Count - 1].SubItems[0].Text;
                if (int.TryParse(stringid, out id))
                {
                    startId = id;
                }
            }

            foreach (var item in SelectData(startId))
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

        private IEnumerable<string[]> SelectData(object startId = null)
        {
            RangeBound? start = null;
            if (!String.IsNullOrWhiteSpace(Search.Text))
            {
                object value = null;

                switch (comboBox1.SelectedIndex)
                {
                    case 0:
                        int id = 0;
                        int.TryParse(Search.Text, out id);
                        value = id;
                        break;
                    case 3:     
                        bool result = true;
                        bool.TryParse(Search.Text, out result);
                        value = result;
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
            range.Start = startId == null ? start : null;

            var set = 
				_database.GetRange
				(
					_columns,
					range,
					50,
                    startId
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

		private void listView1_SearchForVirtualItem(object sender, SearchForVirtualItemEventArgs e)
		{
			
		}

		private void listView1_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
		{
		}

		private void listView1_CacheVirtualItems(object sender, CacheVirtualItemsEventArgs e)
		{

		}


	}
}
