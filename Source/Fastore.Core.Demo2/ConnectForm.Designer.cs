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
			((System.ComponentModel.ISupportInitialize)(this.PortBox)).BeginInit();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(14, 15);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(93, 17);
			this.label1.TabIndex = 0;
			this.label1.Text = "Host Address";
			// 
			// AddressBox
			// 
			this.AddressBox.Location = new System.Drawing.Point(17, 35);
			this.AddressBox.Name = "AddressBox";
			this.AddressBox.Size = new System.Drawing.Size(371, 22);
			this.AddressBox.TabIndex = 1;
			this.AddressBox.Text = "localhost";
			// 
			// OKButton
			// 
			this.OKButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.OKButton.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.OKButton.Location = new System.Drawing.Point(162, 112);
			this.OKButton.Name = "OKButton";
			this.OKButton.Size = new System.Drawing.Size(110, 27);
			this.OKButton.TabIndex = 4;
			this.OKButton.Text = "&OK";
			this.OKButton.UseVisualStyleBackColor = true;
			// 
			// CancelBtn
			// 
			this.CancelBtn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.CancelBtn.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.CancelBtn.Location = new System.Drawing.Point(281, 114);
			this.CancelBtn.Name = "CancelBtn";
			this.CancelBtn.Size = new System.Drawing.Size(107, 24);
			this.CancelBtn.TabIndex = 5;
			this.CancelBtn.Text = "Cancel";
			this.CancelBtn.UseVisualStyleBackColor = true;
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(14, 60);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(34, 17);
			this.label2.TabIndex = 2;
			this.label2.Text = "Port";
			// 
			// PortBox
			// 
			this.PortBox.Location = new System.Drawing.Point(17, 80);
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
			// ConnectForm
			// 
			this.AcceptButton = this.OKButton;
			this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(407, 149);
			this.Controls.Add(this.PortBox);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.CancelBtn);
			this.Controls.Add(this.OKButton);
			this.Controls.Add(this.AddressBox);
			this.Controls.Add(this.label1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
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
	}
}