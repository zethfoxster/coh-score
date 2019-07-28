namespace COH_CostumeUpdater.assetsMangement
{
    partial class ChkdOutFiles
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
            this.searchProgBar = new System.Windows.Forms.ProgressBar();
            this.cofListView = new System.Windows.Forms.ListView();
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.undoCheckoutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openFolderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openFileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.contextMenuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // searchProgBar
            // 
            this.searchProgBar.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.searchProgBar.Location = new System.Drawing.Point(0, 311);
            this.searchProgBar.Maximum = 1000;
            this.searchProgBar.Name = "searchProgBar";
            this.searchProgBar.Size = new System.Drawing.Size(880, 23);
            this.searchProgBar.Step = 1;
            this.searchProgBar.TabIndex = 0;
            // 
            // cofListView
            // 
            this.cofListView.Alignment = System.Windows.Forms.ListViewAlignment.SnapToGrid;
            this.cofListView.ContextMenuStrip = this.contextMenuStrip1;
            this.cofListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.cofListView.Location = new System.Drawing.Point(0, 0);
            this.cofListView.Name = "cofListView";
            this.cofListView.ShowItemToolTips = true;
            this.cofListView.Size = new System.Drawing.Size(880, 311);
            this.cofListView.TabIndex = 1;
            this.cofListView.UseCompatibleStateImageBehavior = false;
            this.cofListView.View = System.Windows.Forms.View.Details;
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            //this.undoCheckoutToolStripMenuItem,
            this.openFolderToolStripMenuItem,
            this.openFileToolStripMenuItem});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(157, 70);
            // 
            // undoCheckoutToolStripMenuItem
            // 
            this.undoCheckoutToolStripMenuItem.Name = "undoCheckoutToolStripMenuItem";
            this.undoCheckoutToolStripMenuItem.Size = new System.Drawing.Size(156, 22);
            this.undoCheckoutToolStripMenuItem.Text = "undo Checkout";
            this.undoCheckoutToolStripMenuItem.Click += new System.EventHandler(this.undoCheckoutToolStripMenuItem_Click);
            // 
            // openFolderToolStripMenuItem
            // 
            this.openFolderToolStripMenuItem.Name = "openFolderToolStripMenuItem";
            this.openFolderToolStripMenuItem.Size = new System.Drawing.Size(156, 22);
            this.openFolderToolStripMenuItem.Text = "Open Folder";
            this.openFolderToolStripMenuItem.Click += new System.EventHandler(this.openFolderToolStripMenuItem_Click);
            // 
            // openFileToolStripMenuItem
            // 
            this.openFileToolStripMenuItem.Name = "openFileToolStripMenuItem";
            this.openFileToolStripMenuItem.Size = new System.Drawing.Size(156, 22);
            this.openFileToolStripMenuItem.Text = "Open File";
            this.openFileToolStripMenuItem.Click += new System.EventHandler(this.openFileToolStripMenuItem_Click);
            // 
            // ChkdOutFiles
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(880, 334);
            this.Controls.Add(this.cofListView);
            this.Controls.Add(this.searchProgBar);
            this.Name = "ChkdOutFiles";
            this.Text = "Files Checked out by you";
            this.contextMenuStrip1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        public System.Collections.ArrayList checkedOutFiless;
        private System.Windows.Forms.ProgressBar searchProgBar;
        private System.Windows.Forms.ListView cofListView;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
        private System.Windows.Forms.ToolStripMenuItem undoCheckoutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openFolderToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openFileToolStripMenuItem;
        private System.Windows.Forms.RichTextBox logTxBox;
    }
}

