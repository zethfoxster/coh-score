namespace COH_CostumeUpdater.assetEditor.textureTrick
{
    partial class TextureBlockForm
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
            this.textureTrickPropertiesPanel = new System.Windows.Forms.Panel();
            this.topPanel = new System.Windows.Forms.Panel();
            this.deleteTextureTrickBtn = new System.Windows.Forms.Button();
            this.renameTextureTrickBtn = new System.Windows.Forms.Button();
            this.dupTextureTrickBtn = new System.Windows.Forms.Button();
            this.newTextureTrickBtn = new System.Windows.Forms.Button();
            this.textureTrickListPanel = new System.Windows.Forms.Panel();
            this.p4GrpBx = new System.Windows.Forms.GroupBox();
            this.p4Panel = new COH_CostumeUpdater.common.P4Panel();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.textureListCmboBx = new System.Windows.Forms.ComboBox();
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
            // textureTrickPropertiesPanel
            // 
            this.textureTrickPropertiesPanel.AutoScroll = true;
            this.textureTrickPropertiesPanel.BackColor = System.Drawing.SystemColors.Control;
            this.textureTrickPropertiesPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.textureTrickPropertiesPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textureTrickPropertiesPanel.Location = new System.Drawing.Point(0, 166);
            this.textureTrickPropertiesPanel.Name = "textureTrickPropertiesPanel";
            this.textureTrickPropertiesPanel.Size = new System.Drawing.Size(1032, 314);
            this.textureTrickPropertiesPanel.TabIndex = 0;
            // 
            // topPanel
            // 
            this.topPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.topPanel.Controls.Add(this.deleteTextureTrickBtn);
            this.topPanel.Controls.Add(this.renameTextureTrickBtn);
            this.topPanel.Controls.Add(this.dupTextureTrickBtn);
            this.topPanel.Controls.Add(this.newTextureTrickBtn);
            this.topPanel.Controls.Add(this.textureTrickListPanel);
            this.topPanel.Controls.Add(this.p4GrpBx);
            this.topPanel.Controls.Add(this.groupBox1);
            this.topPanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.topPanel.Location = new System.Drawing.Point(0, 0);
            this.topPanel.Name = "topPanel";
            this.topPanel.Size = new System.Drawing.Size(1032, 166);
            this.topPanel.TabIndex = 1;
            // 
            // deleteTextureTrickBtn
            // 
            this.deleteTextureTrickBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickDelete;
            this.deleteTextureTrickBtn.Location = new System.Drawing.Point(10, 134);
            this.deleteTextureTrickBtn.Name = "deleteTextureTrickBtn";
            this.deleteTextureTrickBtn.Size = new System.Drawing.Size(24, 24);
            this.deleteTextureTrickBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.deleteTextureTrickBtn, "Delete Selected Trick...");
            this.deleteTextureTrickBtn.UseVisualStyleBackColor = true;
            this.deleteTextureTrickBtn.Click += new System.EventHandler(this.deleteTextureTrickBtn_Click);
            // 
            // renameTextureTrickBtn
            // 
            this.renameTextureTrickBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickRename;
            this.renameTextureTrickBtn.Location = new System.Drawing.Point(10, 108);
            this.renameTextureTrickBtn.Name = "renameTextureTrickBtn";
            this.renameTextureTrickBtn.Size = new System.Drawing.Size(24, 24);
            this.renameTextureTrickBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.renameTextureTrickBtn, "Rename Selected Trick...");
            this.renameTextureTrickBtn.UseVisualStyleBackColor = true;
            this.renameTextureTrickBtn.Click += new System.EventHandler(this.renameTextureTrickBtn_Click);
            // 
            // dupTextureTrickBtn
            // 
            this.dupTextureTrickBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickDuplicate;
            this.dupTextureTrickBtn.Location = new System.Drawing.Point(10, 82);
            this.dupTextureTrickBtn.Name = "dupTextureTrickBtn";
            this.dupTextureTrickBtn.Size = new System.Drawing.Size(24, 24);
            this.dupTextureTrickBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.dupTextureTrickBtn, "Duplicate Selected Trick...");
            this.dupTextureTrickBtn.UseVisualStyleBackColor = true;
            this.dupTextureTrickBtn.Click += new System.EventHandler(this.dupTextureTrickBtn_Click);
            // 
            // newTextureTrickBtn
            // 
            this.newTextureTrickBtn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrickNew;
            this.newTextureTrickBtn.Location = new System.Drawing.Point(10, 56);
            this.newTextureTrickBtn.Name = "newTextureTrickBtn";
            this.newTextureTrickBtn.Size = new System.Drawing.Size(24, 24);
            this.newTextureTrickBtn.TabIndex = 4;
            this.toolTip1.SetToolTip(this.newTextureTrickBtn, "New Trick...");
            this.newTextureTrickBtn.UseVisualStyleBackColor = true;
            this.newTextureTrickBtn.Click += new System.EventHandler(this.newTextureTrickBtn_Click);
            // 
            // textureTrickListPanel
            // 
            this.textureTrickListPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textureTrickListPanel.AutoScroll = true;
            this.textureTrickListPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.textureTrickListPanel.Location = new System.Drawing.Point(40, 56);
            this.textureTrickListPanel.Name = "textureTrickListPanel";
            this.textureTrickListPanel.Size = new System.Drawing.Size(979, 100);
            this.textureTrickListPanel.TabIndex = 3;
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
            this.groupBox1.Controls.Add(this.textureListCmboBx);
            this.groupBox1.Location = new System.Drawing.Point(3, 3);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(428, 46);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Selected Trick";
            // 
            // textureListCmboBx
            // 
            this.textureListCmboBx.Dock = System.Windows.Forms.DockStyle.Fill;
            this.textureListCmboBx.FormattingEnabled = true;
            this.textureListCmboBx.Location = new System.Drawing.Point(3, 16);
            this.textureListCmboBx.Name = "textureListCmboBx";
            this.textureListCmboBx.Size = new System.Drawing.Size(422, 21);
            this.textureListCmboBx.TabIndex = 0;
            this.textureListCmboBx.SelectionChangeCommitted += new System.EventHandler(this.tTrickListCmboBx_SelectionChangeCommitted);
            // 
            // toolTip1
            // 
            this.toolTip1.ShowAlways = true;
            // 
            // TextureBlockForm
            // 
            this.ClientSize = new System.Drawing.Size(1032, 480);
            this.Controls.Add(this.textureTrickPropertiesPanel);
            this.Controls.Add(this.topPanel);
            this.Name = "TextureBlockForm";
            this.topPanel.ResumeLayout(false);
            this.p4GrpBx.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.Panel textureTrickPropertiesPanel;
        private System.Windows.Forms.Panel topPanel;
        private common.P4Panel p4Panel;
        private System.Windows.Forms.GroupBox p4GrpBx;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ComboBox textureListCmboBx;
        private System.Windows.Forms.Panel textureTrickListPanel;
        private System.Windows.Forms.Button newTextureTrickBtn;
        private System.Windows.Forms.Button deleteTextureTrickBtn;
        private System.Windows.Forms.Button renameTextureTrickBtn;
        private System.Windows.Forms.Button dupTextureTrickBtn;
        private System.Windows.Forms.ToolTip toolTip1;
    }
}