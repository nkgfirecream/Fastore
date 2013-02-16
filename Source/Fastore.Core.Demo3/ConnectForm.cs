using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Alphora.Fastore.Data;

namespace Fastore.Core.Demo2
{
	public partial class ConnectForm : Form
	{
		public ConnectForm()
		{
			InitializeComponent();
		}

		public Provider.FastoreAddress[] Addresses
		{
			get
			{
				var results = new List<Provider.FastoreAddress>();
				if (!String.IsNullOrEmpty(AddressBox.Text))
					results.Add(new Provider.FastoreAddress { HostName = AddressBox.Text, Port = (ulong)PortBox.Value });
				foreach (ListViewItem item in this.list.Items)
					results.Add(new Provider.FastoreAddress { HostName = item.SubItems[0].Text, Port = UInt64.Parse(item.SubItems[1].Text) });
				return results.ToArray();
			}
		}

		private void button1_Click(object sender, EventArgs e)
		{
			if (!String.IsNullOrEmpty(AddressBox.Text))
			{
				this.list.Items.Add(new ListViewItem(new string[] { AddressBox.Text, PortBox.Value.ToString() }));
				AddressBox.Text = "";
				PortBox.Value = 8765;
			}
		}

		private void button2_Click(object sender, EventArgs e)
		{
			if (this.list.SelectedIndices.Count > 0)
				this.list.Items.RemoveAt(this.list.SelectedIndices[0]);
		}
	}
}
