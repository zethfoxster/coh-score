namespace COH_CostumeUpdater.common
{
    partial class P4Panel
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
            this.p4Diff_btn = new System.Windows.Forms.Button();
            this.p4CheckIn_btn = new System.Windows.Forms.Button();
            this.p4CheckOut_btn = new System.Windows.Forms.Button();
            this.p4Add_btn = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // p4Diff_btn
            // 
            this.p4Diff_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4Diff_btn.Location = new System.Drawing.Point(546, 4);
            this.p4Diff_btn.Name = "p4Diff_btn";
            this.p4Diff_btn.Size = new System.Drawing.Size(75, 23);
            this.p4Diff_btn.TabIndex = 0;
            this.p4Diff_btn.Text = "Diff";
            this.p4Diff_btn.UseVisualStyleBackColor = true;
            this.p4Diff_btn.Click += new System.EventHandler(p4DepotDiff_btn_Click);
            // 
            // p4CheckIn_btn
            // 
            this.p4CheckIn_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4CheckIn_btn.Location = new System.Drawing.Point(465, 4);
            this.p4CheckIn_btn.Name = "p4CheckIn_btn";
            this.p4CheckIn_btn.Size = new System.Drawing.Size(75, 23);
            this.p4CheckIn_btn.TabIndex = 0;
            this.p4CheckIn_btn.Text = "Check In";
            this.p4CheckIn_btn.UseVisualStyleBackColor = true;
            this.p4CheckIn_btn.Click += new System.EventHandler(p4CheckIn_btn_Click);
            // 
            // p4CheckOut_btn
            // 
            this.p4CheckOut_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4CheckOut_btn.Location = new System.Drawing.Point(384, 4);
            this.p4CheckOut_btn.Name = "p4CheckOut_btn";
            this.p4CheckOut_btn.Size = new System.Drawing.Size(75, 23);
            this.p4CheckOut_btn.TabIndex = 0;
            this.p4CheckOut_btn.Text = "Check Out";
            this.p4CheckOut_btn.UseVisualStyleBackColor = true;
            this.p4CheckOut_btn.Click += new System.EventHandler(p4CheckOut_btn_Click);
            // 
            // p4Add_btn
            // 
            this.p4Add_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4Add_btn.Location = new System.Drawing.Point(303, 4);
            this.p4Add_btn.Name = "p4Add_btn";
            this.p4Add_btn.Size = new System.Drawing.Size(75, 23);
            this.p4Add_btn.TabIndex = 0;
            this.p4Add_btn.Text = "Add";
            this.p4Add_btn.UseVisualStyleBackColor = true;
            this.p4Add_btn.Click += new System.EventHandler(p4Add_btn_Click);
            // 
            // P4Panel
            // 
            this.ClientSize = new System.Drawing.Size(628, 32);
            this.Controls.Add(this.p4Add_btn);
            this.Controls.Add(this.p4CheckOut_btn);
            this.Controls.Add(this.p4CheckIn_btn);
            this.Controls.Add(this.p4Diff_btn);
            this.Name = "P4Panel";
            this.Text = "P4Panel";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button p4Diff_btn;
        private System.Windows.Forms.Button p4CheckIn_btn;
        private System.Windows.Forms.Button p4CheckOut_btn;
        private System.Windows.Forms.Button p4Add_btn;
    }
}