namespace COH_CostumeUpdater.assetsMangement
{
    partial class P4SubmitWin
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
            this.description_textBox = new System.Windows.Forms.TextBox();
            this.keepCheckedOut_ckbx = new System.Windows.Forms.CheckBox();
            this.submit_btn = new System.Windows.Forms.Button();
            this.cancel_btn = new System.Windows.Forms.Button();
            this.panel1 = new System.Windows.Forms.Panel();
            this.label1 = new System.Windows.Forms.Label();
            this.fileName_label = new System.Windows.Forms.Label();
            this.filePath_label = new System.Windows.Forms.Label();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // description_textBox
            // 
            this.description_textBox.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.description_textBox.Location = new System.Drawing.Point(12, 25);
            this.description_textBox.Multiline = true;
            this.description_textBox.Name = "description_textBox";
            this.description_textBox.Size = new System.Drawing.Size(780, 97);
            this.description_textBox.TabIndex = 0;
            this.description_textBox.TextChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // keepCheckedOut_ckbx
            // 
            this.keepCheckedOut_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.keepCheckedOut_ckbx.AutoSize = true;
            this.keepCheckedOut_ckbx.Location = new System.Drawing.Point(596, 183);
            this.keepCheckedOut_ckbx.Name = "keepCheckedOut_ckbx";
            this.keepCheckedOut_ckbx.Size = new System.Drawing.Size(196, 17);
            this.keepCheckedOut_ckbx.TabIndex = 1;
            this.keepCheckedOut_ckbx.Text = "Check out submitted file after submit";
            this.keepCheckedOut_ckbx.UseVisualStyleBackColor = true;
            // 
            // submit_btn
            // 
            this.submit_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.submit_btn.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.submit_btn.Enabled = false;
            this.submit_btn.Location = new System.Drawing.Point(596, 206);
            this.submit_btn.Name = "submit_btn";
            this.submit_btn.Size = new System.Drawing.Size(95, 23);
            this.submit_btn.TabIndex = 2;
            this.submit_btn.Text = "Submit";
            this.submit_btn.UseVisualStyleBackColor = true;
            // 
            // cancel_btn
            // 
            this.cancel_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cancel_btn.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancel_btn.Location = new System.Drawing.Point(696, 206);
            this.cancel_btn.Name = "cancel_btn";
            this.cancel_btn.Size = new System.Drawing.Size(95, 23);
            this.cancel_btn.TabIndex = 2;
            this.cancel_btn.Text = "Cancel";
            this.cancel_btn.UseVisualStyleBackColor = true;
            // 
            // panel1
            // 
            this.panel1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.panel1.BackColor = System.Drawing.SystemColors.ControlLight;
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.panel1.Controls.Add(this.pictureBox1);
            this.panel1.Controls.Add(this.filePath_label);
            this.panel1.Controls.Add(this.fileName_label);
            this.panel1.Location = new System.Drawing.Point(11, 128);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(780, 43);
            this.panel1.TabIndex = 3;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.label1.Location = new System.Drawing.Point(12, 7);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(114, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "Write a description";
            // 
            // fileName_label
            // 
            this.fileName_label.AutoSize = true;
            this.fileName_label.Location = new System.Drawing.Point(36, 12);
            this.fileName_label.Name = "fileName_label";
            this.fileName_label.Size = new System.Drawing.Size(48, 13);
            this.fileName_label.TabIndex = 0;
            this.fileName_label.Text = "fileName";
            // 
            // filePath_label
            // 
            this.filePath_label.AutoSize = true;
            this.filePath_label.Location = new System.Drawing.Point(262, 12);
            this.filePath_label.Name = "filePath_label";
            this.filePath_label.Size = new System.Drawing.Size(42, 13);
            this.filePath_label.TabIndex = 1;
            this.filePath_label.Text = "filePath";
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = global::COH_CostumeUpdater.Properties.Resources.DOC;
            this.pictureBox1.Location = new System.Drawing.Point(11, 11);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(16, 16);
            this.pictureBox1.TabIndex = 2;
            this.pictureBox1.TabStop = false;
            // 
            // P4SubmitWin
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(805, 235);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.panel1);
            this.Controls.Add(this.cancel_btn);
            this.Controls.Add(this.submit_btn);
            this.Controls.Add(this.keepCheckedOut_ckbx);
            this.Controls.Add(this.description_textBox);
            this.Name = "P4SubmitWin";
            this.Text = "P4 Check In:";
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox description_textBox;
        private System.Windows.Forms.CheckBox keepCheckedOut_ckbx;
        private System.Windows.Forms.Button submit_btn;
        private System.Windows.Forms.Button cancel_btn;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label filePath_label;
        private System.Windows.Forms.Label fileName_label;
        private System.Windows.Forms.PictureBox pictureBox1;
    }
}