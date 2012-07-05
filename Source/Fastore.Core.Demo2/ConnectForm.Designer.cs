namespace Fastore.Core.Demo2
{
	partial class ConnectForm
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
			this.label1 = new System.Windows.Forms.Label();
			this.AddressBox = new System.Windows.Forms.TextBox();
			this.OKButton = new System.Windows.Forms.Button();
			this.CancelBtn = new System.Windows.Forms.Button();
			this.label2 = new System.Windows.Forms.Label();
			this.PortBox = new System.Windows.Forms.NumericUpDown();
			this.DetectCheck = new System.Windows.Forms.CheckBox();
			this.list = new System.Windows.Forms.ListView();
			this.button1 = new System.Windows.Forms.Button();
			this.button2 = new System.Windows.Forms.Button();
			this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
			((System.ComponentModel.ISupportInitialize)(this.PortBox)).BeginInit();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(13, 15);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(93, 17);
			this.label1.TabIndex = 0;
			this.label1.Text = "Host Address";
			// 
			// AddressBox
			// 
			this.AddressBox.Location = new System.Drawing.Point(17, 34);
			this.AddressBox.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.AddressBox.Name = "AddressBox";
			this.AddressBox.Size = new System.Drawing.Size(371, 22);
			this.AddressBox.TabIndex = 1;
			this.AddressBox.Text = "localhost";
			// 
			// OKButton
			// 
			this.OKButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.OKButton.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.OKButton.Location = new System.Drawing.Point(233, 252);
			this.OKButton.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.OKButton.Name = "OKButton";
			this.OKButton.Size = new System.Drawing.Size(109, 27);
			this.OKButton.TabIndex = 4;
			this.OKButton.Text = "&OK";
			this.OKButton.UseVisualStyleBackColor = true;
			// 
			// CancelBtn
			// 
			this.CancelBtn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.CancelBtn.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.CancelBtn.Location = new System.Drawing.Point(351, 254);
			this.CancelBtn.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.CancelBtn.Name = "CancelBtn";
			this.CancelBtn.Size = new System.Drawing.Size(107, 25);
			this.CancelBtn.TabIndex = 5;
			this.CancelBtn.Text = "Cancel";
			this.CancelBtn.UseVisualStyleBackColor = true;
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(13, 60);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(34, 17);
			this.label2.TabIndex = 2;
			this.label2.Text = "Port";
			// 
			// PortBox
			// 
			this.PortBox.Location = new System.Drawing.Point(17, 80);
			this.PortBox.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.PortBox.Maximum = new decimal(new int[] {
            65900,
            0,
            0,
            0});
			this.PortBox.Name = "PortBox";
			this.PortBox.Size = new System.Drawing.Size(132, 22);
			this.PortBox.TabIndex = 3;
			this.PortBox.Value = new decimal(new int[] {
            8765,
            0,
            0,
            0});
			// 
			// DetectCheck
			// 
			this.DetectCheck.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.DetectCheck.AutoSize = true;
			this.DetectCheck.Location = new System.Drawing.Point(17, 215);
			this.DetectCheck.Margin = new System.Windows.Forms.Padding(4, 4, 4, 4);
			this.DetectCheck.Name = "DetectCheck";
			this.DetectCheck.Size = new System.Drawing.Size(249, 21);
			this.DetectCheck.TabIndex = 6;
			this.DetectCheck.Text = "Detect Previously Created Schema";
			this.DetectCheck.UseVisualStyleBackColor = true;
			// 
			// list
			// 
			this.list.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.list.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
			this.list.Location = new System.Drawing.Point(16, 108);
			this.list.MultiSelect = false;
			this.list.Name = "list";
			this.list.Size = new System.Drawing.Size(449, 100);
			this.list.TabIndex = 7;
			this.list.UseCompatibleStateImageBehavior = false;
			this.list.View = System.Windows.Forms.View.Details;
			// 
			// button1
			// 
			this.button1.Location = new System.Drawing.Point(307, 80);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(76, 23);
			this.button1.TabIndex = 8;
			this.button1.Text = "Add";
			this.button1.UseVisualStyleBackColor = true;
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// button2
			// 
			this.button2.Location = new System.Drawing.Point(389, 80);
			this.button2.Name = "button2";
			this.button2.Size = new System.Drawing.Size(76, 23);
			this.button2.TabIndex = 9;
			this.button2.Text = "Remove";
			this.button2.UseVisualStyleBackColor = true;
			this.button2.Click += new System.EventHandler(this.button2_Click);
			// 
			// columnHeader1
			// 
			this.columnHeader1.Text = "Address";
			this.columnHeader1.Width = 360;
			// 
			// columnHeader2
			// 
			this.columnHeader2.Text = "Port";
			this.columnHeader2.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
			this.columnHeader2.Width = 80;
			// 
			// ConnectForm
			// 
			this.AcceptButton = this.OKButton;
			this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(477, 289);
			this.Controls.Add(this.button2);
			this.Controls.Add(this.button1);
			this.Controls.Add(this.list);
			this.Controls.Add(this.DetectCheck);
			this.Controls.Add(this.PortBox);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.CancelBtn);
			this.Controls.Add(this.OKButton);
			this.Controls.Add(this.AddressBox);
			this.Controls.Add(this.label1);
			this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
			this.Name = "ConnectForm";
			this.Text = "Connect to Fastore Hive...";
			((System.ComponentModel.ISupportInitialize)(this.PortBox)).EndInit();
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox AddressBox;
		private System.Windows.Forms.Button OKButton;
		private System.Windows.Forms.Button CancelBtn;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.NumericUpDown PortBox;
        private System.Windows.Forms.CheckBox DetectCheck;
		private System.Windows.Forms.ListView list;
		private System.Windows.Forms.ColumnHeader columnHeader1;
		private System.Windows.Forms.ColumnHeader columnHeader2;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.Button button2;
	}
}