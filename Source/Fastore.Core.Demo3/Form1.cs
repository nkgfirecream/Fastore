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

using System.Data.SqlClient;

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
        private SqlConnection conn = new SqlConnection("Server=JP-PC\\SQLEXPRESS;Database=Fastore;Trusted_Connection=Yes;");

        private int NumPerTransaction = 5000;
        private int NumRowsToLoad = 1000000;

		public bool Canceled { get; set; }

		private void Form1_Shown(object sender, EventArgs e)
		{
            //Connect to SQl Server
            conn.Open();

            //Connect to Fastore
            var connect = new ConnectForm();
            if (connect.ShowDialog() != System.Windows.Forms.DialogResult.OK)
            {
                Close();
                return;
            }
            var addresses = connect.Addresses;
            connect.Dispose();

            _connection = new Connection(addresses);


            //Create Schemas in Databases
            //TODO: Do Schema detection and skip load if present
            CreateSQLServerSchema();
            CreateSQLServerIndexedSchema();
            CreateFastoreSchema();

            if(!Canceled)
                LoadFastore();

            if (!Canceled)
                LoadSQLServer();

            if (!Canceled)
                LoadSQLServerIndexed();
           


            //
			StopButton.Visible = false;
			comboBox1.SelectedIndex = 0;
            comboBox2.SelectedIndex = 0;
            sourceCombo.SelectedIndex = 0;

            comboBox1.SelectedIndexChanged += comboBox1_SelectedIndexChanged;
            comboBox2.SelectedIndexChanged += comboBox1_SelectedIndexChanged;
            sourceCombo.SelectedIndexChanged += comboBox1_SelectedIndexChanged;

            RefreshItems();
		}

        void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {
            GetMoreItems();
        }

        private bool DetectSchema()
        {
            using (var statement = _connection.Execute("select * from sqlite_master where name = 'Person'"))
                return statement != null && (statement.GetInt64(0) != null);
        }

		private void CreateFastoreSchema()
		{		
			_connection.Execute
			(
				@"create table Person(ID int primary key, Given varchar not null, Surname varchar not null, Gender int not null, BirthDate varchar not null, BirthPlace varchar not null, MID int, FID int)"
			);
		}

        private void CreateSQLServerSchema()
        {
            var dropCmd = conn.CreateCommand();
            dropCmd.CommandText = @"drop table person";
            dropCmd.ExecuteNonQuery();


            var cmd = conn.CreateCommand();
            cmd.CommandText = @"create table person(ID int primary key, Given varchar(100) not null, Surname varchar(100) not null, Gender int not null, BirthDate varchar(100) not null, BirthPlace varchar(100) not null, MID int, FID int)";
            cmd.ExecuteNonQuery();
        }

        private void CreateSQLServerIndexedSchema()
        {
            var dropCmd2 = conn.CreateCommand();
            dropCmd2.CommandText = @"drop table person2";
            dropCmd2.ExecuteNonQuery();

            var cmd2 = conn.CreateCommand();
            cmd2.CommandText = @"create table person2(ID int primary key, Given varchar(100) not null, Surname varchar(100) not null, Gender int not null, BirthDate varchar(100) not null, BirthPlace varchar(100) not null, MID int, FID int)";
            cmd2.ExecuteNonQuery();


            var cmd3 = conn.CreateCommand();
            cmd3.CommandText = @"create INDEX giveIndex on person2 (Given);" +
                                 @"create INDEX surIndex on person2 (Surname);" +
                                  @"create INDEX genIndex on person2 (Gender);" +
                                  @"create INDEX birthIndex on person2 (BirthDate);" +
                                 @"create INDEX placeIndex on person2 (BirthPlace);" +
                                 @"create INDEX midIndex on person2 (MID);" +
                                 @"create INDEX fidIndex on person2 (FID);";
            cmd3.ExecuteNonQuery();
        }


        private IEnumerable<string[]> GetRecords()
        {
            var fileName = @"e:\Ancestry\owt\owt.csv";
            using (var fileStream = new StreamReader(new FileStream(fileName, FileMode.Open, FileAccess.Read)))
            {
                while (!fileStream.EndOfStream)
                {
                    var line = fileStream.ReadLine();
                    if (String.IsNullOrWhiteSpace(line))
                        break;

                    string[] record = new string[8];
                    LineToRecord(line, record);

                    yield return record;
                }
            }
        }

        private void LoadFastore()
        {
            StatusBox.AppendText("\r\nBeginning Load into Fastore");
            int count = 0;        
            long lastMilliseconds = 0;
            Stopwatch watch = new Stopwatch();
            watch.Start();

            _connection.Execute("begin");

            foreach (var record in GetRecords())
            {
                if (Canceled || count > NumRowsToLoad)
                    break;


                //Skip junk data
                if (record != null && record[0] != null)
                {
                    string sql = RecordToSQL(record, "Person");
                    _connection.Execute(sql);
                }
                else
                {
                    continue;
                }

                if (count % NumPerTransaction == 0 && count > 0)
                {
                    _connection.Execute("commit");
                    _connection.Execute("begin");
                    Application.DoEvents();

                    StatusBox.AppendText(String.Format("\r\nFastore Loaded: {0}  Last Rate: {1} rows/sec", count, 1000 / ((double)(watch.ElapsedMilliseconds - lastMilliseconds) / NumPerTransaction)));
                    lastMilliseconds = watch.ElapsedMilliseconds;
                }

                Application.DoEvents();
                count++;
            }

            _connection.Execute("commit");

            watch.Stop();

            LongStatusBox.AppendText("\r\nRows per second into Fastore : " + (count / (watch.ElapsedMilliseconds / 1000.0)));
            LongStatusBox.AppendText("\r\nLoad time into Fastore: " + watch.Elapsed.ToString()); 
        }

        private void LoadSQLServer()
        {
            StatusBox.AppendText("\r\nBeginning Load into SQLServer");
            var cmd = conn.CreateCommand();

            int count = 0;
            long lastMilliseconds = 0;
            Stopwatch watch = new Stopwatch();
            watch.Start();

            cmd.CommandText = "begin tran";
            cmd.ExecuteNonQuery();

            foreach (var record in GetRecords())
            {
                if (Canceled || count > NumRowsToLoad)
                    break;


                //Skip junk data
                if (record != null && record[0] != null)
                {
                    cmd.CommandText = RecordToSQL(record, "person");
                    cmd.ExecuteNonQuery();
                }
                else
                {
                    continue;
                }

                if (count % NumPerTransaction == 0 && count > 0)
                {
                    cmd.CommandText = "commit";
                    cmd.ExecuteNonQuery();
                    cmd.CommandText = "begin tran";
                    cmd.ExecuteNonQuery();
                    Application.DoEvents();

                    StatusBox.AppendText(String.Format("\r\nSQLServer Loaded: {0}  Last Rate: {1} rows/sec", count, 1000 / ((double)(watch.ElapsedMilliseconds - lastMilliseconds) / NumPerTransaction)));
                    lastMilliseconds = watch.ElapsedMilliseconds;
                }

                Application.DoEvents();
                count++;
            }

            cmd.CommandText = "commit";
            cmd.ExecuteNonQuery();

            watch.Stop();

            LongStatusBox.AppendText("\r\nRows per second into SQLServer : " + (count / (watch.ElapsedMilliseconds / 1000.0)));
            LongStatusBox.AppendText("\r\nLoad time into SQLServer: " + watch.Elapsed.ToString());
        }

        private void LoadSQLServerIndexed()
        {
            StatusBox.AppendText("\r\nBeginning Load into SQLServer(Indexed)");
            var cmd = conn.CreateCommand();

            int count = 0;
            long lastMilliseconds = 0;
            Stopwatch watch = new Stopwatch();
            watch.Start();

            cmd.CommandText = "begin tran";
            cmd.ExecuteNonQuery();

            foreach (var record in GetRecords())
            {
                if (Canceled || count > NumRowsToLoad)
                    break;


                //Skip junk data
                if (record != null && record[0] != null)
                {
                    cmd.CommandText = RecordToSQL(record, "person2");
                    cmd.ExecuteNonQuery();
                }
                else
                {
                    continue;
                }

                if (count % NumPerTransaction == 0 && count > 0)
                {
                    cmd.CommandText = "commit";
                    cmd.ExecuteNonQuery();
                    cmd.CommandText = "begin tran";
                    cmd.ExecuteNonQuery();
                    Application.DoEvents();

                    StatusBox.AppendText(String.Format("\r\nSQLServer(Indexed) Loaded: {0}  Last Rate: {1} rows/sec", count, 1000 / ((double)(watch.ElapsedMilliseconds - lastMilliseconds) / NumPerTransaction)));
                    lastMilliseconds = watch.ElapsedMilliseconds;
                }

                Application.DoEvents();
                count++;
            }

            cmd.CommandText = "commit";
            cmd.ExecuteNonQuery();

            watch.Stop();

            LongStatusBox.AppendText("\r\nRows per second into SQLServer(Indexed) : " + (count / (watch.ElapsedMilliseconds / 1000.0)));
            LongStatusBox.AppendText("\r\nLoad time into SQLServer(Indexed): " + watch.Elapsed.ToString());
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

        private string RecordToSQL(string[] record, string tableName)
        {
			var data = new object[8];
			data[0] = Int32.Parse(record[0]);
            data[1] = Escape(record[1] ?? "");
            data[2] = Escape(record[2] ?? "");
            data[3] = String.IsNullOrEmpty(record[3]) ? 0 : Int32.Parse(record[3]);
            data[4] = Escape(record[4] ?? "");
            data[5] = Escape(record[5] ?? "");
			data[6] = Escape(String.IsNullOrEmpty(record[6]) ? "NULL" : record[6]);
            data[7] = Escape(String.IsNullOrEmpty(record[7]) ? "NULL" : record[7]);

            return String.Format("insert into " + tableName + " (ID, Given, Surname, Gender, BirthDate, BirthPlace, MID, FID) values ({0}, '{1}', '{2}', {3}, '{4}', '{5}', {6}, {7})", data);
        }

		private string Escape(string value)
		{
            var escaped = value.Replace("'", "''");
			return escaped.Substring(0, Math.Min(escaped.Length,100));
		}

        private List<string[]> QueryFastore(string query)
        {
            Stopwatch watch = new Stopwatch();
            watch.Start();
            var statement = _connection.Prepare(query);

            //StatusBox.AppendText("\r\nFastore Query Time: " + watch.Elapsed.ToString());

            List<string[]> list = new List<string[]>();
            foreach (var item in ParseDataSet(statement))
            {
                list.Add(item);
            }

            watch.Stop();

            StatusBox.AppendText("\r\nFastore Total Time: " + watch.Elapsed.ToString());

            return list;

        }

        private List<string[]> QuerySQLServer(string query)
        {
            var cmd = conn.CreateCommand();
            cmd.CommandText = query;

            Stopwatch watch = new Stopwatch();
            watch.Start();

            var reader = cmd.ExecuteReader();

            //StatusBox.AppendText("\r\nSQLServer Query Time: " + watch.Elapsed.ToString());

            List<string[]> list = new List<string[]>();
            foreach (var item in ParseSQLResult(reader))
            {
                list.Add(item);
            }

            watch.Stop();

            StatusBox.AppendText("\r\nSQLServer Total Time: " + watch.Elapsed.ToString());

            reader.Close();

            return list;
        }


		private void RefreshItems()
		{
			listView1.Items.Clear();
            GetMoreItems();
			
		}

        private void GetMoreItems()
        {
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

        private List<string[]> SelectData()
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

            var query = "ID, Given, Surname, Gender, BirthDate, BirthPlace, MID, FID from {0} " 
				+ condition + (String.IsNullOrEmpty(orderBy) ? "" : " order by " + orderBy);

            switch (sourceCombo.SelectedIndex)
            {
                case 0:
                    return QueryFastore("select " + query.Replace("{0}", "Person") + " limit 100");
                case 1:
                    return QuerySQLServer("select top 100 " + query.Replace("{0}", "person"));
                case 2:
                    return QuerySQLServer("select top 100 " + query.Replace("{0}", "person2"));
                default:
                    var fastore = QueryFastore("select " + query.Replace("{0}", "Person") + " limit 100");
                    var sql = QuerySQLServer("select top 100 " + query.Replace("{0}", "person"));
                    var sql2 = QuerySQLServer("select top 100 " + query.Replace("{0}", "person2"));

                    return fastore;
            }
        }

		// Convert each column to a string
		private IEnumerable<string[]> ParseDataSet(Statement set)
		{
			for (var i = 0; i < 100 && set.Next(); i++)
			{
				string[] result = new string[set.ColumnCount];
				for (int c = 0; c < set.ColumnCount; c++)
					result[c] = set.GetAString(c);
				yield return result;
			}
		}

        private IEnumerable<string[]> ParseSQLResult(SqlDataReader reader)
        {

            for (var i = 0; i < 100 && reader.Read(); i++)
            {
                string[] result = new string[reader.FieldCount];
                for (int c = 0; c < reader.FieldCount; c++)
                {
                    result[c] = Convert.ToString(reader.GetValue(c));
                }
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
