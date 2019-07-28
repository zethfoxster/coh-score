namespace COH_CostumeUpdater.costume
{
    partial class CostumeEditor
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
            this.isCostumeFile = false;
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CostumeEditor));
            this.imgeList = new System.Windows.Forms.ImageList(); 
            this.CostumeTV = new System.Windows.Forms.TreeView();
            this.tvContextMenu = new System.Windows.Forms.ContextMenu();
            this.CostumeTV_lable = new System.Windows.Forms.Label();
            this.copySource = null;
            this.SuspendLayout();
            // 
            // CostumeTV
            // 
            this.CostumeTV.Dock = System.Windows.Forms.DockStyle.Fill;
            this.CostumeTV.Location = new System.Drawing.Point(0, 27);
            this.CostumeTV.Name = "CostumeTV";
            this.CostumeTV.ShowNodeToolTips = true;
            //this.CostumeTV.Size = new System.Drawing.Size(967, 463);
            this.CostumeTV.TabIndex = 0;
            this.CostumeTV.ImageList = this.imgeList;
            this.CostumeTV.ContextMenu = this.tvContextMenu;
            // 
            // CostumeTV_lable
            // 
            this.CostumeTV_lable.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.CostumeTV_lable.Dock = System.Windows.Forms.DockStyle.Top;
            this.CostumeTV_lable.Location = new System.Drawing.Point(0, 0);
            this.CostumeTV_lable.Name = "CostumeTV_lable";
            this.CostumeTV_lable.Size = new System.Drawing.Size(967, 27);
            this.CostumeTV_lable.TabIndex = 1;
            this.CostumeTV_lable.Text = "Current File:";
            this.CostumeTV_lable.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // COH_CostumeUpdaterForm
            // 
            //this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            //this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1214, 669);
            this.Controls.Add(this.CostumeTV);
            this.Controls.Add(this.CostumeTV_lable);
            //this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "CostumeEditor";
            this.Text = "COH CostumeEditor";
            this.ResumeLayout(false);
            this.PerformLayout();

        }


        #endregion
        private bool isCostumeFile;
        private string fileName;
        private System.Windows.Forms.RichTextBox logTxBx;

        private COH_CostumeUpdater.costume.CharacterCostumeFile ccf;
        private System.Windows.Forms.TreeNode copySource;
        private System.Windows.Forms.ImageList imgeList;
        private System.Windows.Forms.Label CostumeTV_lable;
        private System.Windows.Forms.TreeView CostumeTV;
        private System.Windows.Forms.ContextMenu tvContextMenu;
    }
}

