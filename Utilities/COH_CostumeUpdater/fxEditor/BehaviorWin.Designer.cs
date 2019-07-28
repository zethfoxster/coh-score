namespace COH_CostumeUpdater.fxEditor
{
    partial class BehaviorWin
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
            this.bhvr_tabControl = new System.Windows.Forms.TabControl();
            this.bhvrBasic_tabPage = new System.Windows.Forms.TabPage();
            this.bhvrColorAlpha_tabPage = new System.Windows.Forms.TabPage();
            this.bhvrPhysics_tabPage = new System.Windows.Forms.TabPage();
            this.bhvrShakeBlurLight_tabPage = new System.Windows.Forms.TabPage();
            this.bhvrAnimationSplat_tabPage = new System.Windows.Forms.TabPage();
            this.tabPage_ContextMenuStrip = new System.Windows.Forms.ContextMenuStrip();
            this.copyTabPageMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pasteTabPageMenuItem = new System.Windows.Forms.ToolStripMenuItem();

            this.bhvr_tabControl.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabPage_ContextMenuStrip
            // 
            this.tabPage_ContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            copyTabPageMenuItem,
            pasteTabPageMenuItem});
            this.tabPage_ContextMenuStrip.Name = "tabPage_ContextMenuStrip";
            this.tabPage_ContextMenuStrip.Size = new System.Drawing.Size(185, 114);
            // 
            // copyTabPageMenuItem
            // 
            this.copyTabPageMenuItem.Name = "copyTabPageMenuItem";
            this.copyTabPageMenuItem.Size = new System.Drawing.Size(184, 22);
            this.copyTabPageMenuItem.Text = "Copy Current Tab";
            this.copyTabPageMenuItem.Click += new System.EventHandler(this.copyTabPageMenuItem_Click);
            // 
            // pasteTabPageMenuItem
            // 
            this.pasteTabPageMenuItem.Name = "pasteTabPageMenuItem";
            this.pasteTabPageMenuItem.Size = new System.Drawing.Size(184, 22);
            this.pasteTabPageMenuItem.Text = "Paste on to Current Tab";
            this.pasteTabPageMenuItem.Enabled = false;
            this.pasteTabPageMenuItem.Click += new System.EventHandler(this.pasteTabPageMenuItem_Click);
            // 
            // bhvr_tabControl
            // 
            this.bhvr_tabControl.Controls.Add(this.bhvrBasic_tabPage);
            this.bhvr_tabControl.Controls.Add(this.bhvrColorAlpha_tabPage);
            this.bhvr_tabControl.Controls.Add(this.bhvrPhysics_tabPage);
            this.bhvr_tabControl.Controls.Add(this.bhvrShakeBlurLight_tabPage);
            this.bhvr_tabControl.Controls.Add(this.bhvrAnimationSplat_tabPage);
            this.bhvr_tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.bhvr_tabControl.ItemSize = new System.Drawing.Size(42, 18);
            this.bhvr_tabControl.Location = new System.Drawing.Point(0, 0);
            this.bhvr_tabControl.Multiline = true;
            this.bhvr_tabControl.Name = "bhvr_tabControl";
            this.bhvr_tabControl.SelectedIndex = 0;
            this.bhvr_tabControl.Size = new System.Drawing.Size(564, 267);
            this.bhvr_tabControl.TabIndex = 0;
            this.bhvr_tabControl.ContextMenuStrip = this.tabPage_ContextMenuStrip;
            this.bhvr_tabControl.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.bhvr_tabControl_MouseWheel);
            // 
            // bhvrBasic_tabPage
            // 
            this.bhvrBasic_tabPage.AutoScroll = true;
            this.bhvrBasic_tabPage.Location = new System.Drawing.Point(4, 22);
            this.bhvrBasic_tabPage.Name = "bhvrBasic_tabPage";
            this.bhvrBasic_tabPage.Size = new System.Drawing.Size(556, 241);
            this.bhvrBasic_tabPage.TabIndex = 0;
            this.bhvrBasic_tabPage.Text = "Basic";
            this.bhvrBasic_tabPage.UseVisualStyleBackColor = true;
            // 
            // bhvrColorAlpha_tabPage
            // 
            this.bhvrColorAlpha_tabPage.AutoScroll = true;
            this.bhvrColorAlpha_tabPage.Location = new System.Drawing.Point(4, 22);
            this.bhvrColorAlpha_tabPage.Name = "bhvrColorAlpha_tabPage";
            this.bhvrColorAlpha_tabPage.Size = new System.Drawing.Size(556, 193);
            this.bhvrColorAlpha_tabPage.TabIndex = 1;
            this.bhvrColorAlpha_tabPage.Text = "Color/Alpha";
            this.bhvrColorAlpha_tabPage.UseVisualStyleBackColor = true;
            // 
            // bhvrPhysics_tabPage
            // 
            this.bhvrPhysics_tabPage.AutoScroll = true;
            this.bhvrPhysics_tabPage.Location = new System.Drawing.Point(4, 22);
            this.bhvrPhysics_tabPage.Name = "bhvrPhysics_tabPage";
            this.bhvrPhysics_tabPage.Size = new System.Drawing.Size(556, 193);
            this.bhvrPhysics_tabPage.TabIndex = 2;
            this.bhvrPhysics_tabPage.Text = "Physics";
            this.bhvrPhysics_tabPage.UseVisualStyleBackColor = true;
            // 
            // bhvrShakeBlurLight_tabPage
            // 
            this.bhvrShakeBlurLight_tabPage.AutoScroll = true;
            this.bhvrShakeBlurLight_tabPage.Location = new System.Drawing.Point(4, 22);
            this.bhvrShakeBlurLight_tabPage.Name = "bhvrShakeBlurLight_tabPage";
            this.bhvrShakeBlurLight_tabPage.Size = new System.Drawing.Size(556, 193);
            this.bhvrShakeBlurLight_tabPage.TabIndex = 3;
            this.bhvrShakeBlurLight_tabPage.Text = "Shake/Blur/Light";
            this.bhvrShakeBlurLight_tabPage.UseVisualStyleBackColor = true;
            // 
            // bhvrAnimationSplat_tabPage
            // 
            this.bhvrAnimationSplat_tabPage.AutoScroll = true;
            this.bhvrAnimationSplat_tabPage.Location = new System.Drawing.Point(4, 22);
            this.bhvrAnimationSplat_tabPage.Name = "bhvrAnimationSplat_tabPage";
            this.bhvrAnimationSplat_tabPage.Size = new System.Drawing.Size(556, 193);
            this.bhvrAnimationSplat_tabPage.TabIndex = 4;
            this.bhvrAnimationSplat_tabPage.Text = "Animation/Splat";
            this.bhvrAnimationSplat_tabPage.UseVisualStyleBackColor = true;
            // 
            // BehaviorWin
            // 
            this.ClientSize = new System.Drawing.Size(564, 267);
            this.Controls.Add(this.bhvr_tabControl);
            this.Name = "BehaviorWin";
            this.bhvr_tabControl.ResumeLayout(false);
            this.ResumeLayout(false);

        }




        #endregion

        private System.Windows.Forms.TabControl bhvr_tabControl;
        private System.Windows.Forms.TabPage bhvrBasic_tabPage;
        private System.Windows.Forms.TabPage bhvrColorAlpha_tabPage;
        private System.Windows.Forms.TabPage bhvrPhysics_tabPage;
        private System.Windows.Forms.TabPage bhvrShakeBlurLight_tabPage;
        private System.Windows.Forms.TabPage bhvrAnimationSplat_tabPage;

        private System.Windows.Forms.ContextMenuStrip  tabPage_ContextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem copyTabPageMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pasteTabPageMenuItem;
    }
}