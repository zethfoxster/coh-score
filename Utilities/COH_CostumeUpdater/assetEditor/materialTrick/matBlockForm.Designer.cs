namespace COH_CostumeUpdater.assetEditor.materialTrick
{
    partial class matBlockForm
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
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.matTrickPropertiesPanel = new System.Windows.Forms.Panel();
            this.topPanel = new System.Windows.Forms.Panel();
            this.deleteMatBtn = new System.Windows.Forms.Button();
            this.renameMatBtn = new System.Windows.Forms.Button();
            this.dupMatBtn = new System.Windows.Forms.Button();
            this.newMatBtn = new System.Windows.Forms.Button();
            this.matTrickListPanel = new System.Windows.Forms.Panel();
            this.p4GrpBx = new System.Windows.Forms.GroupBox();
            this.p4Panel = new COH_CostumeUpdater.common.P4Panel();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.matListCmboBx = new System.Windows.Forms.ComboBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.topPanel.SuspendLayout();
            this.p4GrpBx.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.Filter = "TGA files (*.tga)|*.tga";
            this.openFileDialog1.InitialDirectory = "C:\\game\\src\\Texture_Library";
            // 
            // matTrickPropertiesPanel
            // 
            this.matTrickPropertiesPanel.AutoScroll = true;
            this.matTrickPropertiesPanel.BackColor = System.Drawing.SystemColors.Control;
            this.matTrickPropertiesPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.matTrickPropertiesPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.matTrickPropertiesPanel.Location = new System.Drawing.Point(0, 166);
            this.matTrickPropertiesPanel.Name = "matTrickPropertiesPanel";
            this.matTrickPropertiesPanel.Size = new System.Drawing.Size(1032, 314);
            this.matTrickPropertiesPanel.TabIndex = 0;
            this.matTrickPropertiesPanel.MouseEnter += new System.EventHandler(matTrickPropertiesPanel_MouseEnter);
            // 
            // topPanel
            // 
            this.topPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.topPanel.Controls.Add(this.deleteMatBtn);
            this.topPanel.Controls.Add(this.renameMatBtn);
            this.topPanel.Controls.Add(this.dupMatBtn);
            this.topPanel.Controls.Add(this.newMatBtn);
            this.topPanel.Controls.Add(this.matTrickListPanel);
            this.topPanel.Controls.Add(this.p4GrpBx);
            this.topPanel.Controls.Add(this.groupBox1);
            this.topPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.topPanel.Location = new System.Drawing.Point(0, 0);
            this.topPanel.Name = "topPanel";
            this.topPanel.Size = new System.Drawing.Size(1032, 166);
            this.topPanel.TabIndex = 1;
            // 
            // deleteMatBtn
            // 
            this.deleteMatBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickDelete;
            this.deleteMatBtn.Location = new System.Drawing.Point(10, 134);
            this.deleteMatBtn.Name = "deleteMatBtn";
            this.deleteMatBtn.Size = new System.Drawing.Size(24, 24);
            this.deleteMatBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.deleteMatBtn, "Delete Selected Material Trick...");
            this.deleteMatBtn.UseVisualStyleBackColor = true;
            this.deleteMatBtn.Click += new System.EventHandler(this.deleteMatBtn_Click);
            // 
            // renameMatBtn
            // 
            this.renameMatBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickRename;
            this.renameMatBtn.Location = new System.Drawing.Point(10, 108);
            this.renameMatBtn.Name = "renameMatBtn";
            this.renameMatBtn.Size = new System.Drawing.Size(24, 24);
            this.renameMatBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.renameMatBtn, "Rename Selected Material Trick...");
            this.renameMatBtn.UseVisualStyleBackColor = true;
            this.renameMatBtn.Click += new System.EventHandler(this.renameMatBtn_Click);
            // 
            // dupMatBtn
            // 
            this.dupMatBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickDuplicate;
            this.dupMatBtn.Location = new System.Drawing.Point(10, 82);
            this.dupMatBtn.Name = "dupMatBtn";
            this.dupMatBtn.Size = new System.Drawing.Size(24, 24);
            this.dupMatBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.dupMatBtn, "Duplicate Selected Material Trick...");
            this.dupMatBtn.UseVisualStyleBackColor = true;
            this.dupMatBtn.Click += new System.EventHandler(this.dupMatBtn_Click);
            // 
            // newMatBtn
            // 
            this.newMatBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickNew;
            this.newMatBtn.Location = new System.Drawing.Point(10, 56);
            this.newMatBtn.Name = "newMatBtn";
            this.newMatBtn.Size = new System.Drawing.Size(24, 24);
            this.newMatBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.newMatBtn, "New Material Trick...");
            this.newMatBtn.UseVisualStyleBackColor = true;
            this.newMatBtn.Click += new System.EventHandler(this.newMatBtn_Click);
            // 
            // matTrickListPanel
            // 
            this.matTrickListPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.matTrickListPanel.AutoScroll = true;
            this.matTrickListPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.matTrickListPanel.Location = new System.Drawing.Point(40, 56);
            this.matTrickListPanel.Name = "matTrickListPanel";
            this.matTrickListPanel.Size = new System.Drawing.Size(979, 100);
            this.matTrickListPanel.TabIndex = 3;
            // 
            // p4GrpBx
            // 
            this.p4GrpBx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.p4GrpBx.Controls.Add(this.p4Panel);
            this.p4GrpBx.Location = new System.Drawing.Point(430, 3);
            this.p4GrpBx.Name = "p4GrpBx";
            this.p4GrpBx.Size = new System.Drawing.Size(589, 46);
            this.p4GrpBx.TabIndex = 1;
            this.p4GrpBx.TabStop = false;
            this.p4GrpBx.Text = "P4:";
            // 
            // p4Panel
            // 
            this.p4Panel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.p4Panel.Location = new System.Drawing.Point(3, 16);
            this.p4Panel.Name = "p4Panel";
            this.p4Panel.Size = new System.Drawing.Size(583, 27);
            this.p4Panel.TabIndex = 0;
            this.p4Panel.Text = "P4Panel";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.matListCmboBx);
            this.groupBox1.Location = new System.Drawing.Point(3, 3);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(428, 46);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Selected MatTrick";
            // 
            // matListCmboBx
            // 
            this.matListCmboBx.Dock = System.Windows.Forms.DockStyle.Fill;
            this.matListCmboBx.FormattingEnabled = true;
            this.matListCmboBx.Location = new System.Drawing.Point(3, 16);
            this.matListCmboBx.Name = "matListCmboBx";
            this.matListCmboBx.Size = new System.Drawing.Size(422, 21);
            this.matListCmboBx.TabIndex = 0;
            this.matListCmboBx.SelectionChangeCommitted += new System.EventHandler(this.matListCmboBx_SelectionChangeCommitted);
            // 
            // toolTip1
            // 
            this.toolTip1.ShowAlways = true;
            // 
            // matBlockForm
            // 
            this.ClientSize = new System.Drawing.Size(1032, 480);
            this.Controls.Add(this.matTrickPropertiesPanel);
            this.Controls.Add(this.topPanel);
            this.Name = "matBlockForm";
            this.topPanel.ResumeLayout(false);
            this.p4GrpBx.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.Panel matTrickPropertiesPanel;
        private System.Windows.Forms.Panel topPanel;
        private common.P4Panel p4Panel;
        private System.Windows.Forms.GroupBox p4GrpBx;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ComboBox matListCmboBx;
        private System.Windows.Forms.Panel matTrickListPanel;
        private System.Windows.Forms.Button newMatBtn;
        private System.Windows.Forms.Button deleteMatBtn;
        private System.Windows.Forms.Button renameMatBtn;
        private System.Windows.Forms.Button dupMatBtn;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}