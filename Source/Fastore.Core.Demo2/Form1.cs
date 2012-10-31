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

using Alphora.Fastore.Data;
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

        private Connection _connection;
        private Task _commitTask;
        private int NumPerTransaction = 5000;
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

			_connection = new Connection(addresses);

			if (!DetectSchema())
                CreateSchema();

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

        private bool DetectSchema()
        {
			using (var statement = _connection.Prepare("select * from sqlite_master where name = 'Person'"))
				return statement.Next();
        }

		private void CreateSchema()
		{		
			_connection.Execute
			(
				@"create table Person(ID int primary key, Given varchar not null, Surname varchar not null, Gender int not null, BirthDate varchar not null, BirthPlace varchar not null, MID int, FID int)"
			);
		}

		private void LoadData()
		{
			var fileName = @"e:\Ancestry\owt\owt.csv";
			using (var fileStream = new StreamReader(new FileStream(fileName, FileMode.Open, FileAccess.Read)))
			{
				_connection.Execute("begin");

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

                    if (count % NumPerTransaction == 0)
					{

						//Wait until task is done.
                        //if (_commitTask != null)
                        //    _commitTask.Wait();

                        //_commitTask = Task.Factory.StartNew
                        //    (
                        //        (c) =>
                        //        {
                        //            ((Connection)c).Execute("commit");
                        //            //((Transaction)t).Ping();
                        //        },
                        //        _connection
                        //    );
						//_transaction.Commit();
                        _connection.Execute("commit");
						_connection.Execute("begin");
						Application.DoEvents();
					}
                    if (count % NumPerTransaction == 0)
					{
                        StatusBox.AppendText(String.Format("\r\nLoaded: {0}  Last Rate: {1} rows/sec", count, 1000 / ((double)(watch.ElapsedMilliseconds - lastMilliseconds) / NumPerTransaction)));
						lastMilliseconds = watch.ElapsedMilliseconds;
					}
				}

				//Wait until task is done.
				//if (_commitTask != null)
                //    _commitTask.Wait();

				_connection.Execute("commit;");

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

		private Statement _insertStatement;

        private void InsertRecord(string[] record)
        {
            if (record != null && record[0] != null) //Filter out junk data..
            {
				if (_insertStatement == null)
					_insertStatement = _connection.Prepare("insert into Person (ID, Given, Surname, Gender, BirthDate, BirthPlace, MID, FID) values (?, ?, ?, ?, ?, ?, ?, ?)");
				for (int i = 0; i < record.Length; i++)
					if (record[i] != null)
						_insertStatement.BindAString(i + 1, record[i]);
				_insertStatement.Next();
				//_database.Include(_columns, _ids, record);
            }
        }

		private string Escape(string value)
		{
			return value.Replace("'", "''");
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
            string orderBy = comboBox1.SelectedItem.ToString() + (comboBox2.SelectedIndex == 0 ? " asc" : " desc");
			string condition = "";
            if (!String.IsNullOrWhiteSpace(Search.Text))
            {
				//object value = null;

				//switch (comboBox1.SelectedIndex)
				//{
				//	case 0:
				//		int id = 0;
				//		int.TryParse(Search.Text, out id);
				//		value = id;
				//		break;
				//	case 3:     
				//		bool result = true;
				//		bool.TryParse(Search.Text, out result);
				//		value = result;
				//		break;
				//	default:
                //      value = Search.Text;
				//		break;
				//}

                //start = new RangeBound { Bound = value, Inclusive = true };

				if (!String.IsNullOrWhiteSpace(Search.Text))
                    condition = "where " + comboBox1.SelectedItem.ToString() + (comboBox2.SelectedIndex == 0 ? " >= '" : " <= '") + Search.Text + "'";
            }

			var query = "select ID, Given, Surname, Gender, BirthDate, BirthPlace, MID, FID from Person " 
				+ condition + (String.IsNullOrEmpty(orderBy) ? "" : " order by " + orderBy);
            var statement = _connection.Prepare(query);
	        return ParseDataSet(statement);
        }

		// Convert each column to a string
		private IEnumerable<string[]> ParseDataSet(Statement set)
		{
			for (var i = 0; i < 35 && set.Next(); i++)
			{
				string[] result = new string[set.ColumnCount];
				for (int c = 0; c < set.ColumnCount; c++)
					result[c] = set.GetAString(c);
				yield return result;
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
