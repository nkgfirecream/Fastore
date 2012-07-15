using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Text;
using System.Xml;

namespace Fastore.Core.Demo2
{
	class OWTXMLToCSV
	{
		public bool Canceled { get; set; }

		public void Convert()
		{
			var fileName = @"e:\Ancestry\owt\owt.xml.gz";
			using (var sourceStream = new FileStream(fileName, FileMode.Open, FileAccess.Read))
			{
				var deflated = Path.GetExtension(fileName) == ".gz"
					? (Stream)new GZipStream(sourceStream, CompressionMode.Decompress)
					: sourceStream;

				var xrs = new XmlReaderSettings { ConformanceLevel = ConformanceLevel.Fragment, CheckCharacters = true };
				var xmlReader = XmlReader.Create(deflated, xrs);

				using (var targetWriter = new StreamWriter(new FileStream(@"e:\Ancestry\owt\owt.csv", FileMode.Create, FileAccess.Write)))
				{
					var count = 0;
					long lastMilliseconds = 0;
					Stopwatch watch = new Stopwatch();
					watch.Start();
					while (!Canceled && count < 85000000)
					{
						xmlReader.MoveToContent();
						if (xmlReader.EOF)
							break;

						count++;

						var subReader = xmlReader.ReadSubtree();
						string[] record = null;
						while (subReader.Read())
						{
							if (subReader.NodeType == XmlNodeType.Element)
							{
								if (subReader.Name == "d")
								{
									InsertRecord(record, targetWriter);

									record = new string[8];

									if (subReader.MoveToAttribute("p"))
										record[0] = subReader.Value;
								}
								else if (subReader.Name == "f" && subReader.MoveToAttribute("i"))
								{
									var code = subReader.Value;
									subReader.MoveToContent();
									switch (code)
									{
										case "80004002": record[1] = subReader.ReadString(); break;
										case "80004003": record[2] = subReader.ReadString(); break;
										case "83004003": record[3] = subReader.ReadString().StartsWith("M", StringComparison.OrdinalIgnoreCase) ? "1" : "0"; break;
										case "81004010": record[4] = subReader.ReadString(); break;
										case "82004010": record[5] = subReader.ReadString(); break;
										case "83008004": record[6] = subReader.ReadString(); break;
										case "8300C004": record[7] = subReader.ReadString(); break;
									}
								}
							}
						}

						InsertRecord(record, targetWriter);

						xmlReader.Read();

						if (count % 1000 == 0)
						{
							System.Windows.Forms.Application.DoEvents();
						}
						if (count % 1000 == 0)
						{
							//StatusBox.AppendText(String.Format("\r\nLoaded: {0}  Last Rate: {1} rows/sec", count, 1000 / ((double)(watch.ElapsedMilliseconds - lastMilliseconds) / 1000)));
							lastMilliseconds = watch.ElapsedMilliseconds;
						}
					}

					watch.Stop();

					Debug.WriteLine("\r\nRow per second : " + (count / (watch.ElapsedMilliseconds / 1000.0)));
					Debug.WriteLine("\r\nLoad time: " + watch.Elapsed.ToString());
				}
			}
		}

		private void InsertRecord(string[] record, StreamWriter targetWriter)
		{
			if (record != null)
			{
				for (var i = 0; i < record.Length; i++)
				{
					if (i > 0)
						targetWriter.Write(",");
					targetWriter.Write("\"" + (record[i] ?? "").Replace("\\", "\\\\").Replace("\n", "\\n").Replace("\"", "\\\"") + "\"");
				}
				targetWriter.Write("\r\n");
			}
		}
	}
}
