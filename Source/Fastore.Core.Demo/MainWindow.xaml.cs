using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using System.IO.Compression;
using System.Xml;
using System.Diagnostics;

namespace Fastore.Core.Demo
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		public MainWindow()
		{
			InitializeComponent();

			DataContext = this;
		}

		private Table _table;

		private void Window_Loaded(object sender, RoutedEventArgs e)
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
				while (count++ < 100000)//16000000)
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

				RefreshItems();
			}
		}

		private void RefreshItems()
		{
			var items = new List<ItemRecord>();
			foreach (var item in _table.Select(SelectedColumn, String.IsNullOrEmpty(Criteria) ? null : Criteria, null, 25, true, null))
			{
				var record = new ItemRecord();
				record.ID = (int)item.Value[0];
				record.Given = (string)item.Value[1];
				record.Surname = (string)item.Value[2];
				record.IsMale = item.Value[3] == null ? null : (bool?)item.Value[3];
				record.BirthDate = (string)item.Value[4];
				record.BirthPlace = (string)item.Value[5];
				items.Add(record);
			}
			Items = items.ToArray();
		}

		private void CompleteRecord(object[] record)
		{
			if (record != null)
			{
				_table.Insert(record);
			}
		}

		public int SelectedColumn
		{
			get { return (int)GetValue(SelectedColumnProperty); }
			set { SetValue(SelectedColumnProperty, value); }
		}
		public static DependencyProperty SelectedColumnProperty = DependencyProperty.Register("SelectedColumn", typeof(int), typeof(MainWindow), new PropertyMetadata(new PropertyChangedCallback(SelectedColumnChanged)));

		private static void SelectedColumnChanged(DependencyObject o, DependencyPropertyChangedEventArgs a)
		{
			((MainWindow)o).RefreshItems();
		}   
				
		public ItemRecord[] Items
		{
			get { return (ItemRecord[])GetValue(ItemsProperty); }
			set { SetValue(ItemsProperty, value); }
		}
		public static DependencyProperty ItemsProperty = DependencyProperty.Register("Items", typeof(ItemRecord[]), typeof(MainWindow), null);

		
		public string Criteria
		{
			get { return (string)GetValue(CriteriaProperty); }
			set { SetValue(CriteriaProperty, value); }
		}
		public static DependencyProperty CriteriaProperty = DependencyProperty.Register("Criteria", typeof(string), typeof(MainWindow), new PropertyMetadata(new PropertyChangedCallback(CriteriaChanged)));

		private static void CriteriaChanged(DependencyObject o, DependencyPropertyChangedEventArgs a)
		{
			((MainWindow)o).RefreshItems();
		}        
	}

	public class ItemRecord
	{
		public int ID { get; set; }
		public string Given { get; set; }
		public string Surname { get; set; }
		public bool? IsMale { get; set; }
		public string BirthDate { get; set; }
		public string BirthPlace { get; set; }
	}
}
