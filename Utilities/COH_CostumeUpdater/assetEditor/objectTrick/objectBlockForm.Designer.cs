namespace COH_CostumeUpdater.assetEditor.objectTrick
{
    partial class ObjectBlockForm
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
            this.objTrickPropertiesPanel = new System.Windows.Forms.Panel();
            this.topPanel = new System.Windows.Forms.Panel();
            this.deleteObjTrickBtn = new System.Windows.Forms.Button();
            this.renameObjTrickBtn = new System.Windows.Forms.Button();
            this.dupObjTrickBtn = new System.Windows.Forms.Button();
            this.newObjTrickBtn = new System.Windows.Forms.Button();
            this.objTrickListPanel = new System.Windows.Forms.Panel();
            this.p4GrpBx = new System.Windows.Forms.GroupBox();
            this.p4Panel = new COH_CostumeUpdater.common.P4Panel();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.objListCmboBx = new System.Windows.Forms.ComboBox();
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
            // objTrickPropertiesPanel
            // 
            this.objTrickPropertiesPanel.AutoScroll = true;
            this.objTrickPropertiesPanel.BackColor = System.Drawing.SystemColors.Control;
            this.objTrickPropertiesPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.objTrickPropertiesPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.objTrickPropertiesPanel.Location = new System.Drawing.Point(0, 166);
            this.objTrickPropertiesPanel.Name = "objTrickPropertiesPanel";
            this.objTrickPropertiesPanel.Size = new System.Drawing.Size(1032, 314);
            this.objTrickPropertiesPanel.TabIndex = 0;
            // 
            // topPanel
            // 
            this.topPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.topPanel.Controls.Add(this.deleteObjTrickBtn);
            this.topPanel.Controls.Add(this.renameObjTrickBtn);
            this.topPanel.Controls.Add(this.dupObjTrickBtn);
            this.topPanel.Controls.Add(this.newObjTrickBtn);
            this.topPanel.Controls.Add(this.objTrickListPanel);
            this.topPanel.Controls.Add(this.p4GrpBx);
            this.topPanel.Controls.Add(this.groupBox1);
            this.topPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.topPanel.Location = new System.Drawing.Point(0, 0);
            this.topPanel.Name = "topPanel";
            this.topPanel.Size = new System.Drawing.Size(1032, 166);
            this.topPanel.TabIndex = 1;
            // 
            // deleteObjTrickBtn
            // 
            this.deleteObjTrickBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickDelete;
            this.deleteObjTrickBtn.Location = new System.Drawing.Point(10, 134);
            this.deleteObjTrickBtn.Name = "deleteObjTrickBtn";
            this.deleteObjTrickBtn.Size = new System.Drawing.Size(24, 24);
            this.deleteObjTrickBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.deleteObjTrickBtn, "Delete Selected Trick...");
            this.deleteObjTrickBtn.UseVisualStyleBackColor = true;
            this.deleteObjTrickBtn.Click += new System.EventHandler(this.deleteObjTrickBtn_Click);
            // 
            // renameObjTrickBtn
            // 
            this.renameObjTrickBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickRename;
            this.renameObjTrickBtn.Location = new System.Drawing.Point(10, 108);
            this.renameObjTrickBtn.Name = "renameObjTrickBtn";
            this.renameObjTrickBtn.Size = new System.Drawing.Size(24, 24);
            this.renameObjTrickBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.renameObjTrickBtn, "Rename Selected Trick...");
            this.renameObjTrickBtn.UseVisualStyleBackColor = true;
            this.renameObjTrickBtn.Click += new System.EventHandler(this.renameObjTrickBtn_Click);
            // 
            // dupObjTrickBtn
            // 
            this.dupObjTrickBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickDuplicate;
            this.dupObjTrickBtn.Location = new System.Drawing.Point(10, 82);
            this.dupObjTrickBtn.Name = "dupObjTrickBtn";
            this.dupObjTrickBtn.Size = new System.Drawing.Size(24, 24);
            this.dupObjTrickBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.dupObjTrickBtn, "Duplicate Selected Trick...");
            this.dupObjTrickBtn.UseVisualStyleBackColor = true;
            this.dupObjTrickBtn.Click += new System.EventHandler(this.dupObjTrickBtn_Click);
            // 
            // newObjTrickBtn
            // 
            this.newObjTrickBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickNew;
            this.newObjTrickBtn.Location = new System.Drawing.Point(10, 56);
            this.newObjTrickBtn.Name = "newObjTrickBtn";
            this.newObjTrickBtn.Size = new System.Drawing.Size(24, 24);
            this.newObjTrickBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.newObjTrickBtn, "New Trick...");
            this.newObjTrickBtn.UseVisualStyleBackColor = true;
            this.newObjTrickBtn.Click += new System.EventHandler(this.newObjTrickBtn_Click);
            // 
            // objTrickListPanel
            // 
            this.objTrickListPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.objTrickListPanel.AutoScroll = true;
            this.objTrickListPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.objTrickListPanel.Location = new System.Drawing.Point(40, 56);
            this.objTrickListPanel.Name = "objTrickListPanel";
            this.objTrickListPanel.Size = new System.Drawing.Size(979, 100);
            this.objTrickListPanel.TabIndex = 3;
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
            this.groupBox1.Controls.Add(this.objListCmboBx);
            this.groupBox1.Location = new System.Drawing.Point(3, 3);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(428, 46);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Selected Trick";
            // 
            // objListCmboBx
            // 
            this.objListCmboBx.Dock = System.Windows.Forms.DockStyle.Fill;
            this.objListCmboBx.FormattingEnabled = true;
            this.objListCmboBx.Location = new System.Drawing.Point(3, 16);
            this.objListCmboBx.Name = "objListCmboBx";
            this.objListCmboBx.Size = new System.Drawing.Size(422, 21);
            this.objListCmboBx.TabIndex = 0;
            this.objListCmboBx.SelectionChangeCommitted += new System.EventHandler(this.matListCmboBx_SelectionChangeCommitted);
            // 
            // toolTip1
            // 
            this.toolTip1.ShowAlways = true;
            // 
            // ObjectBlockForm
            // 
            this.ClientSize = new System.Drawing.Size(1032, 480);
            this.Controls.Add(this.objTrickPropertiesPanel);
            this.Controls.Add(this.topPanel);
            this.Name = "ObjectBlockForm";
            this.topPanel.ResumeLayout(false);
            this.p4GrpBx.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.Panel objTrickPropertiesPanel;
        private System.Windows.Forms.Panel topPanel;
        private common.P4Panel p4Panel;
        private System.Windows.Forms.GroupBox p4GrpBx;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ComboBox objListCmboBx;
        private System.Windows.Forms.Panel objTrickListPanel;
        private System.Windows.Forms.Button newObjTrickBtn;
        private System.Windows.Forms.Button deleteObjTrickBtn;
        private System.Windows.Forms.Button renameObjTrickBtn;
        private System.Windows.Forms.Button dupObjTrickBtn;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}