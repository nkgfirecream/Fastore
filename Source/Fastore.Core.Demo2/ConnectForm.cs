using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Fastore.Core.Demo2
{
	public partial class ConnectForm : Form
	{
		public ConnectForm()
		{
			InitializeComponent();
		}

		public string Address
		{
			get { return AddressBox.Text; }
		}

		public int Port
		{
			get { return (int)PortBox.Value; }
		}

        public bool Detect
        {
            get { return DetectCheck.Checked; }
        }
	}
}
