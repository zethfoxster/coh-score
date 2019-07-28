namespace FxLauncher.FxParser
{
    partial class tgaBox
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(tgaBox));
            this.rgbaPanel = new System.Windows.Forms.Panel();
            this.invertAlpha = new System.Windows.Forms.CheckBox();
            this.aChk = new System.Windows.Forms.CheckBox();
            this.bChk = new System.Windows.Forms.CheckBox();
            this.gChk = new System.Windows.Forms.CheckBox();
            this.rChk = new System.Windows.Forms.CheckBox();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.rgbaPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // rgbaPanel
            // 
            this.rgbaPanel.BackColor = System.Drawing.SystemColors.Control;
            this.rgbaPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.rgbaPanel.Controls.Add(this.invertAlpha);
            this.rgbaPanel.Controls.Add(this.aChk);
            this.rgbaPanel.Controls.Add(this.bChk);
            this.rgbaPanel.Controls.Add(this.gChk);
            this.rgbaPanel.Controls.Add(this.rChk);
            this.rgbaPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.rgbaPanel.Location = new System.Drawing.Point(0, 0);
            this.rgbaPanel.Name = "rgbaPanel";
            this.rgbaPanel.Size = new System.Drawing.Size(534, 25);
            this.rgbaPanel.TabIndex = 3;
            // 
            // invertAlpha
            // 
            this.invertAlpha.AutoSize = true;
            this.invertAlpha.Location = new System.Drawing.Point(151, 3);
            this.invertAlpha.Name = "invertAlpha";
            this.invertAlpha.Size = new System.Drawing.Size(83, 17);
            this.invertAlpha.TabIndex = 1;
            this.invertAlpha.Text = "Invert Alpha";
            this.invertAlpha.UseVisualStyleBackColor = true;
            this.invertAlpha.CheckedChanged += new System.EventHandler(this.invertAlpha_CheckedChanged);
            // 
            // aChk
            // 
            this.aChk.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.aChk.Appearance = System.Windows.Forms.Appearance.Button;
            this.aChk.AutoSize = true;
            this.aChk.Checked = true;
            this.aChk.CheckState = System.Windows.Forms.CheckState.Checked;
            this.aChk.Enabled = true;
            this.aChk.Location = new System.Drawing.Point(121, -1);
            this.aChk.Name = "aChk";
            this.aChk.Size = new System.Drawing.Size(24, 23);
            this.aChk.TabIndex = 0;
            this.aChk.Text = "A";
            this.aChk.UseVisualStyleBackColor = true;
            this.aChk.CheckedChanged += new System.EventHandler(this.chk_CheckedChanged);
            // 
            // bChk
            // 
            this.bChk.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.bChk.Appearance = System.Windows.Forms.Appearance.Button;
            this.bChk.AutoSize = true;
            this.bChk.Checked = true;
            this.bChk.CheckState = System.Windows.Forms.CheckState.Checked;
            this.bChk.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.bChk.ForeColor = System.Drawing.Color.Blue;
            this.bChk.Location = new System.Drawing.Point(82, -1);
            this.bChk.Name = "bChk";
            this.bChk.Size = new System.Drawing.Size(25, 23);
            this.bChk.TabIndex = 0;
            this.bChk.Text = "B";
            this.bChk.UseVisualStyleBackColor = true;
            this.bChk.CheckedChanged += new System.EventHandler(this.chk_CheckedChanged);
            // 
            // gChk
            // 
            this.gChk.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.gChk.Appearance = System.Windows.Forms.Appearance.Button;
            this.gChk.AutoSize = true;
            this.gChk.Checked = true;
            this.gChk.CheckState = System.Windows.Forms.CheckState.Checked;
            this.gChk.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.gChk.ForeColor = System.Drawing.Color.Green;
            this.gChk.Location = new System.Drawing.Point(43, -1);
            this.gChk.Name = "gChk";
            this.gChk.Size = new System.Drawing.Size(26, 23);
            this.gChk.TabIndex = 0;
            this.gChk.Text = "G";
            this.gChk.UseVisualStyleBackColor = true;
            this.gChk.CheckedChanged += new System.EventHandler(this.chk_CheckedChanged);
            // 
            // rChk
            // 
            this.rChk.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.rChk.Appearance = System.Windows.Forms.Appearance.Button;
            this.rChk.AutoSize = true;
            this.rChk.Checked = true;
            this.rChk.CheckState = System.Windows.Forms.CheckState.Checked;
            this.rChk.Font = new System.Drawing.Font("Microsoft Sans Serif", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.rChk.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.rChk.Location = new System.Drawing.Point(4, -1);
            this.rChk.Name = "rChk";
            this.rChk.Size = new System.Drawing.Size(26, 23);
            this.rChk.TabIndex = 0;
            this.rChk.Text = "R";
            this.rChk.UseVisualStyleBackColor = true;
            this.rChk.CheckedChanged += new System.EventHandler(this.chk_CheckedChanged);
            // 
            // pictureBox1
            // 
            this.pictureBox1.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("pictureBox1.BackgroundImage")));
            this.pictureBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pictureBox1.Location = new System.Drawing.Point(0, 25);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(534, 460);
            this.pictureBox1.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.pictureBox1.TabIndex = 4;
            this.pictureBox1.TabStop = false;
            // 
            // tgaBox
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(534, 485);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.rgbaPanel);
            this.Name = "tgaBox";
            this.Text = "tgaBox";
            this.rgbaPanel.ResumeLayout(false);
            this.rgbaPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel rgbaPanel;
        private System.Windows.Forms.CheckBox invertAlpha;
        private System.Windows.Forms.CheckBox aChk;
        private System.Windows.Forms.CheckBox bChk;
        private System.Windows.Forms.CheckBox gChk;
        private System.Windows.Forms.CheckBox rChk;
        private System.Windows.Forms.PictureBox pictureBox1;
    }
}