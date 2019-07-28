namespace COH_CostumeUpdater
{
    partial class PicBxNTitle
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
            this.iconPicBx = new System.Windows.Forms.PictureBox();
            this.iconLabel = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.iconPicBx)).BeginInit();
            this.SuspendLayout();
            // 
            // iconPicBx
            // 
            this.iconPicBx.Dock = System.Windows.Forms.DockStyle.Fill;
            this.iconPicBx.Location = new System.Drawing.Point(0, 0);
            this.iconPicBx.Name = "iconPicBx";
            this.iconPicBx.Size = new System.Drawing.Size(168, 168);
            this.iconPicBx.TabIndex = 0;
            this.iconPicBx.TabStop = false;
            this.iconPicBx.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.iconPicBx.Click += new System.EventHandler(this.iconPicBx_Click);
            // 
            // iconLabel
            // 
            this.iconLabel.AutoSize = false;
            this.iconLabel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.iconLabel.Location = new System.Drawing.Point(0, 170);
            this.iconLabel.Name = "iconLabel";
            this.iconLabel.Size = new System.Drawing.Size(168, 13);
            this.iconLabel.TabIndex = 1;
            this.iconLabel.Text = "label1";
            // 
            // PicBxNTitle
            // 
            //this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            //this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(168, 183);
            this.Controls.Add(this.iconPicBx);
            this.Controls.Add(this.iconLabel);
            this.Name = "PicBxNTitle";
            this.Text = "PicBxNTitle";
            ((System.ComponentModel.ISupportInitialize)(this.iconPicBx)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox iconPicBx;
        private System.Windows.Forms.Label iconLabel;
    }
}