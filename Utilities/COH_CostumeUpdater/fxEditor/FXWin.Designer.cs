namespace COH_CostumeUpdater.fxEditor
{
    partial class FXWin
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FXWin));
            this.fx_splitContainer = new System.Windows.Forms.SplitContainer();
            this.fx_tv = new FxLauncher.TreeViewMultiSelect();
            this.fx_tv_ContextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.sendToUltraEditToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openInExplorerMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.copyMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pasteMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.copyBhvrOvrMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pasteBhvrOvrMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.moveUpMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.moveDwnMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addConditionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addEventToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addBhvrToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addPartToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addSoundToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addSplatToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addInputToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.addBhvrOvrMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.renameMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteSplatMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fx_tabControl = new System.Windows.Forms.TabControl();
            this.fx_tabPage = new System.Windows.Forms.TabPage();
            this.fxTags_panel = new System.Windows.Forms.Panel();
            this.fxInput_panel = new System.Windows.Forms.Panel();
            this.fxClampMinMax_panel = new System.Windows.Forms.Panel();
            this.fxClampMaxScaleX_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxClampMinScaleX_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxClampMaxScale_ckbx = new System.Windows.Forms.CheckBox();
            this.fxClampMaxScaleZ_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxClampMaxScaleY_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxClampMinScale_ckbx = new System.Windows.Forms.CheckBox();
            this.fxClampMinScaleZ_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxClampMinScaleY_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxFileAgePlus_panel = new System.Windows.Forms.Panel();
            this.fxPerformanceRadius_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxFileAge_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxOnForceRadius_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxPerformanceRadius_ckbx = new System.Windows.Forms.CheckBox();
            this.fxOnForceRadius_ckbx = new System.Windows.Forms.CheckBox();
            this.fxFileAge_ckbx = new System.Windows.Forms.CheckBox();
            this.fxLifSpanPlus_panel = new System.Windows.Forms.Panel();
            this.fxLighting_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxLighting_ckbx = new System.Windows.Forms.CheckBox();
            this.fxAnimScale_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxLifeSpan_numBx = new System.Windows.Forms.NumericUpDown();
            this.fxAnimScale_ckbx = new System.Windows.Forms.CheckBox();
            this.fxLifeSpan_ckbx = new System.Windows.Forms.CheckBox();
            this.fxName_panel = new System.Windows.Forms.Panel();
            this.fxName_label = new System.Windows.Forms.Label();
            this.fxName_txbx = new System.Windows.Forms.TextBox();
            this.fxFlags_gpbx = new System.Windows.Forms.GroupBox();
            this.NotCompatibleWithAnimalHead_ckbx = new System.Windows.Forms.CheckBox();
            this.SoundOnly_ckbx = new System.Windows.Forms.CheckBox();
            this.DontInheritTexFromCostume_ckbx = new System.Windows.Forms.CheckBox();
            this.InheritGeoScale_ckbx = new System.Windows.Forms.CheckBox();
            this.DontSuppress_ckbx = new System.Windows.Forms.CheckBox();
            this.IsWeapon_ckbx = new System.Windows.Forms.CheckBox();
            this.DontInheritBits_ckbx = new System.Windows.Forms.CheckBox();
            this.IsShield_ckbx = new System.Windows.Forms.CheckBox();
            this.InheritAnimScale_ckbx = new System.Windows.Forms.CheckBox();
            this.UseSkinTint_ckbx = new System.Windows.Forms.CheckBox();
            this.IsArmor_ckbx = new System.Windows.Forms.CheckBox();
            this.InheritAlpha_ckbx = new System.Windows.Forms.CheckBox();
            this.conditions_tabPage = new System.Windows.Forms.TabPage();
            this.mConditionWin = new COH_CostumeUpdater.fxEditor.ConditionWin();
            this.events_tabPage = new System.Windows.Forms.TabPage();
            this.mEventWin = new COH_CostumeUpdater.fxEditor.EventWin();
            this.sound_tabPage = new System.Windows.Forms.TabPage();
            this.splat_tabPage = new System.Windows.Forms.TabPage();
            this.fxOption_panel = new System.Windows.Forms.Panel();
            this.saveConfig_btn = new System.Windows.Forms.Button();
            this.filter_btn = new System.Windows.Forms.Button();
            this.p4Add_btn = new System.Windows.Forms.Button();
            this.p4CheckOut_btn = new System.Windows.Forms.Button();
            this.p4CheckIn_btn = new System.Windows.Forms.Button();
            this.p4DepotDiff_btn = new System.Windows.Forms.Button();
            this.p4_Label = new System.Windows.Forms.Label();
            this.findGame_btn = new System.Windows.Forms.Button();
            this.fx_splitContainer.Panel1.SuspendLayout();
            this.fx_splitContainer.Panel2.SuspendLayout();
            this.fx_splitContainer.SuspendLayout();
            this.fx_tv_ContextMenuStrip.SuspendLayout();
            this.fx_tabControl.SuspendLayout();
            this.fx_tabPage.SuspendLayout();
            this.fxTags_panel.SuspendLayout();
            this.fxClampMinMax_panel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMaxScaleX_numBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMinScaleX_numBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMaxScaleZ_numBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMaxScaleY_numBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMinScaleZ_numBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMinScaleY_numBx)).BeginInit();
            this.fxFileAgePlus_panel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.fxPerformanceRadius_numBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxFileAge_numBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxOnForceRadius_numBx)).BeginInit();
            this.fxLifSpanPlus_panel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.fxLighting_numBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxAnimScale_numBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxLifeSpan_numBx)).BeginInit();
            this.fxName_panel.SuspendLayout();
            this.fxFlags_gpbx.SuspendLayout();
            this.conditions_tabPage.SuspendLayout();
            this.events_tabPage.SuspendLayout();
            this.fxOption_panel.SuspendLayout();
            this.SuspendLayout();
            // 
            // fx_splitContainer
            // 
            this.fx_splitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.fx_splitContainer.Location = new System.Drawing.Point(0, 0);
            this.fx_splitContainer.Name = "fx_splitContainer";
            // 
            // fx_splitContainer.Panel1
            // 
            this.fx_splitContainer.Panel1.Controls.Add(this.fx_tv);
            // 
            // fx_splitContainer.Panel2
            // 
            this.fx_splitContainer.Panel2.AutoScroll = true;
            this.fx_splitContainer.Panel2.Controls.Add(this.fx_tabControl);
            this.fx_splitContainer.Panel2.Controls.Add(this.fxOption_panel);
            this.fx_splitContainer.Size = new System.Drawing.Size(972, 381);
            this.fx_splitContainer.SplitterDistance = 185;
            this.fx_splitContainer.TabIndex = 0;
            // 
            // fx_tv
            // 
            this.fx_tv.ContextMenuStrip = this.fx_tv_ContextMenuStrip;
            this.fx_tv.Dock = System.Windows.Forms.DockStyle.Fill;
            this.fx_tv.Location = new System.Drawing.Point(0, 0);
            this.fx_tv.Name = "fx_tv";
            this.fx_tv.SelectedNodes = ((System.Collections.ArrayList)(resources.GetObject("fx_tv.SelectedNodes")));
            this.fx_tv.ShowNodeToolTips = true;
            this.fx_tv.Size = new System.Drawing.Size(185, 381);
            this.fx_tv.TabIndex = 0;
            // 
            // fx_tv_ContextMenuStrip
            // 
            this.fx_tv_ContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.sendToUltraEditToolStripMenuItem,
            this.openInExplorerMenuItem,
            this.copyMenuItem,
            this.pasteMenuItem,
            this.copyBhvrOvrMenuItem,
            this.pasteBhvrOvrMenuItem,
            this.moveUpMenuItem,
            this.moveDwnMenuItem,
            this.addMenuItem,
            this.renameMenuItem,
            this.deleteMenuItem,
            this.deleteSplatMenuItem});
            this.fx_tv_ContextMenuStrip.Name = "fx_tv_ContextMenuStrip";
            this.fx_tv_ContextMenuStrip.Size = new System.Drawing.Size(209, 268);
            // 
            // sendToUltraEditToolStripMenuItem
            // 
            this.sendToUltraEditToolStripMenuItem.Name = "sendToUltraEditToolStripMenuItem";
            this.sendToUltraEditToolStripMenuItem.Size = new System.Drawing.Size(208, 22);
            this.sendToUltraEditToolStripMenuItem.Text = "send to UltraEdit";
            this.sendToUltraEditToolStripMenuItem.Click += new System.EventHandler(this.sendToUltraEditToolStripMenuItem_Click);
            // 
            // openInExplorerMenuItem
            // 
            this.openInExplorerMenuItem.Name = "openInExplorerMenuItem";
            this.openInExplorerMenuItem.Size = new System.Drawing.Size(208, 22);
            this.openInExplorerMenuItem.Text = "Open in Window Explorer";
            this.openInExplorerMenuItem.Click += new System.EventHandler(this.openInExplorerMenuItem_Click);
            // 
            // copyMenuItem
            // 
            this.copyMenuItem.Name = "copyMenuItem";
            this.copyMenuItem.Size = new System.Drawing.Size(208, 22);
            this.copyMenuItem.Text = "Copy Selected";
            this.copyMenuItem.Click += new System.EventHandler(this.copyMenuItem_Click);
            // 
            // pasteMenuItem
            // 
            this.pasteMenuItem.Name = "pasteMenuItem";
            this.pasteMenuItem.Size = new System.Drawing.Size(208, 22);
            this.pasteMenuItem.Text = "Paste Below Selected";
            this.pasteMenuItem.Click += new System.EventHandler(this.pasteMenuItem_Click);
            // 
            // copyBhvrOvrMenuItem
            // 
            this.copyBhvrOvrMenuItem.Name = "copyBhvrOvrMenuItem";
            this.copyBhvrOvrMenuItem.Size = new System.Drawing.Size(208, 22);
            this.copyBhvrOvrMenuItem.Text = "Copy Bhvroverride";
            this.copyBhvrOvrMenuItem.Click += new System.EventHandler(this.copyMenuItem_Click);
            // 
            // pasteBhvrOvrMenuItem
            // 
            this.pasteBhvrOvrMenuItem.Name = "pasteBhvrOvrMenuItem";
            this.pasteBhvrOvrMenuItem.Size = new System.Drawing.Size(208, 22);
            this.pasteBhvrOvrMenuItem.Text = "Paste Bhvroverride";
            this.pasteBhvrOvrMenuItem.Click += new System.EventHandler(this.pasteBhvrOvrMenuItem_Click);
            // 
            // moveUpMenuItem
            // 
            this.moveUpMenuItem.Name = "moveUpMenuItem";
            this.moveUpMenuItem.Size = new System.Drawing.Size(208, 22);
            this.moveUpMenuItem.Text = "Move Selected Up";
            this.moveUpMenuItem.Click += new System.EventHandler(this.moveUpMenuItem_Click);
            // 
            // moveDwnMenuItem
            // 
            this.moveDwnMenuItem.Name = "moveDwnMenuItem";
            this.moveDwnMenuItem.Size = new System.Drawing.Size(208, 22);
            this.moveDwnMenuItem.Text = "Move Selected Down";
            this.moveDwnMenuItem.Click += new System.EventHandler(this.moveDwnMenuItem_Click);
            // 
            // addMenuItem
            // 
            this.addMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.addConditionToolStripMenuItem,
            this.addEventToolStripMenuItem,
            this.addBhvrToolStripMenuItem,
            this.addPartToolStripMenuItem,
            this.addSoundToolStripMenuItem,
            this.addSplatToolStripMenuItem,
            this.addInputToolStripMenuItem,
            this.addBhvrOvrMenuItem});
            this.addMenuItem.Name = "addMenuItem";
            this.addMenuItem.Size = new System.Drawing.Size(208, 22);
            this.addMenuItem.Text = "Add";
            // 
            // addConditionToolStripMenuItem
            // 
            this.addConditionToolStripMenuItem.Name = "addConditionToolStripMenuItem";
            this.addConditionToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.addConditionToolStripMenuItem.Text = "Condition";
            this.addConditionToolStripMenuItem.Click += new System.EventHandler(this.addConditionToolStripMenuItem_Click);
            // 
            // addEventToolStripMenuItem
            // 
            this.addEventToolStripMenuItem.Name = "addEventToolStripMenuItem";
            this.addEventToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.addEventToolStripMenuItem.Text = "Event";
            this.addEventToolStripMenuItem.Click += new System.EventHandler(this.addEventToolStripMenuItem_Click);
            // 
            // addBhvrToolStripMenuItem
            // 
            this.addBhvrToolStripMenuItem.Name = "addBhvrToolStripMenuItem";
            this.addBhvrToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.addBhvrToolStripMenuItem.Text = "Bhvr";
            this.addBhvrToolStripMenuItem.Click += new System.EventHandler(this.addBhvrToolStripMenuItem_Click);
            // 
            // addPartToolStripMenuItem
            // 
            this.addPartToolStripMenuItem.Name = "addPartToolStripMenuItem";
            this.addPartToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.addPartToolStripMenuItem.Text = "Part";
            this.addPartToolStripMenuItem.Click += new System.EventHandler(this.addPartToolStripMenuItem_Click);
            // 
            // addSoundToolStripMenuItem
            // 
            this.addSoundToolStripMenuItem.Name = "addSoundToolStripMenuItem";
            this.addSoundToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.addSoundToolStripMenuItem.Text = "Sound";
            this.addSoundToolStripMenuItem.Click += new System.EventHandler(this.addSoundToolStripMenuItem_Click);
            // 
            // addSplatToolStripMenuItem
            // 
            this.addSplatToolStripMenuItem.Name = "addSplatToolStripMenuItem";
            this.addSplatToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.addSplatToolStripMenuItem.Text = "Splat";
            this.addSplatToolStripMenuItem.Click += new System.EventHandler(this.addSplatToolStripMenuItem_Click);
            // 
            // addInputToolStripMenuItem
            // 
            this.addInputToolStripMenuItem.Name = "addInputToolStripMenuItem";
            this.addInputToolStripMenuItem.Size = new System.Drawing.Size(194, 22);
            this.addInputToolStripMenuItem.Text = "Input";
            this.addInputToolStripMenuItem.Click += new System.EventHandler(this.addInputToolStripMenuItem_Click);
            // 
            // addBhvrOvrMenuItem
            // 
            this.addBhvrOvrMenuItem.Name = "addBhvrOvrMenuItem";
            this.addBhvrOvrMenuItem.Size = new System.Drawing.Size(194, 22);
            this.addBhvrOvrMenuItem.Text = "Add Bhvroverride Only";
            this.addBhvrOvrMenuItem.Click += new System.EventHandler(this.addBhvrOvrMenuItem_Click);
            // 
            // renameMenuItem
            // 
            this.renameMenuItem.Name = "renameMenuItem";
            this.renameMenuItem.Size = new System.Drawing.Size(208, 22);
            this.renameMenuItem.Text = "Rename Part File";
            this.renameMenuItem.Click += new System.EventHandler(this.renameMenuItem_Click);
            // 
            // deleteMenuItem
            // 
            this.deleteMenuItem.Name = "deleteMenuItem";
            this.deleteMenuItem.Size = new System.Drawing.Size(208, 22);
            this.deleteMenuItem.Text = "Delete Selected";
            this.deleteMenuItem.Click += new System.EventHandler(this.deleteMenuItem_Click);
            // 
            // deleteSplatMenuItem
            // 
            this.deleteSplatMenuItem.Name = "deleteSplatMenuItem";
            this.deleteSplatMenuItem.Size = new System.Drawing.Size(208, 22);
            this.deleteSplatMenuItem.Text = "Delete Event Splat";
            this.deleteSplatMenuItem.Click += new System.EventHandler(this.deleteSplatMenuItem_Click);
            // 
            // fx_tabControl
            // 
            this.fx_tabControl.Controls.Add(this.fx_tabPage);
            this.fx_tabControl.Controls.Add(this.conditions_tabPage);
            this.fx_tabControl.Controls.Add(this.events_tabPage);
            this.fx_tabControl.Controls.Add(this.sound_tabPage);
            this.fx_tabControl.Controls.Add(this.splat_tabPage);
            this.fx_tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.fx_tabControl.Location = new System.Drawing.Point(0, 28);
            this.fx_tabControl.Name = "fx_tabControl";
            this.fx_tabControl.SelectedIndex = 0;
            this.fx_tabControl.Size = new System.Drawing.Size(783, 353);
            this.fx_tabControl.TabIndex = 1;
            // 
            // fx_tabPage
            // 
            this.fx_tabPage.Controls.Add(this.fxTags_panel);
            this.fx_tabPage.Controls.Add(this.fxFlags_gpbx);
            this.fx_tabPage.Location = new System.Drawing.Point(4, 22);
            this.fx_tabPage.Name = "fx_tabPage";
            this.fx_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.fx_tabPage.Size = new System.Drawing.Size(775, 327);
            this.fx_tabPage.TabIndex = 0;
            this.fx_tabPage.Text = "FX:";
            this.fx_tabPage.UseVisualStyleBackColor = true;
            // 
            // fxTags_panel
            // 
            this.fxTags_panel.AutoScroll = true;
            this.fxTags_panel.Controls.Add(this.fxInput_panel);
            this.fxTags_panel.Controls.Add(this.fxClampMinMax_panel);
            this.fxTags_panel.Controls.Add(this.fxFileAgePlus_panel);
            this.fxTags_panel.Controls.Add(this.fxLifSpanPlus_panel);
            this.fxTags_panel.Controls.Add(this.fxName_panel);
            this.fxTags_panel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.fxTags_panel.Location = new System.Drawing.Point(3, 73);
            this.fxTags_panel.Name = "fxTags_panel";
            this.fxTags_panel.Size = new System.Drawing.Size(769, 251);
            this.fxTags_panel.TabIndex = 1;
            // 
            // fxInput_panel
            // 
            this.fxInput_panel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.fxInput_panel.AutoScroll = true;
            this.fxInput_panel.BackColor = System.Drawing.SystemColors.Control;
            this.fxInput_panel.Location = new System.Drawing.Point(0, 108);
            this.fxInput_panel.Name = "fxInput_panel";
            this.fxInput_panel.Size = new System.Drawing.Size(666, 300);
            this.fxInput_panel.TabIndex = 0;
            // 
            // fxClampMinMax_panel
            // 
            this.fxClampMinMax_panel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.fxClampMinMax_panel.BackColor = System.Drawing.SystemColors.ControlLight;
            this.fxClampMinMax_panel.Controls.Add(this.fxClampMaxScaleX_numBx);
            this.fxClampMinMax_panel.Controls.Add(this.fxClampMinScaleX_numBx);
            this.fxClampMinMax_panel.Controls.Add(this.fxClampMaxScale_ckbx);
            this.fxClampMinMax_panel.Controls.Add(this.fxClampMaxScaleZ_numBx);
            this.fxClampMinMax_panel.Controls.Add(this.fxClampMaxScaleY_numBx);
            this.fxClampMinMax_panel.Controls.Add(this.fxClampMinScale_ckbx);
            this.fxClampMinMax_panel.Controls.Add(this.fxClampMinScaleZ_numBx);
            this.fxClampMinMax_panel.Controls.Add(this.fxClampMinScaleY_numBx);
            this.fxClampMinMax_panel.Location = new System.Drawing.Point(0, 81);
            this.fxClampMinMax_panel.Name = "fxClampMinMax_panel";
            this.fxClampMinMax_panel.Size = new System.Drawing.Size(666, 25);
            this.fxClampMinMax_panel.TabIndex = 0;
            // 
            // fxClampMaxScaleX_numBx
            // 
            this.fxClampMaxScaleX_numBx.DecimalPlaces = 4;
            this.fxClampMaxScaleX_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxClampMaxScaleX_numBx.Location = new System.Drawing.Point(437, 2);
            this.fxClampMaxScaleX_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxClampMaxScaleX_numBx.Name = "fxClampMaxScaleX_numBx";
            this.fxClampMaxScaleX_numBx.Size = new System.Drawing.Size(70, 20);
            this.fxClampMaxScaleX_numBx.TabIndex = 1;
            this.fxClampMaxScaleX_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.fxClampMaxScaleX_numBx.ValueChanged += new System.EventHandler(this.numBx_ValueChanged);
            // 
            // fxClampMinScaleX_numBx
            // 
            this.fxClampMinScaleX_numBx.DecimalPlaces = 4;
            this.fxClampMinScaleX_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxClampMinScaleX_numBx.Location = new System.Drawing.Point(107, 2);
            this.fxClampMinScaleX_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxClampMinScaleX_numBx.Name = "fxClampMinScaleX_numBx";
            this.fxClampMinScaleX_numBx.Size = new System.Drawing.Size(70, 20);
            this.fxClampMinScaleX_numBx.TabIndex = 1;
            this.fxClampMinScaleX_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxClampMaxScale_ckbx
            // 
            this.fxClampMaxScale_ckbx.AutoSize = true;
            this.fxClampMaxScale_ckbx.Location = new System.Drawing.Point(337, 4);
            this.fxClampMaxScale_ckbx.Name = "fxClampMaxScale_ckbx";
            this.fxClampMaxScale_ckbx.Size = new System.Drawing.Size(102, 17);
            this.fxClampMaxScale_ckbx.TabIndex = 0;
            this.fxClampMaxScale_ckbx.Text = "ClampMaxScale";
            this.fxClampMaxScale_ckbx.UseVisualStyleBackColor = true;
            this.fxClampMaxScale_ckbx.CheckedChanged += new System.EventHandler(this.enableCtl);
            // 
            // fxClampMaxScaleZ_numBx
            // 
            this.fxClampMaxScaleZ_numBx.DecimalPlaces = 4;
            this.fxClampMaxScaleZ_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxClampMaxScaleZ_numBx.Location = new System.Drawing.Point(582, 2);
            this.fxClampMaxScaleZ_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxClampMaxScaleZ_numBx.Name = "fxClampMaxScaleZ_numBx";
            this.fxClampMaxScaleZ_numBx.Size = new System.Drawing.Size(70, 20);
            this.fxClampMaxScaleZ_numBx.TabIndex = 1;
            this.fxClampMaxScaleZ_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxClampMaxScaleY_numBx
            // 
            this.fxClampMaxScaleY_numBx.DecimalPlaces = 4;
            this.fxClampMaxScaleY_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxClampMaxScaleY_numBx.Location = new System.Drawing.Point(509, 2);
            this.fxClampMaxScaleY_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxClampMaxScaleY_numBx.Name = "fxClampMaxScaleY_numBx";
            this.fxClampMaxScaleY_numBx.Size = new System.Drawing.Size(70, 20);
            this.fxClampMaxScaleY_numBx.TabIndex = 1;
            this.fxClampMaxScaleY_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxClampMinScale_ckbx
            // 
            this.fxClampMinScale_ckbx.AutoSize = true;
            this.fxClampMinScale_ckbx.Location = new System.Drawing.Point(9, 3);
            this.fxClampMinScale_ckbx.Name = "fxClampMinScale_ckbx";
            this.fxClampMinScale_ckbx.Size = new System.Drawing.Size(99, 17);
            this.fxClampMinScale_ckbx.TabIndex = 0;
            this.fxClampMinScale_ckbx.Text = "ClampMinScale";
            this.fxClampMinScale_ckbx.UseVisualStyleBackColor = true;
            this.fxClampMinScale_ckbx.CheckedChanged += new System.EventHandler(this.enableCtl);
            // 
            // fxClampMinScaleZ_numBx
            // 
            this.fxClampMinScaleZ_numBx.DecimalPlaces = 4;
            this.fxClampMinScaleZ_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxClampMinScaleZ_numBx.Location = new System.Drawing.Point(252, 2);
            this.fxClampMinScaleZ_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxClampMinScaleZ_numBx.Name = "fxClampMinScaleZ_numBx";
            this.fxClampMinScaleZ_numBx.Size = new System.Drawing.Size(70, 20);
            this.fxClampMinScaleZ_numBx.TabIndex = 1;
            this.fxClampMinScaleZ_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxClampMinScaleY_numBx
            // 
            this.fxClampMinScaleY_numBx.DecimalPlaces = 4;
            this.fxClampMinScaleY_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxClampMinScaleY_numBx.Location = new System.Drawing.Point(179, 2);
            this.fxClampMinScaleY_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxClampMinScaleY_numBx.Name = "fxClampMinScaleY_numBx";
            this.fxClampMinScaleY_numBx.Size = new System.Drawing.Size(70, 20);
            this.fxClampMinScaleY_numBx.TabIndex = 1;
            this.fxClampMinScaleY_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxFileAgePlus_panel
            // 
            this.fxFileAgePlus_panel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.fxFileAgePlus_panel.BackColor = System.Drawing.SystemColors.Control;
            this.fxFileAgePlus_panel.Controls.Add(this.fxPerformanceRadius_numBx);
            this.fxFileAgePlus_panel.Controls.Add(this.fxFileAge_numBx);
            this.fxFileAgePlus_panel.Controls.Add(this.fxOnForceRadius_numBx);
            this.fxFileAgePlus_panel.Controls.Add(this.fxPerformanceRadius_ckbx);
            this.fxFileAgePlus_panel.Controls.Add(this.fxOnForceRadius_ckbx);
            this.fxFileAgePlus_panel.Controls.Add(this.fxFileAge_ckbx);
            this.fxFileAgePlus_panel.Location = new System.Drawing.Point(0, 54);
            this.fxFileAgePlus_panel.Name = "fxFileAgePlus_panel";
            this.fxFileAgePlus_panel.Size = new System.Drawing.Size(666, 25);
            this.fxFileAgePlus_panel.TabIndex = 0;
            // 
            // fxPerformanceRadius_numBx
            // 
            this.fxPerformanceRadius_numBx.DecimalPlaces = 4;
            this.fxPerformanceRadius_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxPerformanceRadius_numBx.Location = new System.Drawing.Point(344, 2);
            this.fxPerformanceRadius_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxPerformanceRadius_numBx.Name = "fxPerformanceRadius_numBx";
            this.fxPerformanceRadius_numBx.Size = new System.Drawing.Size(74, 20);
            this.fxPerformanceRadius_numBx.TabIndex = 1;
            this.fxPerformanceRadius_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxFileAge_numBx
            // 
            this.fxFileAge_numBx.DecimalPlaces = 4;
            this.fxFileAge_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxFileAge_numBx.Location = new System.Drawing.Point(86, 2);
            this.fxFileAge_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxFileAge_numBx.Name = "fxFileAge_numBx";
            this.fxFileAge_numBx.Size = new System.Drawing.Size(74, 20);
            this.fxFileAge_numBx.TabIndex = 1;
            this.fxFileAge_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxOnForceRadius_numBx
            // 
            this.fxOnForceRadius_numBx.DecimalPlaces = 4;
            this.fxOnForceRadius_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxOnForceRadius_numBx.Location = new System.Drawing.Point(578, 2);
            this.fxOnForceRadius_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxOnForceRadius_numBx.Name = "fxOnForceRadius_numBx";
            this.fxOnForceRadius_numBx.Size = new System.Drawing.Size(74, 20);
            this.fxOnForceRadius_numBx.TabIndex = 1;
            this.fxOnForceRadius_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxPerformanceRadius_ckbx
            // 
            this.fxPerformanceRadius_ckbx.AutoSize = true;
            this.fxPerformanceRadius_ckbx.Location = new System.Drawing.Point(221, 4);
            this.fxPerformanceRadius_ckbx.Name = "fxPerformanceRadius_ckbx";
            this.fxPerformanceRadius_ckbx.Size = new System.Drawing.Size(119, 17);
            this.fxPerformanceRadius_ckbx.TabIndex = 0;
            this.fxPerformanceRadius_ckbx.Text = "PerformanceRadius";
            this.fxPerformanceRadius_ckbx.UseVisualStyleBackColor = true;
            this.fxPerformanceRadius_ckbx.CheckedChanged += new System.EventHandler(this.enableCtl);
            // 
            // fxOnForceRadius_ckbx
            // 
            this.fxOnForceRadius_ckbx.AutoSize = true;
            this.fxOnForceRadius_ckbx.Location = new System.Drawing.Point(477, 4);
            this.fxOnForceRadius_ckbx.Name = "fxOnForceRadius_ckbx";
            this.fxOnForceRadius_ckbx.Size = new System.Drawing.Size(100, 17);
            this.fxOnForceRadius_ckbx.TabIndex = 0;
            this.fxOnForceRadius_ckbx.Text = "OnForceRadius";
            this.fxOnForceRadius_ckbx.UseVisualStyleBackColor = true;
            this.fxOnForceRadius_ckbx.CheckedChanged += new System.EventHandler(this.enableCtl);
            // 
            // fxFileAge_ckbx
            // 
            this.fxFileAge_ckbx.AutoSize = true;
            this.fxFileAge_ckbx.Location = new System.Drawing.Point(9, 4);
            this.fxFileAge_ckbx.Name = "fxFileAge_ckbx";
            this.fxFileAge_ckbx.Size = new System.Drawing.Size(61, 17);
            this.fxFileAge_ckbx.TabIndex = 0;
            this.fxFileAge_ckbx.Text = "FileAge";
            this.fxFileAge_ckbx.UseVisualStyleBackColor = true;
            this.fxFileAge_ckbx.CheckedChanged += new System.EventHandler(this.enableCtl);
            // 
            // fxLifSpanPlus_panel
            // 
            this.fxLifSpanPlus_panel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.fxLifSpanPlus_panel.BackColor = System.Drawing.SystemColors.ControlLight;
            this.fxLifSpanPlus_panel.Controls.Add(this.fxLighting_numBx);
            this.fxLifSpanPlus_panel.Controls.Add(this.fxLighting_ckbx);
            this.fxLifSpanPlus_panel.Controls.Add(this.fxAnimScale_numBx);
            this.fxLifSpanPlus_panel.Controls.Add(this.fxLifeSpan_numBx);
            this.fxLifSpanPlus_panel.Controls.Add(this.fxAnimScale_ckbx);
            this.fxLifSpanPlus_panel.Controls.Add(this.fxLifeSpan_ckbx);
            this.fxLifSpanPlus_panel.Location = new System.Drawing.Point(0, 27);
            this.fxLifSpanPlus_panel.Name = "fxLifSpanPlus_panel";
            this.fxLifSpanPlus_panel.Size = new System.Drawing.Size(632, 25);
            this.fxLifSpanPlus_panel.TabIndex = 0;
            // 
            // fxLighting_numBx
            // 
            this.fxLighting_numBx.Location = new System.Drawing.Point(578, 2);
            this.fxLighting_numBx.Maximum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.fxLighting_numBx.Name = "fxLighting_numBx";
            this.fxLighting_numBx.Size = new System.Drawing.Size(74, 20);
            this.fxLighting_numBx.TabIndex = 1;
            this.fxLighting_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxLighting_ckbx
            // 
            this.fxLighting_ckbx.AutoSize = true;
            this.fxLighting_ckbx.Location = new System.Drawing.Point(477, 4);
            this.fxLighting_ckbx.Name = "fxLighting_ckbx";
            this.fxLighting_ckbx.Size = new System.Drawing.Size(63, 17);
            this.fxLighting_ckbx.TabIndex = 0;
            this.fxLighting_ckbx.Text = "Lighting";
            this.fxLighting_ckbx.UseVisualStyleBackColor = true;
            this.fxLighting_ckbx.CheckedChanged += new System.EventHandler(this.enableCtl);
            // 
            // fxAnimScale_numBx
            // 
            this.fxAnimScale_numBx.DecimalPlaces = 4;
            this.fxAnimScale_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxAnimScale_numBx.Location = new System.Drawing.Point(343, 2);
            this.fxAnimScale_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxAnimScale_numBx.Name = "fxAnimScale_numBx";
            this.fxAnimScale_numBx.Size = new System.Drawing.Size(74, 20);
            this.fxAnimScale_numBx.TabIndex = 1;
            this.fxAnimScale_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxLifeSpan_numBx
            // 
            this.fxLifeSpan_numBx.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.fxLifeSpan_numBx.Location = new System.Drawing.Point(86, 2);
            this.fxLifeSpan_numBx.Maximum = new decimal(new int[] {
            1000000,
            0,
            0,
            0});
            this.fxLifeSpan_numBx.Name = "fxLifeSpan_numBx";
            this.fxLifeSpan_numBx.Size = new System.Drawing.Size(74, 20);
            this.fxLifeSpan_numBx.TabIndex = 1;
            this.fxLifeSpan_numBx.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // fxAnimScale_ckbx
            // 
            this.fxAnimScale_ckbx.AutoSize = true;
            this.fxAnimScale_ckbx.Location = new System.Drawing.Point(221, 4);
            this.fxAnimScale_ckbx.Name = "fxAnimScale_ckbx";
            this.fxAnimScale_ckbx.Size = new System.Drawing.Size(76, 17);
            this.fxAnimScale_ckbx.TabIndex = 0;
            this.fxAnimScale_ckbx.Text = "AnimScale";
            this.fxAnimScale_ckbx.UseVisualStyleBackColor = true;
            this.fxAnimScale_ckbx.CheckedChanged += new System.EventHandler(this.enableCtl);
            // 
            // fxLifeSpan_ckbx
            // 
            this.fxLifeSpan_ckbx.AutoSize = true;
            this.fxLifeSpan_ckbx.Location = new System.Drawing.Point(9, 4);
            this.fxLifeSpan_ckbx.Name = "fxLifeSpan_ckbx";
            this.fxLifeSpan_ckbx.Size = new System.Drawing.Size(68, 17);
            this.fxLifeSpan_ckbx.TabIndex = 0;
            this.fxLifeSpan_ckbx.Text = "LifeSpan";
            this.fxLifeSpan_ckbx.UseVisualStyleBackColor = true;
            this.fxLifeSpan_ckbx.CheckedChanged += new System.EventHandler(this.enableCtl);
            // 
            // fxName_panel
            // 
            this.fxName_panel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.fxName_panel.BackColor = System.Drawing.SystemColors.Control;
            this.fxName_panel.Controls.Add(this.fxName_label);
            this.fxName_panel.Controls.Add(this.fxName_txbx);
            this.fxName_panel.Location = new System.Drawing.Point(0, 0);
            this.fxName_panel.Name = "fxName_panel";
            this.fxName_panel.Size = new System.Drawing.Size(666, 25);
            this.fxName_panel.TabIndex = 0;
            // 
            // fxName_label
            // 
            this.fxName_label.AutoSize = true;
            this.fxName_label.Location = new System.Drawing.Point(9, 6);
            this.fxName_label.Name = "fxName_label";
            this.fxName_label.Size = new System.Drawing.Size(73, 13);
            this.fxName_label.TabIndex = 2;
            this.fxName_label.Text = "FX File Name:";
            // 
            // fxName_txbx
            // 
            this.fxName_txbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.fxName_txbx.Enabled = false;
            this.fxName_txbx.Location = new System.Drawing.Point(86, 2);
            this.fxName_txbx.Name = "fxName_txbx";
            this.fxName_txbx.Size = new System.Drawing.Size(507, 20);
            this.fxName_txbx.TabIndex = 1;
            // 
            // fxFlags_gpbx
            // 
            this.fxFlags_gpbx.Controls.Add(this.NotCompatibleWithAnimalHead_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.SoundOnly_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.DontInheritTexFromCostume_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.InheritGeoScale_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.DontSuppress_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.IsWeapon_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.DontInheritBits_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.IsShield_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.InheritAnimScale_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.UseSkinTint_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.IsArmor_ckbx);
            this.fxFlags_gpbx.Controls.Add(this.InheritAlpha_ckbx);
            this.fxFlags_gpbx.Dock = System.Windows.Forms.DockStyle.Top;
            this.fxFlags_gpbx.Location = new System.Drawing.Point(3, 3);
            this.fxFlags_gpbx.Name = "fxFlags_gpbx";
            this.fxFlags_gpbx.Size = new System.Drawing.Size(769, 70);
            this.fxFlags_gpbx.TabIndex = 0;
            this.fxFlags_gpbx.TabStop = false;
            this.fxFlags_gpbx.Text = "Fx Flags";
            // 
            // NotCompatibleWithAnimalHead_ckbx
            // 
            this.NotCompatibleWithAnimalHead_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.NotCompatibleWithAnimalHead_ckbx.AutoSize = true;
            this.NotCompatibleWithAnimalHead_ckbx.Location = new System.Drawing.Point(464, 43);
            this.NotCompatibleWithAnimalHead_ckbx.Name = "NotCompatibleWithAnimalHead_ckbx";
            this.NotCompatibleWithAnimalHead_ckbx.Size = new System.Drawing.Size(174, 17);
            this.NotCompatibleWithAnimalHead_ckbx.TabIndex = 0;
            this.NotCompatibleWithAnimalHead_ckbx.Text = "NotCompatibleWithAnimalHead";
            this.NotCompatibleWithAnimalHead_ckbx.UseVisualStyleBackColor = true;
            // 
            // SoundOnly_ckbx
            // 
            this.SoundOnly_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.SoundOnly_ckbx.AutoSize = true;
            this.SoundOnly_ckbx.Location = new System.Drawing.Point(371, 43);
            this.SoundOnly_ckbx.Name = "SoundOnly_ckbx";
            this.SoundOnly_ckbx.Size = new System.Drawing.Size(78, 17);
            this.SoundOnly_ckbx.TabIndex = 0;
            this.SoundOnly_ckbx.Text = "SoundOnly";
            this.SoundOnly_ckbx.UseVisualStyleBackColor = true;
            // 
            // DontInheritTexFromCostume_ckbx
            // 
            this.DontInheritTexFromCostume_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.DontInheritTexFromCostume_ckbx.AutoSize = true;
            this.DontInheritTexFromCostume_ckbx.Location = new System.Drawing.Point(464, 20);
            this.DontInheritTexFromCostume_ckbx.Name = "DontInheritTexFromCostume_ckbx";
            this.DontInheritTexFromCostume_ckbx.Size = new System.Drawing.Size(160, 17);
            this.DontInheritTexFromCostume_ckbx.TabIndex = 0;
            this.DontInheritTexFromCostume_ckbx.Text = "DontInheritTexFromCostume";
            this.DontInheritTexFromCostume_ckbx.UseVisualStyleBackColor = true;
            // 
            // InheritGeoScale_ckbx
            // 
            this.InheritGeoScale_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.InheritGeoScale_ckbx.AutoSize = true;
            this.InheritGeoScale_ckbx.Location = new System.Drawing.Point(265, 43);
            this.InheritGeoScale_ckbx.Name = "InheritGeoScale_ckbx";
            this.InheritGeoScale_ckbx.Size = new System.Drawing.Size(102, 17);
            this.InheritGeoScale_ckbx.TabIndex = 0;
            this.InheritGeoScale_ckbx.Text = "InheritGeoScale";
            this.InheritGeoScale_ckbx.UseVisualStyleBackColor = true;
            // 
            // DontSuppress_ckbx
            // 
            this.DontSuppress_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.DontSuppress_ckbx.AutoSize = true;
            this.DontSuppress_ckbx.Location = new System.Drawing.Point(371, 19);
            this.DontSuppress_ckbx.Name = "DontSuppress_ckbx";
            this.DontSuppress_ckbx.Size = new System.Drawing.Size(93, 17);
            this.DontSuppress_ckbx.TabIndex = 0;
            this.DontSuppress_ckbx.Text = "DontSuppress";
            this.DontSuppress_ckbx.UseVisualStyleBackColor = true;
            // 
            // IsWeapon_ckbx
            // 
            this.IsWeapon_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.IsWeapon_ckbx.AutoSize = true;
            this.IsWeapon_ckbx.Location = new System.Drawing.Point(160, 43);
            this.IsWeapon_ckbx.Name = "IsWeapon_ckbx";
            this.IsWeapon_ckbx.Size = new System.Drawing.Size(75, 17);
            this.IsWeapon_ckbx.TabIndex = 0;
            this.IsWeapon_ckbx.Text = "IsWeapon";
            this.IsWeapon_ckbx.UseVisualStyleBackColor = true;
            // 
            // DontInheritBits_ckbx
            // 
            this.DontInheritBits_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.DontInheritBits_ckbx.AutoSize = true;
            this.DontInheritBits_ckbx.Location = new System.Drawing.Point(265, 20);
            this.DontInheritBits_ckbx.Name = "DontInheritBits_ckbx";
            this.DontInheritBits_ckbx.Size = new System.Drawing.Size(95, 17);
            this.DontInheritBits_ckbx.TabIndex = 0;
            this.DontInheritBits_ckbx.Text = "DontInheritBits";
            this.DontInheritBits_ckbx.UseVisualStyleBackColor = true;
            // 
            // IsShield_ckbx
            // 
            this.IsShield_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.IsShield_ckbx.AutoSize = true;
            this.IsShield_ckbx.Location = new System.Drawing.Point(95, 43);
            this.IsShield_ckbx.Name = "IsShield_ckbx";
            this.IsShield_ckbx.Size = new System.Drawing.Size(63, 17);
            this.IsShield_ckbx.TabIndex = 0;
            this.IsShield_ckbx.Text = "IsShield";
            this.IsShield_ckbx.UseVisualStyleBackColor = true;
            // 
            // InheritAnimScale_ckbx
            // 
            this.InheritAnimScale_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.InheritAnimScale_ckbx.AutoSize = true;
            this.InheritAnimScale_ckbx.Location = new System.Drawing.Point(160, 20);
            this.InheritAnimScale_ckbx.Name = "InheritAnimScale_ckbx";
            this.InheritAnimScale_ckbx.Size = new System.Drawing.Size(105, 17);
            this.InheritAnimScale_ckbx.TabIndex = 0;
            this.InheritAnimScale_ckbx.Text = "InheritAnimScale";
            this.InheritAnimScale_ckbx.UseVisualStyleBackColor = true;
            // 
            // UseSkinTint_ckbx
            // 
            this.UseSkinTint_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.UseSkinTint_ckbx.AutoSize = true;
            this.UseSkinTint_ckbx.Location = new System.Drawing.Point(9, 43);
            this.UseSkinTint_ckbx.Name = "UseSkinTint_ckbx";
            this.UseSkinTint_ckbx.Size = new System.Drawing.Size(84, 17);
            this.UseSkinTint_ckbx.TabIndex = 0;
            this.UseSkinTint_ckbx.Text = "UseSkinTint";
            this.UseSkinTint_ckbx.UseVisualStyleBackColor = true;
            // 
            // IsArmor_ckbx
            // 
            this.IsArmor_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.IsArmor_ckbx.AutoSize = true;
            this.IsArmor_ckbx.Location = new System.Drawing.Point(95, 20);
            this.IsArmor_ckbx.Name = "IsArmor_ckbx";
            this.IsArmor_ckbx.Size = new System.Drawing.Size(61, 17);
            this.IsArmor_ckbx.TabIndex = 0;
            this.IsArmor_ckbx.Text = "IsArmor";
            this.IsArmor_ckbx.UseVisualStyleBackColor = true;
            // 
            // InheritAlpha_ckbx
            // 
            this.InheritAlpha_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.InheritAlpha_ckbx.AutoSize = true;
            this.InheritAlpha_ckbx.Location = new System.Drawing.Point(9, 20);
            this.InheritAlpha_ckbx.Name = "InheritAlpha_ckbx";
            this.InheritAlpha_ckbx.Size = new System.Drawing.Size(82, 17);
            this.InheritAlpha_ckbx.TabIndex = 0;
            this.InheritAlpha_ckbx.Text = "InheritAlpha";
            this.InheritAlpha_ckbx.UseVisualStyleBackColor = true;
            // 
            // conditions_tabPage
            // 
            this.conditions_tabPage.Controls.Add(this.mConditionWin);
            this.conditions_tabPage.Location = new System.Drawing.Point(4, 22);
            this.conditions_tabPage.Name = "conditions_tabPage";
            this.conditions_tabPage.Size = new System.Drawing.Size(775, 327);
            this.conditions_tabPage.TabIndex = 1;
            this.conditions_tabPage.Text = "Conditions";
            this.conditions_tabPage.UseVisualStyleBackColor = true;
            // 
            // mConditionWin
            // 
            this.mConditionWin.CurrentRowCondition = null;
            this.mConditionWin.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mConditionWin.Location = new System.Drawing.Point(0, 0);
            this.mConditionWin.Name = "mConditionWin";
            this.mConditionWin.Size = new System.Drawing.Size(775, 327);
            this.mConditionWin.TabIndex = 0;
            this.mConditionWin.Text = "FXWin";
            this.mConditionWin.CurrentRowChanged += new System.EventHandler(this.mConditionWin_CurrentRowChanged);
            // 
            // events_tabPage
            // 
            this.events_tabPage.Controls.Add(this.mEventWin);
            this.events_tabPage.Location = new System.Drawing.Point(4, 22);
            this.events_tabPage.Name = "events_tabPage";
            this.events_tabPage.Size = new System.Drawing.Size(775, 327);
            this.events_tabPage.TabIndex = 2;
            this.events_tabPage.Text = "Events";
            this.events_tabPage.UseVisualStyleBackColor = true;
            // 
            // mEventWin
            // 
            this.mEventWin.CurrentRowEvent = null;
            this.mEventWin.Dock = System.Windows.Forms.DockStyle.Fill;
            this.mEventWin.Location = new System.Drawing.Point(0, 0);
            this.mEventWin.Name = "mEventWin";
            this.mEventWin.Size = new System.Drawing.Size(775, 327);
            this.mEventWin.TabIndex = 0;
            this.mEventWin.Text = "FXWin";
            this.mEventWin.CurrentRowChanged += new System.EventHandler(this.mEventWin_CurrentRowChanged);
            // 
            // sound_tabPage
            // 
            this.sound_tabPage.AutoScroll = true;
            this.sound_tabPage.Location = new System.Drawing.Point(4, 22);
            this.sound_tabPage.Name = "sound_tabPage";
            this.sound_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.sound_tabPage.Size = new System.Drawing.Size(775, 327);
            this.sound_tabPage.TabIndex = 0;
            this.sound_tabPage.Text = "Ev. Sounds:";
            this.sound_tabPage.UseVisualStyleBackColor = true;
            // 
            // splat_tabPage
            // 
            this.splat_tabPage.AutoScroll = true;
            this.splat_tabPage.Location = new System.Drawing.Point(4, 22);
            this.splat_tabPage.Name = "splat_tabPage";
            this.splat_tabPage.Size = new System.Drawing.Size(775, 327);
            this.splat_tabPage.TabIndex = 0;
            this.splat_tabPage.Text = "Ev. Splat:";
            this.splat_tabPage.UseVisualStyleBackColor = true;
            // 
            // fxOption_panel
            // 
            this.fxOption_panel.BackColor = System.Drawing.SystemColors.GradientActiveCaption;
            this.fxOption_panel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.fxOption_panel.Controls.Add(this.saveConfig_btn);
            this.fxOption_panel.Controls.Add(this.findGame_btn);
            this.fxOption_panel.Controls.Add(this.filter_btn);
            this.fxOption_panel.Controls.Add(this.p4Add_btn);
            this.fxOption_panel.Controls.Add(this.p4CheckOut_btn);
            this.fxOption_panel.Controls.Add(this.p4CheckIn_btn);
            this.fxOption_panel.Controls.Add(this.p4DepotDiff_btn);
            this.fxOption_panel.Controls.Add(this.p4_Label);
            this.fxOption_panel.Dock = System.Windows.Forms.DockStyle.Top;
            this.fxOption_panel.Location = new System.Drawing.Point(0, 0);
            this.fxOption_panel.Name = "fxOption_panel";
            this.fxOption_panel.Size = new System.Drawing.Size(783, 28);
            this.fxOption_panel.TabIndex = 0;
            // 
            // saveConfig_btn
            // 
            this.saveConfig_btn.Location = new System.Drawing.Point(73, 1);
            this.saveConfig_btn.Name = "saveConfig_btn";
            this.saveConfig_btn.Size = new System.Drawing.Size(88, 23);
            this.saveConfig_btn.TabIndex = 2;
            this.saveConfig_btn.Text = "Save Config.";
            this.saveConfig_btn.UseVisualStyleBackColor = true;
            this.saveConfig_btn.Click += new System.EventHandler(this.saveConfig_Click);
            // 
            // filter_btn
            // 
            this.filter_btn.Location = new System.Drawing.Point(2, 1);
            this.filter_btn.Name = "filter_btn";
            this.filter_btn.Size = new System.Drawing.Size(65, 23);
            this.filter_btn.TabIndex = 2;
            this.filter_btn.Text = "Filter";
            this.filter_btn.UseVisualStyleBackColor = true;
            this.filter_btn.Click += new System.EventHandler(this.filter_btn_Click);
            // 
            // p4Add_btn
            // 
            this.p4Add_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4Add_btn.Location = new System.Drawing.Point(515, 1);
            this.p4Add_btn.Name = "p4Add_btn";
            this.p4Add_btn.Size = new System.Drawing.Size(65, 23);
            this.p4Add_btn.TabIndex = 0;
            this.p4Add_btn.Text = "Add";
            this.p4Add_btn.UseVisualStyleBackColor = true;
            this.p4Add_btn.Click += new System.EventHandler(this.p4Add_btn_Click);
            // 
            // p4CheckOut_btn
            // 
            this.p4CheckOut_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4CheckOut_btn.Location = new System.Drawing.Point(580, 1);
            this.p4CheckOut_btn.Name = "p4CheckOut_btn";
            this.p4CheckOut_btn.Size = new System.Drawing.Size(65, 23);
            this.p4CheckOut_btn.TabIndex = 0;
            this.p4CheckOut_btn.Text = "CheckOut";
            this.p4CheckOut_btn.UseVisualStyleBackColor = true;
            this.p4CheckOut_btn.Click += new System.EventHandler(this.p4CheckOut_btn_Click);
            // 
            // p4CheckIn_btn
            // 
            this.p4CheckIn_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4CheckIn_btn.Location = new System.Drawing.Point(645, 1);
            this.p4CheckIn_btn.Name = "p4CheckIn_btn";
            this.p4CheckIn_btn.Size = new System.Drawing.Size(65, 23);
            this.p4CheckIn_btn.TabIndex = 0;
            this.p4CheckIn_btn.Text = "CheckIn";
            this.p4CheckIn_btn.UseVisualStyleBackColor = true;
            this.p4CheckIn_btn.Click += new System.EventHandler(this.p4CheckIn_btn_Click);
            // 
            // p4DepotDiff_btn
            // 
            this.p4DepotDiff_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4DepotDiff_btn.Location = new System.Drawing.Point(710, 1);
            this.p4DepotDiff_btn.Name = "p4DepotDiff_btn";
            this.p4DepotDiff_btn.Size = new System.Drawing.Size(65, 23);
            this.p4DepotDiff_btn.TabIndex = 0;
            this.p4DepotDiff_btn.Text = "Depot Diff";
            this.p4DepotDiff_btn.UseVisualStyleBackColor = true;
            this.p4DepotDiff_btn.Click += new System.EventHandler(this.p4DepotDiff_btn_Click);
            // 
            // p4_Label
            // 
            this.p4_Label.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4_Label.AutoSize = true;
            this.p4_Label.Location = new System.Drawing.Point(493, 6);
            this.p4_Label.Name = "p4_Label";
            this.p4_Label.Size = new System.Drawing.Size(23, 13);
            this.p4_Label.TabIndex = 1;
            this.p4_Label.Text = "P4:";
            // 
            // findGame_btn
            // 
            this.findGame_btn.Location = new System.Drawing.Point(167, 1);
            this.findGame_btn.Name = "findGame_btn";
            this.findGame_btn.Size = new System.Drawing.Size(73, 23);
            this.findGame_btn.TabIndex = 2;
            this.findGame_btn.Text = "Find Game";
            this.findGame_btn.UseVisualStyleBackColor = true;
            this.findGame_btn.Click += new System.EventHandler(this.findGame_btn_Click);
            // 
            // FXWin
            // 
            this.ClientSize = new System.Drawing.Size(972, 381);
            this.Controls.Add(this.fx_splitContainer);
            this.Name = "FXWin";
            this.Text = "FXWin";
            this.fx_splitContainer.Panel1.ResumeLayout(false);
            this.fx_splitContainer.Panel2.ResumeLayout(false);
            this.fx_splitContainer.ResumeLayout(false);
            this.fx_tv_ContextMenuStrip.ResumeLayout(false);
            this.fx_tabControl.ResumeLayout(false);
            this.fx_tabPage.ResumeLayout(false);
            this.fxTags_panel.ResumeLayout(false);
            this.fxClampMinMax_panel.ResumeLayout(false);
            this.fxClampMinMax_panel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMaxScaleX_numBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMinScaleX_numBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMaxScaleZ_numBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMaxScaleY_numBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMinScaleZ_numBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxClampMinScaleY_numBx)).EndInit();
            this.fxFileAgePlus_panel.ResumeLayout(false);
            this.fxFileAgePlus_panel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.fxPerformanceRadius_numBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxFileAge_numBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxOnForceRadius_numBx)).EndInit();
            this.fxLifSpanPlus_panel.ResumeLayout(false);
            this.fxLifSpanPlus_panel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.fxLighting_numBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxAnimScale_numBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.fxLifeSpan_numBx)).EndInit();
            this.fxName_panel.ResumeLayout(false);
            this.fxName_panel.PerformLayout();
            this.fxFlags_gpbx.ResumeLayout(false);
            this.fxFlags_gpbx.PerformLayout();
            this.conditions_tabPage.ResumeLayout(false);
            this.events_tabPage.ResumeLayout(false);
            this.fxOption_panel.ResumeLayout(false);
            this.fxOption_panel.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer fx_splitContainer;
        //private System.Windows.Forms.TreeView fx_tv;
        private FxLauncher.TreeViewMultiSelect fx_tv;//multi select TV
        private System.Windows.Forms.TabControl fx_tabControl;
        private System.Windows.Forms.TabPage fx_tabPage;
        private System.Windows.Forms.TabPage conditions_tabPage;
        private System.Windows.Forms.TabPage events_tabPage;
        private System.Windows.Forms.TabPage sound_tabPage;
        private System.Windows.Forms.TabPage splat_tabPage;
        private System.Windows.Forms.GroupBox fxFlags_gpbx;
        private System.Windows.Forms.CheckBox NotCompatibleWithAnimalHead_ckbx;
        private System.Windows.Forms.CheckBox SoundOnly_ckbx;
        private System.Windows.Forms.CheckBox DontInheritTexFromCostume_ckbx;
        private System.Windows.Forms.CheckBox InheritGeoScale_ckbx;
        private System.Windows.Forms.CheckBox DontSuppress_ckbx;
        private System.Windows.Forms.CheckBox IsWeapon_ckbx;
        private System.Windows.Forms.CheckBox DontInheritBits_ckbx;
        private System.Windows.Forms.CheckBox IsShield_ckbx;
        private System.Windows.Forms.CheckBox InheritAnimScale_ckbx;
        private System.Windows.Forms.CheckBox UseSkinTint_ckbx;
        private System.Windows.Forms.CheckBox IsArmor_ckbx;
        private System.Windows.Forms.CheckBox InheritAlpha_ckbx;
        private System.Windows.Forms.CheckBox fxPerformanceRadius_ckbx;
        private System.Windows.Forms.CheckBox fxLighting_ckbx;
        private System.Windows.Forms.CheckBox fxFileAge_ckbx;
        private System.Windows.Forms.CheckBox fxLifeSpan_ckbx;
        private System.Windows.Forms.CheckBox fxClampMaxScale_ckbx;
        private System.Windows.Forms.CheckBox fxClampMinScale_ckbx;
        private System.Windows.Forms.CheckBox fxAnimScale_ckbx;
        private System.Windows.Forms.CheckBox fxOnForceRadius_ckbx;        
        private System.Windows.Forms.Panel fxOption_panel;
        private System.Windows.Forms.Panel fxTags_panel;
        private System.Windows.Forms.Panel fxName_panel;
        private System.Windows.Forms.Panel fxInput_panel;
        private System.Windows.Forms.Panel fxClampMinMax_panel;
        private System.Windows.Forms.Panel fxFileAgePlus_panel;
        private System.Windows.Forms.Panel fxLifSpanPlus_panel;
        private System.Windows.Forms.TextBox fxName_txbx;
        private System.Windows.Forms.NumericUpDown fxLifeSpan_numBx;
        private System.Windows.Forms.NumericUpDown fxFileAge_numBx;
        private System.Windows.Forms.NumericUpDown fxPerformanceRadius_numBx;
        private System.Windows.Forms.NumericUpDown fxOnForceRadius_numBx;
        private System.Windows.Forms.NumericUpDown fxAnimScale_numBx;
        private System.Windows.Forms.NumericUpDown fxClampMaxScaleX_numBx;
        private System.Windows.Forms.NumericUpDown fxClampMaxScaleZ_numBx;
        private System.Windows.Forms.NumericUpDown fxClampMaxScaleY_numBx;
        private System.Windows.Forms.NumericUpDown fxClampMinScaleZ_numBx;
        private System.Windows.Forms.NumericUpDown fxClampMinScaleY_numBx;
        private System.Windows.Forms.NumericUpDown fxClampMinScaleX_numBx;
        private System.Windows.Forms.NumericUpDown fxLighting_numBx;
        private ConditionWin mConditionWin;
        private EventWin mEventWin;
        private System.Windows.Forms.Button p4Add_btn;
        private System.Windows.Forms.Button p4CheckOut_btn;
        private System.Windows.Forms.Button p4CheckIn_btn;
        private System.Windows.Forms.Button p4DepotDiff_btn;
        private System.Windows.Forms.Label p4_Label;
        //private SplatPanel splatPanel;
        private System.Windows.Forms.Button filter_btn;
        private System.Windows.Forms.Label fxName_label;
        private System.Windows.Forms.ContextMenuStrip fx_tv_ContextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem sendToUltraEditToolStripMenuItem;


        private System.Windows.Forms.ToolStripMenuItem openInExplorerMenuItem;
        private System.Windows.Forms.ToolStripMenuItem copyMenuItem;
        private System.Windows.Forms.ToolStripMenuItem renameMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pasteMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteSplatMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addConditionToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addEventToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addBhvrToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addPartToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addSoundToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addSplatToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addInputToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem copyBhvrOvrMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pasteBhvrOvrMenuItem;
        private System.Windows.Forms.ToolStripMenuItem moveUpMenuItem;
        private System.Windows.Forms.ToolStripMenuItem moveDwnMenuItem;
        private System.Windows.Forms.ToolStripMenuItem addBhvrOvrMenuItem;
        private System.Windows.Forms.Button saveConfig_btn;
        private System.Windows.Forms.Button findGame_btn;
    }
}