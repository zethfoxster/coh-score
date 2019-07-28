namespace COH_CostumeUpdater.assetEditor
{
    partial class AssetTricks
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
            this.assetTricks_tabControl = new System.Windows.Forms.TabControl();
            this.materialTricks_tabPage = new System.Windows.Forms.TabPage();
            this.objectTricks_tabPage = new System.Windows.Forms.TabPage();
            this.textureTricks_tabPage = new System.Windows.Forms.TabPage();
            this.assetTricks_tabControl.SuspendLayout();
            this.SuspendLayout();
            // 
            // assetTricks_tabControl
            // 
            this.assetTricks_tabControl.Controls.Add(this.materialTricks_tabPage);
            this.assetTricks_tabControl.Controls.Add(this.objectTricks_tabPage);
            this.assetTricks_tabControl.Controls.Add(this.textureTricks_tabPage);
            this.assetTricks_tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.assetTricks_tabControl.Location = new System.Drawing.Point(0, 0);
            this.assetTricks_tabControl.Name = "assetTricks_tabControl";
            this.assetTricks_tabControl.SelectedIndex = 0;
            this.assetTricks_tabControl.Size = new System.Drawing.Size(922, 650);
            this.assetTricks_tabControl.TabIndex = 0;
            // 
            // materialTricks_tabPage
            // 
            this.materialTricks_tabPage.BackColor = System.Drawing.SystemColors.Control;
            this.materialTricks_tabPage.Location = new System.Drawing.Point(4, 22);
            this.materialTricks_tabPage.Name = "materialTricks_tabPage";
            this.materialTricks_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.materialTricks_tabPage.Size = new System.Drawing.Size(914, 624);
            this.materialTricks_tabPage.TabIndex = 0;
            this.materialTricks_tabPage.Text = "Material Tricks";
            // 
            // objectTricks_tabPage
            // 
            this.objectTricks_tabPage.BackColor = System.Drawing.SystemColors.Control;
            this.objectTricks_tabPage.Location = new System.Drawing.Point(4, 22);
            this.objectTricks_tabPage.Name = "objectTricks_tabPage";
            this.objectTricks_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.objectTricks_tabPage.Size = new System.Drawing.Size(914, 624);
            this.objectTricks_tabPage.TabIndex = 1;
            this.objectTricks_tabPage.Text = "Object Tricks";
            // 
            // textureTricks_tabPage
            // 
            this.textureTricks_tabPage.BackColor = System.Drawing.SystemColors.Control;
            this.textureTricks_tabPage.Location = new System.Drawing.Point(4, 22);
            this.textureTricks_tabPage.Name = "textureTricks_tabPage";
            this.textureTricks_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.textureTricks_tabPage.Size = new System.Drawing.Size(914, 624);
            this.textureTricks_tabPage.TabIndex = 2;
            this.textureTricks_tabPage.Text = "Texture Tricks";
            // 
            // AssetTricks
            // 
            this.ClientSize = new System.Drawing.Size(922, 650);
            this.Controls.Add(this.assetTricks_tabControl);
            this.Name = "AssetTricks";
            this.Text = "AssetTricks";
            this.assetTricks_tabControl.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TabControl assetTricks_tabControl;
        private System.Windows.Forms.TabPage materialTricks_tabPage;
        private System.Windows.Forms.TabPage objectTricks_tabPage;
        private System.Windows.Forms.TabPage textureTricks_tabPage;
    }
}