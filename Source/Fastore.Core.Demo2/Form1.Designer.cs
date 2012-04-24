namespace Fastore.Core.Demo2
{
	partial class Form1
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.IDSearch = new System.Windows.Forms.TextBox();
            this.listView1 = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.GivenSearch = new System.Windows.Forms.TextBox();
            this.SurnameSearch = new System.Windows.Forms.TextBox();
            this.GenderSearch = new System.Windows.Forms.TextBox();
            this.BirthDateSearch = new System.Windows.Forms.TextBox();
            this.BirthPlaceSearch = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // comboBox1
            // 
            this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox1.FormattingEnabled = true;
            this.comboBox1.Items.AddRange(new object[] {
            "ID",
            "Given",
            "Surname",
            "Gender",
            "BirthDate",
            "BirthPlace"});
            this.comboBox1.Location = new System.Drawing.Point(11, 11);
            this.comboBox1.Margin = new System.Windows.Forms.Padding(2);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(100, 21);
            this.comboBox1.TabIndex = 0;
            this.comboBox1.SelectedIndexChanged += new System.EventHandler(this.comboBox1_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(402, 14);
            this.label1.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(41, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Search";
            // 
            // IDSearch
            // 
            this.IDSearch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.IDSearch.Location = new System.Drawing.Point(11, 36);
            this.IDSearch.Margin = new System.Windows.Forms.Padding(2);
            this.IDSearch.Name = "IDSearch";
            this.IDSearch.Size = new System.Drawing.Size(112, 20);
            this.IDSearch.TabIndex = 2;
            this.IDSearch.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // listView1
            // 
            this.listView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4,
            this.columnHeader5,
            this.columnHeader6});
            this.listView1.Location = new System.Drawing.Point(11, 60);
            this.listView1.Margin = new System.Windows.Forms.Padding(2);
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(904, 493);
            this.listView1.TabIndex = 3;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "ID";
            this.columnHeader1.Width = 112;
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Given";
            this.columnHeader2.Width = 156;
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Surname";
            this.columnHeader3.Width = 135;
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Is Male";
            this.columnHeader4.Width = 116;
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "Birth Date";
            this.columnHeader5.Width = 184;
            // 
            // columnHeader6
            // 
            this.columnHeader6.Text = "Birth Place";
            this.columnHeader6.Width = 198;
            // 
            // GivenSearch
            // 
            this.GivenSearch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.GivenSearch.Location = new System.Drawing.Point(127, 36);
            this.GivenSearch.Margin = new System.Windows.Forms.Padding(2);
            this.GivenSearch.Name = "GivenSearch";
            this.GivenSearch.Size = new System.Drawing.Size(151, 20);
            this.GivenSearch.TabIndex = 4;
            this.GivenSearch.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // SurnameSearch
            // 
            this.SurnameSearch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.SurnameSearch.Location = new System.Drawing.Point(282, 36);
            this.SurnameSearch.Margin = new System.Windows.Forms.Padding(2);
            this.SurnameSearch.Name = "SurnameSearch";
            this.SurnameSearch.Size = new System.Drawing.Size(131, 20);
            this.SurnameSearch.TabIndex = 5;
            this.SurnameSearch.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // GenderSearch
            // 
            this.GenderSearch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.GenderSearch.Location = new System.Drawing.Point(417, 36);
            this.GenderSearch.Margin = new System.Windows.Forms.Padding(2);
            this.GenderSearch.Name = "GenderSearch";
            this.GenderSearch.Size = new System.Drawing.Size(112, 20);
            this.GenderSearch.TabIndex = 6;
            this.GenderSearch.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // BirthDateSearch
            // 
            this.BirthDateSearch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.BirthDateSearch.Location = new System.Drawing.Point(533, 36);
            this.BirthDateSearch.Margin = new System.Windows.Forms.Padding(2);
            this.BirthDateSearch.Name = "BirthDateSearch";
            this.BirthDateSearch.Size = new System.Drawing.Size(178, 20);
            this.BirthDateSearch.TabIndex = 7;
            this.BirthDateSearch.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // BirthPlaceSearch
            // 
            this.BirthPlaceSearch.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.BirthPlaceSearch.Location = new System.Drawing.Point(719, 36);
            this.BirthPlaceSearch.Margin = new System.Windows.Forms.Padding(2);
            this.BirthPlaceSearch.Name = "BirthPlaceSearch";
            this.BirthPlaceSearch.Size = new System.Drawing.Size(186, 20);
            this.BirthPlaceSearch.TabIndex = 8;
            this.BirthPlaceSearch.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(926, 564);
            this.Controls.Add(this.BirthPlaceSearch);
            this.Controls.Add(this.BirthDateSearch);
            this.Controls.Add(this.GenderSearch);
            this.Controls.Add(this.SurnameSearch);
            this.Controls.Add(this.GivenSearch);
            this.Controls.Add(this.listView1);
            this.Controls.Add(this.IDSearch);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.comboBox1);
            this.Margin = new System.Windows.Forms.Padding(2);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ComboBox comboBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox IDSearch;
		private System.Windows.Forms.ListView listView1;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader4;
		private System.Windows.Forms.ColumnHeader columnHeader5;
		private System.Windows.Forms.ColumnHeader columnHeader6;
        private System.Windows.Forms.TextBox GivenSearch;
        private System.Windows.Forms.TextBox SurnameSearch;
        private System.Windows.Forms.TextBox GenderSearch;
        private System.Windows.Forms.TextBox BirthDateSearch;
        private System.Windows.Forms.TextBox BirthPlaceSearch;
	}
}

