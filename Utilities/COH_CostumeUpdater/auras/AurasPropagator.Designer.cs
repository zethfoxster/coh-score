namespace COH_CostumeUpdater.auras
{
    partial class AurasPropagator
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AurasPropagator));
            this.AurasTV = new System.Windows.Forms.TreeView();
            this.tvContextMenu = new System.Windows.Forms.ContextMenu();
            this.AurasTV_lable = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // AurasTV
            // 
            this.AurasTV.Dock = System.Windows.Forms.DockStyle.Fill;
            this.AurasTV.Location = new System.Drawing.Point(0, 27);
            this.AurasTV.Name = "AurasTV";
            this.AurasTV.ShowNodeToolTips = true;
            this.AurasTV.Size = new System.Drawing.Size(967, 463);
            this.AurasTV.TabIndex = 0;
            this.AurasTV.ContextMenu = this.tvContextMenu;
            // 
            // AurasTV_lable
            // 
            this.AurasTV_lable.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.AurasTV_lable.Dock = System.Windows.Forms.DockStyle.Top;
            this.AurasTV_lable.Location = new System.Drawing.Point(0, 0);
            this.AurasTV_lable.Name = "AurasTV_lable";
            this.AurasTV_lable.Size = new System.Drawing.Size(967, 27);
            this.AurasTV_lable.TabIndex = 1;
            this.AurasTV_lable.Text = "Current File:";
            this.AurasTV_lable.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // COH_CostumeUpdaterForm
            // 
            //this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            //this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1214, 669);
            this.Controls.Add(this.AurasTV);
            this.Controls.Add(this.AurasTV_lable);
            this.Name = "AurasPropagator";
            this.Text = "Auras Propagator";
            this.ResumeLayout(false);
            this.PerformLayout();
        }


        #endregion

        private string fileName;
        private System.Windows.Forms.Label AurasTV_lable;
        private System.Windows.Forms.TreeView AurasTV;
        private System.Windows.Forms.ContextMenu tvContextMenu;
        private System.Windows.Forms.RichTextBox logTxBx;
    }
}

