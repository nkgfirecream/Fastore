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

namespace Fastore.Core.Demo2
{
	public partial class Form1 : Form
	{
		public Form1()
		{
			InitializeComponent();
		}

		private Table _table;

		private void Form1_Load(object sender, EventArgs e)
		{
			_table =
				new Table
				(
					new ColumnDef("ID", typeof(int), true),
					new ColumnDef("Given", typeof(string), false),
					new ColumnDef("Surname", typeof(string), false),
					new ColumnDef("Gender", typeof(bool), false),
					new ColumnDef("BirthDate", typeof(string), false),
					new ColumnDef("BirthPlace", typeof(string), false)
				);

			using (var fileStream = new FileStream(@"E:\owt.xml.gz", FileMode.Open, FileAccess.Read))
			{
				var deflateStream = new GZipStream(fileStream, CompressionMode.Decompress);
				var streamReader = new StreamReader(deflateStream);

				// output sample
				//var output = new StringWriter();
				//for (var i = 0; i < 150; i++)
				//{
				//    output.WriteLine(streamReader.ReadLine());
				//}
				//System.Diagnostics.Debug.WriteLine(output.ToString());

				var xrs = new XmlReaderSettings { ConformanceLevel = ConformanceLevel.Fragment };
				var xmlReader = XmlReader.Create(deflateStream, xrs);

				var count = 0;
				Stopwatch timer = new Stopwatch();
				timer.Start();
				while (count++ < 1000000)//16000000)
				{
					xmlReader.MoveToContent();
					if (xmlReader.EOF)
						break;

					var subReader = xmlReader.ReadSubtree();
					object[] record = null;
					while (subReader.Read())
					{
						if (subReader.NodeType == XmlNodeType.Element)
						{
							if (subReader.Name == "d")
							{
								CompleteRecord(record);
								record = new object[_table.ColumnCount];

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
					CompleteRecord(record);

					xmlReader.Read();
				}
				timer.Stop();

				System.Diagnostics.Debug.WriteLine("Load time: " + timer.Elapsed.ToString());

				comboBox1.SelectedIndex = 0;
			}
		}

		private void RefreshItems()
		{
			listView1.Items.Clear();
			foreach (var item in _table.Select(comboBox1.SelectedIndex, String.IsNullOrEmpty(textBox1.Text) ? null : textBox1.Text, null, 35, true, null))
			{
				listView1.Items.Add
				(
					new ListViewItem
					(
						new string[]
						{
							((int)item.Value[0]).ToString(),
							(string)item.Value[1],
							(string)item.Value[2],
							item.Value[3] == null ? "-" : ((bool?)item.Value[3]).ToString(),
							(string)item.Value[4],
							(string)item.Value[5]
						}
					)
				);
			}
		}

		private void CompleteRecord(object[] record)
		{
			if (record != null)
			{
				_table.Insert(record);
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


	}
}
