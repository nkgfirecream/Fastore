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
			this.Search = new System.Windows.Forms.TextBox();
			this.listView1 = new System.Windows.Forms.ListView();
			this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader5 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader6 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.StatusBox = new System.Windows.Forms.TextBox();
			this.StopButton = new System.Windows.Forms.Button();
			this.splitter1 = new System.Windows.Forms.Splitter();
			this.panel1 = new System.Windows.Forms.Panel();
			this.comboBox2 = new System.Windows.Forms.ComboBox();
			this.panel1.SuspendLayout();
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
			this.comboBox1.Location = new System.Drawing.Point(15, 11);
			this.comboBox1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.comboBox1.Name = "comboBox1";
			this.comboBox1.Size = new System.Drawing.Size(132, 24);
			this.comboBox1.TabIndex = 0;
			this.comboBox1.SelectedIndexChanged += new System.EventHandler(this.comboBox1_SelectedIndexChanged);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(271, 15);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(53, 17);
			this.label1.TabIndex = 1;
			this.label1.Text = "Search";
			// 
			// Search
			// 
			this.Search.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.Search.Location = new System.Drawing.Point(331, 11);
			this.Search.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.Search.Name = "Search";
			this.Search.Size = new System.Drawing.Size(599, 22);
			this.Search.TabIndex = 2;
			this.Search.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
			// 
			// listView1
			// 
			this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4,
            this.columnHeader5,
            this.columnHeader6});
			this.listView1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.listView1.Location = new System.Drawing.Point(0, 47);
			this.listView1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.listView1.Name = "listView1";
			this.listView1.Size = new System.Drawing.Size(945, 586);
			this.listView1.TabIndex = 3;
			this.listView1.UseCompatibleStateImageBehavior = false;
			this.listView1.View = System.Windows.Forms.View.Details;
			this.listView1.CacheVirtualItems += new System.Windows.Forms.CacheVirtualItemsEventHandler(this.listView1_CacheVirtualItems);
			this.listView1.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.listView1_RetrieveVirtualItem);
			this.listView1.SearchForVirtualItem += new System.Windows.Forms.SearchForVirtualItemEventHandler(this.listView1_SearchForVirtualItem);
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
			// StatusBox
			// 
			this.StatusBox.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.StatusBox.Location = new System.Drawing.Point(0, 633);
			this.StatusBox.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.StatusBox.Multiline = true;
			this.StatusBox.Name = "StatusBox";
			this.StatusBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.StatusBox.Size = new System.Drawing.Size(945, 45);
			this.StatusBox.TabIndex = 4;
			// 
			// StopButton
			// 
			this.StopButton.Location = new System.Drawing.Point(331, 314);
			this.StopButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.StopButton.Name = "StopButton";
			this.StopButton.Size = new System.Drawing.Size(229, 27);
			this.StopButton.TabIndex = 5;
			this.StopButton.Text = "Loading... press to stop.";
			this.StopButton.UseVisualStyleBackColor = true;
			this.StopButton.Click += new System.EventHandler(this.StopButton_Click);
			// 
			// splitter1
			// 
			this.splitter1.Dock = System.Windows.Forms.DockStyle.Bottom;
			this.splitter1.Location = new System.Drawing.Point(0, 631);
			this.splitter1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.splitter1.Name = "splitter1";
			this.splitter1.Size = new System.Drawing.Size(945, 2);
			this.splitter1.TabIndex = 6;
			this.splitter1.TabStop = false;
			// 
			// panel1
			// 
			this.panel1.Controls.Add(this.comboBox2);
			this.panel1.Controls.Add(this.label1);
			this.panel1.Controls.Add(this.comboBox1);
			this.panel1.Controls.Add(this.Search);
			this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
			this.panel1.Location = new System.Drawing.Point(0, 0);
			this.panel1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.panel1.Name = "panel1";
			this.panel1.Size = new System.Drawing.Size(945, 47);
			this.panel1.TabIndex = 7;
			// 
			// comboBox2
			// 
			this.comboBox2.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.comboBox2.FormattingEnabled = true;
			this.comboBox2.Items.AddRange(new object[] {
            "Asc",
            "Desc"});
			this.comboBox2.Location = new System.Drawing.Point(153, 11);
			this.comboBox2.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.comboBox2.Name = "comboBox2";
			this.comboBox2.Size = new System.Drawing.Size(111, 24);
			this.comboBox2.TabIndex = 3;
			this.comboBox2.SelectedIndexChanged += new System.EventHandler(this.comboBox2_SelectedIndexChanged);
			// 
			// Form1
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(945, 678);
			this.Controls.Add(this.splitter1);
			this.Controls.Add(this.StopButton);
			this.Controls.Add(this.listView1);
			this.Controls.Add(this.StatusBox);
			this.Controls.Add(this.panel1);
			this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.Name = "Form1";
			this.Text = "Form1";
			this.Shown += new System.EventHandler(this.Form1_Shown);
			this.panel1.ResumeLayout(false);
			this.panel1.PerformLayout();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.ComboBox comboBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox Search;
		private System.Windows.Forms.ListView listView1;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.ColumnHeader columnHeader3;
		private System.Windows.Forms.ColumnHeader columnHeader4;
		private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.ColumnHeader columnHeader6;
		private System.Windows.Forms.TextBox StatusBox;
		private System.Windows.Forms.Button StopButton;
		private System.Windows.Forms.Splitter splitter1;
		private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.ComboBox comboBox2;
	}
}

