using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Alphora.Fastore.Client;

namespace Fastore.Core.Demo2
{
	public partial class ConnectForm : Form
	{
		public ConnectForm()
		{
			InitializeComponent();
		}

		public ServiceAddress[] Addresses
		{
			get
			{
				var results = new List<ServiceAddress>();
				if (!String.IsNullOrEmpty(AddressBox.Text))
					results.Add(new ServiceAddress { Name = AddressBox.Text, Port = (int)PortBox.Value });
				foreach (ListViewItem item in this.list.Items)
					results.Add(new ServiceAddress { Name = item.SubItems[0].Text, Port = Int32.Parse(item.SubItems[1].Text) });
				return results.ToArray();
			}
		}

        public bool Detect
        {
            get { return DetectCheck.Checked; }
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
