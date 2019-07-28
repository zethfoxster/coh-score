namespace COH_CostumeUpdater
{
    partial class COH_CostumeUpdaterForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(COH_CostumeUpdaterForm));
            this.cohTabControl = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.CostumeSplitContainer = new System.Windows.Forms.SplitContainer();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.ProjectsLable = new System.Windows.Forms.Label();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newFXToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newAssetsTrickToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadCoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadAssetsTrickToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadFXToolStripMenueItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadPartToolStripMenueItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadRegionCTM_MenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveLoadedFile = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsLoadedFile = new System.Windows.Forms.ToolStripMenuItem();
            this.showP4CheckedOutFiles = new System.Windows.Forms.ToolStripMenuItem();
            this.viewTextureFileItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.logTxBx = new System.Windows.Forms.RichTextBox();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this.revisionLable = new System.Windows.Forms.Label();
            this.cohTabControl.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.CostumeSplitContainer.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // cohTabControl
            // 
            this.cohTabControl.Controls.Add(this.tabPage1);
            this.cohTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.cohTabControl.Location = new System.Drawing.Point(0, 24);
            this.cohTabControl.Name = "cohTabControl";
            this.cohTabControl.SelectedIndex = 0;
            this.cohTabControl.Size = new System.Drawing.Size(1550, 522);
            this.cohTabControl.TabIndex = 3;
            this.cohTabControl.Selected += new System.Windows.Forms.TabControlEventHandler(this.cohTabControl_Selected);
            // 
            // tabPage1
            // 
            this.tabPage1.CausesValidation = false;
            this.tabPage1.Controls.Add(this.CostumeSplitContainer);
            this.tabPage1.Location = new System.Drawing.Point(4, 22);
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage1.Size = new System.Drawing.Size(1542, 496);
            this.tabPage1.TabIndex = 0;
            this.tabPage1.Text = "Tab Page 1";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // CostumeSplitContainer
            // 
            this.CostumeSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.CostumeSplitContainer.Location = new System.Drawing.Point(3, 3);
            this.CostumeSplitContainer.Name = "CostumeSplitContainer";
            // 
            // CostumeSplitContainer.Panel2
            // 
            this.CostumeSplitContainer.Panel2.BackColor = System.Drawing.SystemColors.Control;
            this.CostumeSplitContainer.Size = new System.Drawing.Size(1536, 490);
            this.CostumeSplitContainer.SplitterDistance = 292;
            this.CostumeSplitContainer.TabIndex = 1;
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.splitContainer1);
            this.tabPage2.Location = new System.Drawing.Point(4, 22);
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.Padding = new System.Windows.Forms.Padding(3);
            this.tabPage2.Size = new System.Drawing.Size(1292, 496);
            this.tabPage2.TabIndex = 1;
            this.tabPage2.Text = "Tab Page2";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(3, 3);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.BackColor = System.Drawing.SystemColors.Control;
            this.splitContainer1.Size = new System.Drawing.Size(1286, 490);
            this.splitContainer1.SplitterDistance = 245;
            this.splitContainer1.TabIndex = 0;
            // 
            // ProjectsLable
            // 
            this.ProjectsLable.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.ProjectsLable.Dock = System.Windows.Forms.DockStyle.Top;
            this.ProjectsLable.Location = new System.Drawing.Point(0, 0);
            this.ProjectsLable.Name = "ProjectsLable";
            this.ProjectsLable.Size = new System.Drawing.Size(245, 27);
            this.ProjectsLable.TabIndex = 1;
            this.ProjectsLable.Text = "Projects";
            this.ProjectsLable.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // menuStrip1
            // 
            this.menuStrip1.BackColor = System.Drawing.SystemColors.Control;
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(1550, 24);
            this.menuStrip1.TabIndex = 2;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newToolStripMenuItem,
            this.loadToolStripMenuItem,
            this.saveLoadedFile,
            this.saveAsLoadedFile,
            this.showP4CheckedOutFiles,
            this.viewTextureFileItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // newToolStripMenuItem
            // 
            this.newToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newFXToolStripMenuItem,
            this.newAssetsTrickToolStripMenuItem});
            this.newToolStripMenuItem.Name = "newToolStripMenuItem";
            this.newToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.newToolStripMenuItem.Text = "New";
            this.newToolStripMenuItem.DropDownOpened += new System.EventHandler(this.newToolStripMenuItem_Click);
            // 
            // newFXToolStripMenuItem
            // 
            this.newFXToolStripMenuItem.Name = "newFXToolStripMenuItem";
            this.newFXToolStripMenuItem.Size = new System.Drawing.Size(157, 22);
            this.newFXToolStripMenuItem.Text = "FX file";
            this.newFXToolStripMenuItem.Click += new System.EventHandler(this.newFXToolStripMenuItem_Click);
            // 
            // newAssetsTrickToolStripMenuItem
            // 
            this.newAssetsTrickToolStripMenuItem.Name = "newAssetsTrickToolStripMenuItem";
            this.newAssetsTrickToolStripMenuItem.Size = new System.Drawing.Size(157, 22);
            this.newAssetsTrickToolStripMenuItem.Text = "Assets Trick File";
            this.newAssetsTrickToolStripMenuItem.Click += new System.EventHandler(this.newAssetsTrickToolStripMenuItem_Click);
            // 
            // loadToolStripMenuItem
            // 
            this.loadToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            //this.loadCoToolStripMenuItem,
            this.loadAssetsTrickToolStripMenuItem,
            this.loadFXToolStripMenueItem,
            this.loadPartToolStripMenueItem,
            this.loadRegionCTM_MenuItem});
            this.loadToolStripMenuItem.Name = "loadToolStripMenuItem";
            this.loadToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.loadToolStripMenuItem.Text = "Load";
            this.loadToolStripMenuItem.DropDownOpened += new System.EventHandler(this.loadToolStripMenuItem_Click);
            // 
            // loadCoToolStripMenuItem
            // 
            this.loadCoToolStripMenuItem.Name = "loadCoToolStripMenuItem";
            this.loadCoToolStripMenuItem.Size = new System.Drawing.Size(145, 22);
            this.loadCoToolStripMenuItem.Text = "Costumes";
            this.loadCoToolStripMenuItem.Click += new System.EventHandler(this.loadCoToolStripMenuItem_Click);
            // 
            // loadAssetsTrickToolStripMenuItem
            // 
            this.loadAssetsTrickToolStripMenuItem.Name = "loadAssetsTrickToolStripMenuItem";
            this.loadAssetsTrickToolStripMenuItem.Size = new System.Drawing.Size(145, 22);
            this.loadAssetsTrickToolStripMenuItem.Text = "Assets Trick";
            this.loadAssetsTrickToolStripMenuItem.Click += new System.EventHandler(this.loadObjectTrickToolStripMenuItem_Click);
            // 
            // loadFXToolStripMenueItem
            // 
            this.loadFXToolStripMenueItem.Name = "loadFXToolStripMenueItem";
            this.loadFXToolStripMenueItem.Size = new System.Drawing.Size(145, 22);
            this.loadFXToolStripMenueItem.Text = "FX";
            this.loadFXToolStripMenueItem.Click += new System.EventHandler(this.loadFXToolStripMenueItem_Click);
            // 
            // loadPartToolStripMenueItem
            // 
            this.loadPartToolStripMenueItem.Name = "loadPartToolStripMenueItem";
            this.loadPartToolStripMenueItem.Size = new System.Drawing.Size(145, 22);
            this.loadPartToolStripMenueItem.Text = "PART";
            this.loadPartToolStripMenueItem.Click += new System.EventHandler(this.loadPartToolStripMenueItem_Click);
            // 
            // loadRegionCTM_MenuItem
            // 
            this.loadRegionCTM_MenuItem.Name = "loadRegionCTM_MenuItem";
            this.loadRegionCTM_MenuItem.Size = new System.Drawing.Size(145, 22);
            this.loadRegionCTM_MenuItem.Text = "Regions CTM";
            this.loadRegionCTM_MenuItem.Click += new System.EventHandler(this.loadRegionCTM_MenuItem_Click);
            // 
            // saveLoadedFile
            // 
            this.saveLoadedFile.Name = "saveLoadedFile";
            this.saveLoadedFile.Size = new System.Drawing.Size(199, 22);
            this.saveLoadedFile.Text = "Save";
            this.saveLoadedFile.Click += new System.EventHandler(this.saveLoadedFile_Click);
            // 
            // saveAsLoadedFile
            // 
            this.saveAsLoadedFile.Name = "saveAsLoadedFile";
            this.saveAsLoadedFile.Size = new System.Drawing.Size(199, 22);
            this.saveAsLoadedFile.Text = "Save As";
            this.saveAsLoadedFile.Click += new System.EventHandler(this.saveAsLoadedFile_Click);
            // 
            // showP4CheckedOutFiles
            // 
            this.showP4CheckedOutFiles.Name = "showP4CheckedOutFiles";
            this.showP4CheckedOutFiles.Size = new System.Drawing.Size(199, 22);
            this.showP4CheckedOutFiles.Text = "Show Checked out Files";
            this.showP4CheckedOutFiles.Click += new System.EventHandler(this.showReadOnlyFilesToolStripMenuItem_Click);
            // 
            // viewTextureFileItem
            // 
            this.viewTextureFileItem.Name = "viewTextureFileItem";
            this.viewTextureFileItem.Size = new System.Drawing.Size(199, 22);
            this.viewTextureFileItem.Text = "View Texture File";
            this.viewTextureFileItem.Click += new System.EventHandler(this.viewTextureFile);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.DefaultExt = "ctm";
            this.openFileDialog1.FileName = "openFileDialog1";
            this.openFileDialog1.Filter = "CTM document (*.ctm)|*.ctm";
            // 
            // logTxBx
            // 
            this.logTxBx.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.logTxBx.Location = new System.Drawing.Point(0, 546);
            this.logTxBx.Name = "logTxBx";
            this.logTxBx.Size = new System.Drawing.Size(1550, 123);
            this.logTxBx.TabIndex = 4;
            this.logTxBx.Text = "";
            // 
            // revisionLable
            // 
            this.revisionLable.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.revisionLable.AutoSize = true;
            this.revisionLable.Location = new System.Drawing.Point(1450, 11);
            this.revisionLable.Name = "revisionLable";
            this.revisionLable.Size = new System.Drawing.Size(35, 13);
            this.revisionLable.TabIndex = 5;
            this.revisionLable.Text = "label1";
            // 
            // COH_CostumeUpdaterForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1550, 669);
            this.Controls.Add(this.revisionLable);
            this.Controls.Add(this.cohTabControl);
            this.Controls.Add(this.logTxBx);
            this.Controls.Add(this.menuStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.KeyPreview = true;
            this.Name = "COH_CostumeUpdaterForm";
            this.Text = "COH Assets Editor";
            this.Load += new System.EventHandler(this.COH_CostumeUpdaterForm_Load);
            this.KeyUp += new System.Windows.Forms.KeyEventHandler(this.COH_CostumeUpdaterForm_KeyUp);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.COH_CostumeUpdaterForm_FormClosing);
            this.cohTabControl.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.CostumeSplitContainer.ResumeLayout(false);
            this.tabPage2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private COH_CostumeUpdater.assetsMangement.ChkdOutFiles ckoFiles;
        private COH_CostumeUpdater.costume.CostumeEditor cEditor;
        private string loadedObject;
        private System.Windows.Forms.SplitContainer CostumeSplitContainer;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;

        private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem newFXToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem newAssetsTrickToolStripMenuItem;

        private System.Windows.Forms.ToolStripMenuItem loadRegionCTM_MenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadCoToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveLoadedFile;
        private System.Windows.Forms.ToolStripMenuItem saveAsLoadedFile;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private System.Windows.Forms.TabControl cohTabControl;
        private System.Windows.Forms.TabPage tabPage1;
        private System.Windows.Forms.TabPage tabPage2;
        private System.Windows.Forms.Label ProjectsLable;
        //private System.Windows.Forms.ListView ProjectsListView;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.ToolStripMenuItem showP4CheckedOutFiles;
        private System.Windows.Forms.ToolStripMenuItem viewTextureFileItem;
        private System.Windows.Forms.RichTextBox logTxBx;
        private System.Windows.Forms.ToolStripMenuItem loadAssetsTrickToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadFXToolStripMenueItem;
        private System.Windows.Forms.ToolStripMenuItem loadPartToolStripMenueItem;
        private System.Windows.Forms.ToolStripMenuItem loadToolStripMenuItem;
        private System.Windows.Forms.ToolTip toolTip1;
        private System.Windows.Forms.Label revisionLable;

    }
}

