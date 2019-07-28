using System;
using System.Runtime.InteropServices;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.assetEditor.aeCommon
{
    class MatBlock : GroupBox

    {
        #region members
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private object[] blend_Items;
        private object[] scroll_Items;
        private decimal dIncrement;
        private decimal dMaximum;
        private decimal dMinimum;
        public bool isDirty;

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.PictureBox tgaIconPicBx;
        private System.Windows.Forms.ContextMenu iconCntxMn;
        private System.Windows.Forms.MenuItem sendToPhotoShop_menuItem;
        private System.Windows.Forms.MenuItem undoDrag_menuItem;

        private System.Windows.Forms.Panel texturePanel;
        private System.Windows.Forms.Panel scaleScrollPanel;
        private System.Windows.Forms.Panel scalePanel;
        private System.Windows.Forms.Panel scrollPanel;

        private System.Windows.Forms.ComboBox blendType;
        private System.Windows.Forms.ComboBox scrollTypeCB;
        private System.Windows.Forms.Label scroll_V_Label;
        private System.Windows.Forms.Label scroll_U_Label;
        private System.Windows.Forms.CheckBox useScrollChBx;
        private System.Windows.Forms.NumericUpDown scrollVnumDeciUpDwn;
        private System.Windows.Forms.NumericUpDown scrollUnumDeciUpDwn;

        private System.Windows.Forms.Label scale_V_Label;
        private System.Windows.Forms.Label scale_U_Label;
        private System.Windows.Forms.CheckBox useScaleChBx;
        private System.Windows.Forms.NumericUpDown scaleVnumDeciUpDwn;
        private System.Windows.Forms.NumericUpDown scaleUnumDeciUpDwn;

        private System.Windows.Forms.Button tgaFileOpenBtn;
        private System.Windows.Forms.TextBox tgaFileNameTxBx;
        private System.Windows.Forms.CheckBox useTextureChBx;

        private System.Windows.Forms.CheckBox extraOptionChBx;
        private System.Windows.Forms.CheckBox addGlowTintCkBx;
        private System.Windows.Forms.Panel weightPanel;
        private System.Windows.Forms.CheckBox weightChBx;
        private System.Windows.Forms.NumericUpDown weightDecNumUpDwn;

        private bool useExtraOptionChBx;
        private bool extraOpIsAlpha;
        public bool isFallBack;
        private bool isFallBackBlend;
        private string extraOptionName;
        private ArrayList dataSnapShot01;
        private string[] texPath;
        private string[] texName;
        private string[] tPath;
        private string[] tName;

        #endregion 

        private delegate void DelegateOpenFile(String dropFileStr); 
        
        private DelegateOpenFile m_DelegateProcessDropedFile;

        public MatBlock(string name, string text, System.Drawing.Point location, 
                        System.Windows.Forms.OpenFileDialog openFileDialog, 
                        bool useExtraOptChBx,string extraOpName)
        {
            prep(name, text, location, openFileDialog, useExtraOptChBx, extraOpName, false);
        }

        public MatBlock(string name, string text, System.Drawing.Point location,
                        System.Windows.Forms.OpenFileDialog openFileDialog,
                        bool useExtraOptChBx, string extraOpName, bool isFallbackMat)
        {
            if (text.ToLower().Equals("blend"))
            {
                isFallBackBlend = true;
            }
            else
            {
                isFallBackBlend = false;
            }
            prep(name, text, location, openFileDialog, useExtraOptChBx, extraOpName, isFallbackMat);

        }

        private void prep(string name, string text, System.Drawing.Point location,
                        System.Windows.Forms.OpenFileDialog openFile_Dialog,
                        bool useExtraOptChBx, string extraOpName, bool isFallbackMat)
        {
            isDirty = false;
            this.isFallBack = isFallbackMat;
            this.openFileDialog1 = openFile_Dialog;
            this.Location = location;
            this.Name = name;
            this.Text = text;

            this.texName = null;
            this.texPath = null;

            this.blend_Items = new object[] { "Multiply", "ColorBlendDual",/*"DualColor",*/ "AddGlow", "AlphaDetail" };
            //Bounce Dose not work
            this.scroll_Items = new object[] { "Normal" };//, "Bounce"};

            this.dIncrement = new decimal(new int[] { 5, 0, 0, 65536 });
            this.dMaximum = new decimal(new int[] { 10000000, 0, 0, 0 });
            this.dMinimum = new decimal(new int[] { 10000000, 0, 0, -2147483648 });
            this.useExtraOptionChBx = useExtraOptChBx;
            this.extraOptionName = extraOpName;
            this.extraOpIsAlpha = extraOpName.Equals("Alpha");
            this.InitializeComponent();
            m_DelegateProcessDropedFile = new DelegateOpenFile(this.processDropedFile);
            if (useExtraOptChBx && extraOptionName.Trim().Equals("AddGlowMat2"))
            {
                this.addGlowTintCkBx.Visible = true;
            }
        }


        #region Initialization
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MatBlock));
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.tgaIconPicBx = new System.Windows.Forms.PictureBox();
            this.iconCntxMn = new System.Windows.Forms.ContextMenu();
            this.sendToPhotoShop_menuItem = new System.Windows.Forms.MenuItem();
            this.undoDrag_menuItem = new System.Windows.Forms.MenuItem();
            this.scaleScrollPanel = new System.Windows.Forms.Panel();
            this.scrollPanel = new System.Windows.Forms.Panel();
            this.blendType = new System.Windows.Forms.ComboBox();
            this.scrollTypeCB = new System.Windows.Forms.ComboBox();
            this.scroll_V_Label = new System.Windows.Forms.Label();
            this.scroll_U_Label = new System.Windows.Forms.Label();
            this.useScrollChBx = new System.Windows.Forms.CheckBox();
            this.scrollVnumDeciUpDwn = new System.Windows.Forms.NumericUpDown();
            this.scrollUnumDeciUpDwn = new System.Windows.Forms.NumericUpDown();
            this.scalePanel = new System.Windows.Forms.Panel();
            this.scale_V_Label = new System.Windows.Forms.Label();
            this.scale_U_Label = new System.Windows.Forms.Label();
            this.useScaleChBx = new System.Windows.Forms.CheckBox();
            this.scaleVnumDeciUpDwn = new System.Windows.Forms.NumericUpDown();
            this.scaleUnumDeciUpDwn = new System.Windows.Forms.NumericUpDown();
            this.texturePanel = new System.Windows.Forms.Panel();
            this.tgaFileOpenBtn = new System.Windows.Forms.Button();
            this.tgaFileNameTxBx = new System.Windows.Forms.TextBox();
            this.useTextureChBx = new System.Windows.Forms.CheckBox();
            this.extraOptionChBx = new System.Windows.Forms.CheckBox();
            this.addGlowTintCkBx = new System.Windows.Forms.CheckBox();
            this.weightPanel = new System.Windows.Forms.Panel();
            this.weightChBx = new System.Windows.Forms.CheckBox();
            this.weightDecNumUpDwn = new System.Windows.Forms.NumericUpDown();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.tgaIconPicBx)).BeginInit();
            this.scaleScrollPanel.SuspendLayout();
            this.scrollPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.scrollVnumDeciUpDwn)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.scrollUnumDeciUpDwn)).BeginInit();
            this.scalePanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.scaleVnumDeciUpDwn)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.scaleUnumDeciUpDwn)).BeginInit();
            this.texturePanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // matBlock
            // 
            this.Controls.Add(this.splitContainer1);
            this.AllowDrop = true;
            this.DragEnter += new DragEventHandler(MatBlock_DragEnter);
            this.DragDrop += new DragEventHandler(MatBlock_DragDrop);
            this.MouseDown += new MouseEventHandler(MatBlock_MouseDown);
            this.DragOver += new DragEventHandler(MatBlock_DragOver);
            this.DragLeave += new EventHandler(MatBlock_DragLeave);
            this.TabIndex = 300;
            this.TabStop = false;
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(3, 16);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.tgaIconPicBx);
            // 
            // splitContainer1.Panel2
            // 

            this.splitContainer1.Panel2.Controls.Add(this.blendType);
            this.splitContainer1.Panel2.Controls.Add(this.scaleScrollPanel);
            this.splitContainer1.Panel2.Controls.Add(this.texturePanel);
            this.splitContainer1.Size = new System.Drawing.Size(906, 100);
            this.splitContainer1.SplitterDistance = 61;
            this.splitContainer1.TabIndex = 300;
            // 
            // ctxMn
            // 
            this.iconCntxMn.MenuItems.Add(this.undoDrag_menuItem);
            this.iconCntxMn.MenuItems.Add(this.sendToPhotoShop_menuItem);
            // 
            // sendToPhotoShop_menuItem
            // 
            this.sendToPhotoShop_menuItem.Index = 0;
            this.sendToPhotoShop_menuItem.Text = "Send To Photoshop";
            this.sendToPhotoShop_menuItem.Click += new System.EventHandler(sendToPhotoShop_menuItem_Click);
            // 
            // undoDrag_menuItem
            // 
            this.undoDrag_menuItem.Index = 0;
            this.undoDrag_menuItem.Text = "Undo Drag Operation";
            this.undoDrag_menuItem.Click += new System.EventHandler(undoDrag_menuItem_Click);
            // 
            // tgaIconPicBx
            // 
            this.tgaIconPicBx.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tgaIconPicBx.BackgroundImage = COH_CostumeUpdater.Properties.Resources.picBoxBkGrndImg;
            this.tgaIconPicBx.BackgroundImageLayout = ImageLayout.Tile;
            this.tgaIconPicBx.Location = new System.Drawing.Point(0, 0);
            this.tgaIconPicBx.Name = "tgaIconPicBx";
            this.tgaIconPicBx.Size = new System.Drawing.Size(60,60);
            this.tgaIconPicBx.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.tgaIconPicBx.TabIndex = 300;
            this.tgaIconPicBx.MouseDoubleClick += new MouseEventHandler(tgaIconPicBx_MouseDoubleClick);
            this.tgaIconPicBx.ContextMenu = this.iconCntxMn;
            this.tgaIconPicBx.TabStop = false;
            // 
            // texturePanel
            // 
            this.texturePanel.Controls.Add(this.tgaFileOpenBtn);
            this.texturePanel.Controls.Add(this.tgaFileNameTxBx);
            this.texturePanel.Controls.Add(this.useTextureChBx);
            this.texturePanel.Dock = System.Windows.Forms.DockStyle.Top;
            this.texturePanel.Location = new System.Drawing.Point(0, 0);
            this.texturePanel.Name = "texturePanel";
            this.texturePanel.Size = new System.Drawing.Size(/*781*/826, 30);
            this.texturePanel.TabIndex = 300;
            // 
            // scaleScrollPanel
            // 
            this.scaleScrollPanel.BackColor = System.Drawing.SystemColors.Control;
            this.scaleScrollPanel.Controls.Add(this.weightPanel);
            this.scaleScrollPanel.Controls.Add(this.extraOptionChBx);
            this.scaleScrollPanel.Controls.Add(this.addGlowTintCkBx);
            this.scaleScrollPanel.Controls.Add(this.scrollPanel);
            this.scaleScrollPanel.Controls.Add(this.scalePanel);
            this.scaleScrollPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.scaleScrollPanel.Location = new System.Drawing.Point(0, 40);
            this.scaleScrollPanel.Name = "scaleScrollPanel";
            this.scaleScrollPanel.Size = new System.Drawing.Size(781, 40);
            this.scaleScrollPanel.TabIndex = 300;
            this.scaleScrollPanel.Visible = !isFallBack;
            // 
            // scrollPanel
            // 
            this.scrollPanel.Controls.Add(this.scrollTypeCB);
            this.scrollPanel.Controls.Add(this.scroll_V_Label);
            this.scrollPanel.Controls.Add(this.scroll_U_Label);
            this.scrollPanel.Controls.Add(this.useScrollChBx);
            this.scrollPanel.Controls.Add(this.scrollVnumDeciUpDwn);
            this.scrollPanel.Controls.Add(this.scrollUnumDeciUpDwn);
            this.scrollPanel.Location = new System.Drawing.Point(290, 0);
            this.scrollPanel.Name = "scrollPanel";
            this.scrollPanel.Size = new System.Drawing.Size(376, 36);
            this.scrollPanel.TabIndex = 1;
            // 
            // scalePanel
            // 
            this.scalePanel.Controls.Add(this.scale_V_Label);
            this.scalePanel.Controls.Add(this.scale_U_Label);
            this.scalePanel.Controls.Add(this.useScaleChBx);
            this.scalePanel.Controls.Add(this.scaleVnumDeciUpDwn);
            this.scalePanel.Controls.Add(this.scaleUnumDeciUpDwn);
            this.scalePanel.Location = new System.Drawing.Point(10, 0);
            this.scalePanel.Name = "scalePanel";
            this.scalePanel.Size = new System.Drawing.Size(275, 36);
            this.scalePanel.TabIndex = 300;
            // 
            // useTextureChBx
            // 
            this.useTextureChBx.AutoSize = true;
            this.useTextureChBx.Location = new System.Drawing.Point(16, 7);
            this.useTextureChBx.Name = "useTextureChBx";
            this.useTextureChBx.Size = new System.Drawing.Size(62, 17);
            this.useTextureChBx.TabIndex = 0;
            this.useTextureChBx.Text = "Texture";
            this.useTextureChBx.UseVisualStyleBackColor = true;
            this.useTextureChBx.Click += new EventHandler(useTextureChBx_Click);
            // 
            // tgaFileNameTxBx
            // 
            this.tgaFileNameTxBx.Location = new System.Drawing.Point(75, 5);
            this.tgaFileNameTxBx.Name = "tgaFileNameTxBx";
            this.tgaFileNameTxBx.Size = new System.Drawing.Size(682, 20);
            this.tgaFileNameTxBx.TabIndex = 1;
            //this.tgaFileNameTxBx.ReadOnly = true;
            this.tgaFileNameTxBx.Enabled = useTextureChBx.Checked;
            this.tgaFileNameTxBx.KeyDown += new KeyEventHandler(tgaFileNameTxBx_KeyDown);
            // 
            // tgaFileOpenBtn
            // 
            this.tgaFileOpenBtn.Location = new System.Drawing.Point(766, 5);
            this.tgaFileOpenBtn.Name = "tgaFileOpenBtn";
            this.tgaFileOpenBtn.Size = new System.Drawing.Size(65, 23);
            this.tgaFileOpenBtn.TabIndex = 2;
            this.tgaFileNameTxBx.Margin = new Padding(10);
            this.tgaFileOpenBtn.Text = "...";
            this.tgaFileOpenBtn.UseVisualStyleBackColor = true;
            this.tgaFileOpenBtn.Click += new System.EventHandler(this.tgaFileOpenBtn_Click);
            this.tgaFileOpenBtn.Enabled = useTextureChBx.Checked;
            // 
            // useScaleChBx
            // 
            this.useScaleChBx.AutoSize = true;
            this.useScaleChBx.Location = new System.Drawing.Point(5, 10);
            this.useScaleChBx.Name = "useScaleChBx";
            this.useScaleChBx.Size = new System.Drawing.Size(53, 17);
            this.useScaleChBx.TabIndex = 0;
            this.useScaleChBx.Text = "Scale";
            this.useScaleChBx.UseVisualStyleBackColor = true;
            this.useScaleChBx.Click += new EventHandler(useScaleChBx_Click);
            // 
            // scale_U_Label
            // 
            this.scale_U_Label.AutoSize = true;
            this.scale_U_Label.Location = new System.Drawing.Point(58, 12);
            this.scale_U_Label.Name = "scale_U_Label";
            this.scale_U_Label.Size = new System.Drawing.Size(18, 13);
            this.scale_U_Label.TabIndex = 1;
            this.scale_U_Label.Text = "U:";
            this.scale_U_Label.Enabled = useScaleChBx.Checked;
            // 
            // scaleUnumDeciUpDwn
            // 
            this.scaleUnumDeciUpDwn.DecimalPlaces = 4;
            this.scaleUnumDeciUpDwn.Location = new System.Drawing.Point(79, 8);
            this.scaleUnumDeciUpDwn.Increment = this.dIncrement;
            this.scaleUnumDeciUpDwn.Maximum = this.dMaximum;
            this.scaleUnumDeciUpDwn.Minimum = this.dMinimum;
            this.scaleUnumDeciUpDwn.Value = 1;
            this.scaleUnumDeciUpDwn.Name = "scaleUnumDeciUpDwn";
            this.scaleUnumDeciUpDwn.Size = new System.Drawing.Size(78, 20);
            this.scaleUnumDeciUpDwn.TabIndex = 0;
            this.scaleUnumDeciUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.scaleUnumDeciUpDwn.Enabled = useScaleChBx.Checked;
            this.scaleUnumDeciUpDwn.ValueChanged += new EventHandler(numUpDwn_ValueChanged);
            // 
            // scale_V_Label
            // 
            this.scale_V_Label.AutoSize = true;
            this.scale_V_Label.Location = new System.Drawing.Point(168, 12);
            this.scale_V_Label.Name = "scale_V_Label";
            this.scale_V_Label.Size = new System.Drawing.Size(17, 13);
            this.scale_V_Label.TabIndex = 1;
            this.scale_V_Label.Text = "V:";
            this.scale_V_Label.Enabled = useScaleChBx.Checked;
            // 
            // scaleVnumDeciUpDwn
            // 
            this.scaleVnumDeciUpDwn.DecimalPlaces = 4;
            this.scaleVnumDeciUpDwn.Location = new System.Drawing.Point(190, 8);
            this.scaleVnumDeciUpDwn.Increment = this.dIncrement;
            this.scaleVnumDeciUpDwn.Maximum = this.dMaximum;
            this.scaleVnumDeciUpDwn.Minimum = this.dMinimum;
            this.scaleVnumDeciUpDwn.Value = 1;
            this.scaleVnumDeciUpDwn.Name = "scaleVnumDeciUpDwn";
            this.scaleVnumDeciUpDwn.Size = new System.Drawing.Size(78, 20);
            this.scaleVnumDeciUpDwn.TabIndex = 0;
            this.scaleVnumDeciUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.scaleVnumDeciUpDwn.Enabled = useScaleChBx.Checked;
            this.scaleVnumDeciUpDwn.ValueChanged += new EventHandler(numUpDwn_ValueChanged);
            // 
            // useScrollChBx
            // 
            this.useScrollChBx.AutoSize = true;
            this.useScrollChBx.Location = new System.Drawing.Point(6, 10);
            this.useScrollChBx.Name = "useScrollChBx";
            this.useScrollChBx.Size = new System.Drawing.Size(52, 17);
            this.useScrollChBx.TabIndex = 0;
            this.useScrollChBx.Text = "Scroll";
            this.useScrollChBx.Click += new EventHandler(useScrollChBx_Click);
            this.useScrollChBx.UseVisualStyleBackColor = true;
            // 
            // blendType
            // 
            this.blendType.FormattingEnabled = true;
            this.blendType.Items.AddRange(blend_Items);
            this.blendType.Location = new System.Drawing.Point(10, 36);
            this.blendType.Name = "BlendType";
            this.blendType.Size = new System.Drawing.Size(107, 21);
            this.blendType.TabIndex = 2;
            this.blendType.Text = blend_Items[0].ToString();
            this.blendType.Enabled = this.splitContainer1.Enabled;
            this.blendType.Click += new EventHandler(numUpDwn_ValueChanged);
            this.blendType.Visible = isFallBackBlend;
            // 
            // scrollTypeCB
            // 
            this.scrollTypeCB.FormattingEnabled = true;
            this.scrollTypeCB.Items.AddRange(scroll_Items);
            this.scrollTypeCB.Location = new System.Drawing.Point(59, 8);
            this.scrollTypeCB.Name = "scrollTypeCB";
            this.scrollTypeCB.Size = new System.Drawing.Size(107, 21);
            this.scrollTypeCB.TabIndex = 2;
            this.scrollTypeCB.Text = scroll_Items[0].ToString();
            this.scrollTypeCB.Enabled = this.useScrollChBx.Checked;
            // 
            // scroll_U_Label
            // 
            this.scroll_U_Label.AutoSize = true;
            this.scroll_U_Label.Location = new System.Drawing.Point(170, 12);
            this.scroll_U_Label.Name = "scroll_U_Label";
            this.scroll_U_Label.Size = new System.Drawing.Size(18, 13);
            this.scroll_U_Label.TabIndex = 1;
            this.scroll_U_Label.Text = "U:";
            this.scroll_U_Label.Enabled = this.useScrollChBx.Checked;
            // 
            // scrollUnumDeciUpDwn
            // 
            this.scrollUnumDeciUpDwn.DecimalPlaces = 4;
            this.scrollUnumDeciUpDwn.Location = new System.Drawing.Point(190, 8);
            this.scrollUnumDeciUpDwn.Increment = this.dIncrement;
            this.scrollUnumDeciUpDwn.Maximum = this.dMaximum;
            this.scrollUnumDeciUpDwn.Minimum = this.dMinimum;
            this.scrollUnumDeciUpDwn.Name = "scrollUnumDeciUpDwn";
            this.scrollUnumDeciUpDwn.Size = new System.Drawing.Size(78, 20);
            this.scrollUnumDeciUpDwn.TabIndex = 0;
            this.scrollUnumDeciUpDwn.Enabled = this.useScrollChBx.Checked;
            this.scrollUnumDeciUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.scrollUnumDeciUpDwn.ValueChanged += new EventHandler(numUpDwn_ValueChanged);
            // 
            // scroll_V_Label
            // 
            this.scroll_V_Label.AutoSize = true;
            this.scroll_V_Label.Location = new System.Drawing.Point(272, 12);
            this.scroll_V_Label.Name = "scroll_V_Label";
            this.scroll_V_Label.Size = new System.Drawing.Size(17, 13);
            this.scroll_V_Label.TabIndex = 1;
            this.scroll_V_Label.Text = "V:";
            this.scroll_V_Label.Enabled = this.useScrollChBx.Checked;
            // 
            // scrollVnumDeciUpDwn
            // 
            this.scrollVnumDeciUpDwn.DecimalPlaces = 4;
            this.scrollVnumDeciUpDwn.Location = new System.Drawing.Point(293, 8);
            this.scrollVnumDeciUpDwn.Increment = this.dIncrement;
            this.scrollVnumDeciUpDwn.Maximum = this.dMaximum;
            this.scrollVnumDeciUpDwn.Minimum = this.dMinimum;
            this.scrollVnumDeciUpDwn.Name = "scrollVnumDeciUpDwn";
            this.scrollVnumDeciUpDwn.Size = new System.Drawing.Size(78, 20);
            this.scrollVnumDeciUpDwn.TabIndex = 0;
            this.scrollVnumDeciUpDwn.Enabled = this.useScrollChBx.Checked;
            this.scrollVnumDeciUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.scrollVnumDeciUpDwn.ValueChanged += new EventHandler(numUpDwn_ValueChanged);
            // 
            // extraOptionChBx
            // 
            this.extraOptionChBx.AutoSize = true;
            this.extraOptionChBx.Location = new System.Drawing.Point(668, 10);
            this.extraOptionChBx.Name = "extraOptionChBx";
            this.extraOptionChBx.Size = new System.Drawing.Size(98, 17);
            this.extraOptionChBx.TabIndex = 2;
            this.extraOptionChBx.Text = extraOptionName;
            this.extraOptionChBx.Visible = useExtraOptionChBx;
            this.extraOptionChBx.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.extraOptionChBx.UseVisualStyleBackColor = true;
            // 
            // addGlowTintCkBx
            // 
            this.addGlowTintCkBx.AutoSize = true;
            this.addGlowTintCkBx.Location = new System.Drawing.Point(770, 10);
            this.addGlowTintCkBx.Name = "addGlowTintCkBx";
            this.addGlowTintCkBx.Size = new System.Drawing.Size(87, 17);
            this.addGlowTintCkBx.TabIndex = 3;
            this.addGlowTintCkBx.Text = "AddGlowTint";
            this.addGlowTintCkBx.Visible = false;
            this.addGlowTintCkBx.UseVisualStyleBackColor = true;
            // 
            // weightPanel
            // 
            this.weightPanel.Controls.Add(this.weightDecNumUpDwn);
            this.weightPanel.Controls.Add(this.weightChBx);
            this.weightPanel.Location = new System.Drawing.Point(714, 6);
            this.weightPanel.Name = "weightPanel";
            this.weightPanel.Size = new System.Drawing.Size(137, 26);
            this.weightPanel.TabIndex = 0;
            this.weightPanel.Visible = extraOpIsAlpha;
            // 
            // weightChBx
            //
            this.weightChBx.AutoSize = true;
            this.weightChBx.Location = new System.Drawing.Point(4, 4);
            this.weightChBx.Name = "weightChBx";
            this.weightChBx.Size = new System.Drawing.Size(52, 17);
            this.weightChBx.TabIndex = 0;
            this.weightChBx.Text = "Weight";
            this.weightChBx.UseVisualStyleBackColor = true;
            // 
            // weightDecNumUpDwn
            // 
            this.weightDecNumUpDwn.DecimalPlaces = 2;
            this.weightDecNumUpDwn.Increment = this.dIncrement;
            this.weightDecNumUpDwn.Maximum = this.dMaximum;
            this.weightDecNumUpDwn.Minimum = this.dMinimum;
            this.weightDecNumUpDwn.Location = new System.Drawing.Point(60, 3);
            this.weightDecNumUpDwn.Name = "weightDecNumUpDwn";
            this.weightDecNumUpDwn.Size = new System.Drawing.Size(56, 20);
            this.weightDecNumUpDwn.TabIndex = 1;
            this.weightDecNumUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.weightDecNumUpDwn.ValueChanged += new EventHandler(numUpDwn_ValueChanged);
            // 
            // matBlock
            //
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.tgaIconPicBx)).EndInit();
            this.scaleScrollPanel.ResumeLayout(false);
            this.scaleScrollPanel.PerformLayout();
            this.scrollPanel.ResumeLayout(false);
            this.scrollPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.scrollVnumDeciUpDwn)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.scrollUnumDeciUpDwn)).EndInit();
            this.scalePanel.ResumeLayout(false);
            this.scalePanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.scaleVnumDeciUpDwn)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.scaleUnumDeciUpDwn)).EndInit();
            this.texturePanel.ResumeLayout(false);
            this.texturePanel.PerformLayout();
            this.ResumeLayout(false);

        }



        void tgaFileNameTxBx_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyData == Keys.Enter)
            {
                TextBox tb = (TextBox)sender;
                string tbName = tb.Name;

                setTexture(tb.Text.Trim(),texPath,texName);
            }
        }
        #endregion

        public string getTgaFileNameTxBxText()
        {
            return tgaFileNameTxBx.Text;
        }

        public void undo()
        {
            if (dataSnapShot01 != null)
            {
                ArrayList tmpData = new ArrayList();
                getData(ref tmpData);
                string[] tmpPath = { "c:/hello.tga", this.tgaFileNameTxBx.Text };
                string[] tmpName = { "hello.tga", System.IO.Path.GetFileName(this.tgaFileNameTxBx.Text) };
                setData(ref dataSnapShot01, tPath, tName);
                dataSnapShot01.Clear();
                dataSnapShot01 = (ArrayList)tmpData.Clone();
                tmpPath.CopyTo(tPath, 0);
                tmpName.CopyTo(tName, 0);
            }
        }
        
        #region DragNDrop

        private void MatBlock_MouseDown(object sender, MouseEventArgs e)
        {
            ArrayList data = new ArrayList();
            
            this.getData(ref data);
            
            RemotingMatBlock rmb = new RemotingMatBlock(this.Text, this.tgaFileNameTxBx.Text, data);

            this.DoDragDrop(rmb, DragDropEffects.Copy|DragDropEffects.Move);
        }       

        private void processDropedFile(string dropFile)
        {
            if(dropFile.ToLower().EndsWith(".tga"))
            {
                useTextureChBx.Checked = true;
                tgaFileNameTxBx.Text = dropFile;
                setIconImage(dropFile);
                isDirty = true;
                openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(dropFile);
                openFileDialog1.FileName = dropFile;
                useTextureChBx_Click(useTextureChBx, new EventArgs());
            }
        }

        private void scrollMatPanel()
        {
            Panel matTrickPropertiesPanel = (Panel)this.Parent.Parent.Parent;
            
            if (matTrickPropertiesPanel != null)
            {
                matTrickPropertiesPanel.SuspendLayout();

                int scrollVal = matTrickPropertiesPanel.VerticalScroll.Value;

                int scrollMax = matTrickPropertiesPanel.VerticalScroll.Maximum;

                int scrollMin = matTrickPropertiesPanel.VerticalScroll.Minimum;

                Point p = Cursor.Position;

                p = matTrickPropertiesPanel.PointToClient(p);

                if (p.Y - this.Height <= 0)
                {
                    scrollVal -= 10;
                }
                else if (p.Y + this.Height + 10 >= matTrickPropertiesPanel.Height)
                {
                    scrollVal += 10;
                }

                scrollVal = Math.Min(Math.Max(scrollVal, scrollMin), scrollMax);

                matTrickPropertiesPanel.VerticalScroll.Value = scrollVal;

                matTrickPropertiesPanel.ResumeLayout(false);
            }
        }
        
        private void MatBlock_DragLeave(object sender, EventArgs e)
        {
            scrollMatPanel();
        }

        private void MatBlock_DragOver(object sender, DragEventArgs e)
        {
            scrollMatPanel();
        }
        
        private void MatBlock_DragDrop(object sender, DragEventArgs e)
        {
            try
            {
                Array a = (Array)e.Data.GetData(DataFormats.FileDrop);

                RemotingMatBlock mb = (RemotingMatBlock)e.Data.GetData("COH_CostumeUpdater.assetEditor.aeCommon.RemotingMatBlock");
                //MatBlock mb = (MatBlock)e.Data.GetData("COH_CostumeUpdater.assetEditor.aeCommon.MatBlock");
                if (a != null)
                {
                    string dropFileStr = a.GetValue(0).ToString();

                    this.BeginInvoke(m_DelegateProcessDropedFile, new Object[] { dropFileStr });

                    this.Focus();
                }
                if (mb != null)
                {/*
                    ArrayList al = new ArrayList();
                    mb.getData(ref al);

                    string secName = this.Text;
                    string oldSecName = mb.Text;
                    */
                    ArrayList al = mb.getData();

                    string secName = this.Text;
                    string oldSecName = mb.getName();
                    string texPath = mb.getTexturePath();

                    for (int i = 0; i < al.Count; i++)
                    {
                        al[i] = ((string)al[i]).Replace(oldSecName, secName);
                    }
                    string [] tgaPaths = {"c:/hello.tga",texPath};//mb.getTgaFileNameTxBxText()};
                    string[] tgaName = { "Hello.tga", System.IO.Path.GetFileName(texPath) };//mb.getTgaFileNameTxBxText())};
                    
                    dataSnapShot01 = new ArrayList();
                    tPath = new string[] { "c:/hello.tga", this.tgaFileNameTxBx.Text };
                    tName = new string[] {"hello.tga", System.IO.Path.GetFileName(this.tgaFileNameTxBx.Text) };
                    this.getData(ref dataSnapShot01);
                    this.setData(ref al, tgaPaths, tgaName);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
                
        }

        private void MatBlock_DragEnter(object sender, DragEventArgs e)
        {
            scrollMatPanel();

            if (e.Data.GetDataPresent(DataFormats.FileDrop))
                e.Effect = DragDropEffects.Copy;
            else if (e.Data.GetDataPresent("COH_CostumeUpdater.assetEditor.aeCommon.MatBlock"))
                e.Effect = DragDropEffects.All;
            else if (e.Data.GetDataPresent("COH_CostumeUpdater.assetEditor.aeCommon.RemotingMatBlock"))
                e.Effect = DragDropEffects.All;
            else
                e.Effect= DragDropEffects.None;
        }

        #endregion

        private void numUpDwn_ValueChanged(object sender, EventArgs e)
        {
            isDirty = true;
        }

        private void resetFields()
        {
            //blendType.Enabled = false;
            //scrollTypeCB.Enabled = false;
            tgaFileNameTxBx.Enabled = false;
            tgaFileNameTxBx.Text = "";
            tgaFileOpenBtn.Enabled = false;

            useScrollChBx.Checked = false;
            useScaleChBx.Checked = false;
            extraOptionChBx.Checked = false;
            addGlowTintCkBx.Checked = false;
            useTextureChBx.Checked = false;
            weightChBx.Checked = false;

            scale_U_Label.Enabled = false;
            scale_V_Label.Enabled = false;
            scaleUnumDeciUpDwn.Enabled = false;
            scaleVnumDeciUpDwn.Enabled = false;

            scroll_U_Label.Enabled = false;
            scroll_V_Label.Enabled = false;
            scrollUnumDeciUpDwn.Enabled = false;
            scrollVnumDeciUpDwn.Enabled = false;
            scrollTypeCB.Enabled = false;

            scrollVnumDeciUpDwn.Value = 0;
            scrollUnumDeciUpDwn.Value = 0;
            scaleVnumDeciUpDwn.Value = 1;
            scaleUnumDeciUpDwn.Value = 1;
            weightDecNumUpDwn.Value = 1;

            tgaIconPicBx.Image = null;

            //setEnabled(false);
        }
        
        public void setEnabled(bool isEnabled)
        {
            this.splitContainer1.Enabled = isEnabled;
        }

        private void tgaIconPicBx_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            try
            {
                viewTga(tgaFileNameTxBx.Text, true);
            }
            catch (Exception ex)
            {
            }
        }

        private void sendToPhotoShop_menuItem_Click(object sender, System.EventArgs e)
        {
            string path = this.tgaFileNameTxBx.Text;
            if (System.IO.File.Exists(path))
            {
                System.Diagnostics.Process.Start(path);
            }
        }

        private void undoDrag_menuItem_Click(object sender, System.EventArgs e)
        {
            this.undo();
        }

        private string viewTga(string path, bool show)
        {
            string results = "";
            common.TgaBox tbx = new common.TgaBox(path, true);
            if (show)
                tbx.ShowDialog();
            else
            {
                results = tbx.getImageSize();
            }
            return results;
        }

        private void tgaFileOpenBtn_Click(object sender, EventArgs e)
        {
            if (System.IO.File.Exists(tgaFileNameTxBx.Text))
            {
                openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(tgaFileNameTxBx.Text);
            }
            DialogResult dr =  openFileDialog1.ShowDialog();

            if (dr == DialogResult.OK)
            {
                tgaFileNameTxBx.Text = openFileDialog1.FileName;
                setIconImage(openFileDialog1.FileName);
                isDirty = true;
            }
        }
       
        private void setIconImage(string path)
        {
            try
            {
                PresentParameters pp = new PresentParameters();
                pp.Windowed = true;
                pp.SwapEffect = SwapEffect.Copy;
                Device device = new Device(0, DeviceType.Hardware, this.tgaIconPicBx, CreateFlags.HardwareVertexProcessing, pp);
                Microsoft.DirectX.Direct3D.Texture tx = TextureLoader.FromFile(device, path);
                Microsoft.DirectX.GraphicsStream gs = TextureLoader.SaveToStream(ImageFileFormat.Png, tx);

                Bitmap btmap = new Bitmap(gs);

                tgaIconPicBx.Image = btmap;

                gs.Dispose();
                tx.Dispose();
                device.Dispose();
            }
            catch (Exception ex)
            { }
        }
       
        private void useScaleChBx_Click(object sender, EventArgs e)
        {
            scale_U_Label.Enabled = useScaleChBx.Checked;
            scale_V_Label.Enabled = useScaleChBx.Checked;
            scaleUnumDeciUpDwn.Enabled = useScaleChBx.Checked;
            scaleVnumDeciUpDwn.Enabled = useScaleChBx.Checked;
            isDirty = true;
        }

        private void useTextureChBx_Click(object sender, EventArgs e)
        {
            this.tgaFileNameTxBx.Enabled = useTextureChBx.Checked;
            this.tgaFileOpenBtn.Enabled = useTextureChBx.Checked;
            scale_U_Label.Enabled = useTextureChBx.Checked;
            scale_V_Label.Enabled = useTextureChBx.Checked;
            scaleUnumDeciUpDwn.Enabled = useTextureChBx.Checked;
            scaleVnumDeciUpDwn.Enabled = useTextureChBx.Checked;
            scroll_U_Label.Enabled = useTextureChBx.Checked;
            scroll_V_Label.Enabled = useTextureChBx.Checked;
            scrollUnumDeciUpDwn.Enabled = useTextureChBx.Checked;
            scrollVnumDeciUpDwn.Enabled = useTextureChBx.Checked;
            scrollTypeCB.Enabled = useTextureChBx.Checked;
            useScaleChBx.Enabled = useTextureChBx.Checked;
            useScrollChBx.Enabled = useTextureChBx.Checked;
            extraOptionChBx.Enabled = useTextureChBx.Checked;
            if (!useTextureChBx.Checked && 
                extraOptionChBx.Text.Equals("Multiply Reflect") &&
                extraOptionChBx.Checked)
            {
                extraOptionChBx.Checked = useTextureChBx.Checked;
                COH_CostumeUpdaterForm cuf = (COH_CostumeUpdaterForm)this.FindForm();
                if (cuf != null)
                {
                    string msg = "Due to engin bug in Performance Mode “Multiply Reflect” is unchecked when Texture is unchecked!!! (Requested By RR)\r\n";
                    cuf.writeToLogBox(msg, Color.Blue); 
                }
            }
            addGlowTintCkBx.Enabled = useTextureChBx.Checked;
            weightChBx.Enabled = useTextureChBx.Checked;
            weightDecNumUpDwn.Enabled = useTextureChBx.Checked;
            isDirty = true;
        }

        private void useScrollChBx_Click(object sender, EventArgs e)
        {
            scroll_U_Label.Enabled = useScrollChBx.Checked;
            scroll_V_Label.Enabled = useScrollChBx.Checked;
            scrollUnumDeciUpDwn.Enabled = useScrollChBx.Checked;
            scrollVnumDeciUpDwn.Enabled = useScrollChBx.Checked;
            scrollTypeCB.Enabled = useScrollChBx.Checked;
            isDirty = true;
        }


        #region Set Data Functions
        
        private string fixCamelCase(string line, string secName)
        {
            string fixedLine="";
            char [] lineChars = line.ToCharArray();
            char[] secChars = secName.ToCharArray();
            if(line.ToLower().StartsWith((secName + " ").ToLower()))
            {
                for(int i = 0; i < secName.Length; i++)
                {
                    char c = lineChars[i];
                    char ch = secChars[i];
                    {
                        if(ch.ToString().ToLower().Equals(c.ToString().ToLower()))
                        {
                            lineChars[i] = ch;
                        }
                    }
                }
            }
            foreach (char m in lineChars)
            {
                fixedLine += m;
            }

            return fixedLine.Trim();
        }
        
        public void setData(ref ArrayList data, string[] tgafilePaths, string[] tgafileNames)
        {
            if (texPath == null)
            {
                texPath = new string[tgafilePaths.Length];
                tgafilePaths.CopyTo(texPath, 0);
            }
            if (texName == null)
            {
                texName = new string[tgafileNames.Length];
                tgafileNames.CopyTo(texName, 0);
            }
            setData(ref data, tgafilePaths, tgafileNames, false);
        }
        //added to fix a bug that removed the word Mask from tga file name due to the global replace of string.Replace
        private string removeTagName(string line, string secName)
        {
            int startIndex = secName.Length;
            string results = line.Substring(startIndex).Trim();
            return results;
        }

        public void setData(ref ArrayList data, string[] tgafilePaths, string[] tgafileNames,bool tgaPathIsKnown)
        {
            resetFields();

            string secName = this.Text;

            //added to fix Don Pham Bug "AddGlowMat2  1" not followed by "AddGlowTint 0"
            string specialSecName = secName; //special case for addGlow
            
            bool secNameFound = false;

            int i = 0;
            
            foreach (object obj in data)
            {
                Application.DoEvents();
                string line = common.COH_IO.removeExtraSpaceBetweenWords(((string)obj).Replace("\t", " ")).Trim();

                line = line.Replace("//", "#");                
                
                if(line.ToLower().StartsWith((secName + " ").ToLower()))
                {
                    specialSecName = secName;

                    System.Windows.Forms.Application.DoEvents();

                    secNameFound = !secName.ToLower().Equals("Blend".ToLower());

                    string fixedLine = fixCamelCase(line, secName);

                    setTexture(removeTagName(fixedLine,secName + " ").Trim(), tgafilePaths, tgafileNames);

                }
                else if (line.ToLower().StartsWith((secName + "Scroll ").ToLower()))
                {
                    System.Windows.Forms.Application.DoEvents();

                    string fixedLine = fixCamelCase(line, secName+ "Scroll");

                    setUVnumUpDnComboBxOptions(fixedLine.Replace(secName + "Scroll ", "").Trim(), 
                        ref scrollUnumDeciUpDwn,
                        ref scrollVnumDeciUpDwn, 
                        ref useScrollChBx);
                }
                else if (line.ToLower().StartsWith((secName + "ScrollType ").ToLower()))
                {
                    System.Windows.Forms.Application.DoEvents();

                    string fixedLine = fixCamelCase(line, secName+ "ScrollType");

                    setScrollType(fixedLine.Replace(secName + "ScrollType ", "").Trim());
                }
                else if (line.ToLower().StartsWith("BlendType ".ToLower()))
                {
                    System.Windows.Forms.Application.DoEvents();

                    secNameFound = secName.ToLower().Equals("Blend".ToLower());

                    string fixedLine = fixCamelCase(line, "BlendType");

                    string[] lineSplit = fixedLine.Replace("BlendType ", "").Split('#');

                    blendType.SelectedItem = lineSplit[0].Trim();
                }
                else if (line.ToLower().StartsWith((secName + "Scale ").ToLower()))
                {
                    System.Windows.Forms.Application.DoEvents();

                    string fixedLine = fixCamelCase(line, secName + "Scale");

                    setUVnumUpDnComboBxOptions(fixedLine.Replace(secName + "Scale ", "").Trim(),
                        ref scaleUnumDeciUpDwn,
                        ref scaleVnumDeciUpDwn, 
                        ref useScaleChBx);
                }
                else if (line.ToLower().StartsWith((secName + "Reflect ").ToLower()))
                {
                    System.Windows.Forms.Application.DoEvents();

                    string fixedLine = fixCamelCase(line, secName + "Reflect");

                    setExtratOptionData(fixedLine.Replace(secName + "Reflect ", "").Trim());
                }
                else if (line.ToLower().StartsWith("AddGlowMat2 ".ToLower()))
                {
                    System.Windows.Forms.Application.DoEvents();

                    string fixedLine = fixCamelCase(line, "AddGlowMat2");

                    setExtratOptionData(fixedLine.Replace("AddGlowMat2 ", "").Trim());

                    //secNameFound = false;//removed to fix Don Pham Bug "AddGlowMat2  1" not followed by "AddGlowTint 0"
                    specialSecName = "AddGlow";//added to fix Don Pham Bug "AddGlowMat2  1" not followed by "AddGlowTint 0"
                }
                else if (line.ToLower().StartsWith("addGlowTint ".ToLower()))
                {
                    if (secName.Trim().Equals("AddGlow1"))
                    {
                        string fixedLine = fixCamelCase(line, "addGlowTint");

                        string[] lineSplit = fixedLine.ToLower().Replace("addGlowTint ".ToLower(), "").Split('#');

                        string lineWOcomment = lineSplit[0].Trim();

                        if (lineWOcomment.StartsWith("1"))
                            addGlowTintCkBx.Checked = true;
                        else
                            addGlowTintCkBx.Checked = false;

                        secNameFound = true;
                        specialSecName = "AddGlow";//added to fix Don Pham Bug "AddGlowMat2  1" not followed by "AddGlowTint 0"
                    }
                }
                else if (line.ToLower().StartsWith((secName + "Weight ").ToLower()))
                {
                    System.Windows.Forms.Application.DoEvents();

                    string fixedLine = fixCamelCase(line, secName+ "Weight");

                    setExtratOptionData(fixedLine.Replace(secName + "Weight ", "").Trim());
                }
                else if (line.ToLower().StartsWith("AlphaMask ".ToLower()))
                {
                    System.Windows.Forms.Application.DoEvents();

                    string fixedLine = fixCamelCase(line, "AlphaMask");

                    setAlphaMask(fixedLine.Replace("AlphaMask ", "").Trim());
                }

                i++;
                //replaced secName with specialSecName to fix Don Pham Bug "AddGlowMat2  1" not followed by "AddGlowTint 0"
                if (!line.ToLower().Contains(specialSecName.ToLower()) && secNameFound)
                {
                    System.Windows.Forms.Application.DoEvents();
                    return;
                }
            }
            return;
        }

        public void setTexturePath(string path)
        {
            updateTextureProp(path);
        }

        private void setTexture(string line,  string[] tgafilePaths, string[] tgafileNames)
        {
            setTexture(line, tgafilePaths, tgafileNames, false);
        }
        
        private void setTexture(string line, string[] tgafilePaths, string[] tgafileNames, bool tgaPathIsKnown)
        {
            int len = line.Length;

            bool noTexture = line.ToLower().StartsWith("none");

            string tgaFile = ""; 
            
            string [] lineCmtSplit = line.Trim().Split('#');

            string textureName = lineCmtSplit[0].Trim();

            textureName = textureName.ToLower().EndsWith(".tga") ? textureName : textureName + ".tga";

            if ( noTexture )            
            {
                useTextureChBx.Checked = false;

                useTextureChBx_Click(useTextureChBx, new EventArgs());

                if (lineCmtSplit.Length > 1)
                {
                    string commentedTxName = lineCmtSplit[1];

                    commentedTxName = commentedTxName.ToLower().EndsWith(".tga") ? commentedTxName : commentedTxName + ".tga";

                    if (System.IO.File.Exists(commentedTxName))
                    {
                        updateTextureProp(commentedTxName);
                    }
                }
            }
            else
            {
                useTextureChBx.Checked = true;

                tgaFile = common.COH_IO.findTGAPathFast(textureName, tgafilePaths, tgafileNames);

                if (tgaFile == null)
                {
                    tgaFile = textureName;
                }

                updateTextureProp(tgaFile);
            }
        }

        private void updateTextureProp(string tgaFile)
        {
            if (tgaFile != null)
            {
                useTextureChBx_Click(useTextureChBx, new EventArgs());

                tgaFileNameTxBx.Text = tgaFile;

                openFileDialog1.FileName = tgaFile;

                setIconImage(tgaFile);

                if(System.IO.File.Exists(tgaFile))
                    openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(tgaFile);
            }
        }

        private void setUVnumUpDnComboBxOptions(string line, ref NumericUpDown uNupDown, 
            ref NumericUpDown vNupDown, ref CheckBox uvChkBx)
        {
            string[] lineSplit = line.Split('#');

            decimal cmtUd = 0;

            decimal cmtVd = 0;
            
            decimal uD = 0;
            
            decimal vD = 0;

            bool cmtUdConvSucceeded = false;

            bool cmtVdConvSucceeded = false;

            bool decUConvSucceeded = false;

            bool decVConvSucceeded = false;
            
            if(lineSplit.Length > 1)
            {
                string[] commentStrSplit = lineSplit[1].Trim().Split(' ');

                cmtUdConvSucceeded = Decimal.TryParse(commentStrSplit[0], out cmtUd);

                cmtVdConvSucceeded = Decimal.TryParse(commentStrSplit[1], out cmtVd);                
            }

            string[] mainStrSplit = lineSplit[0].Trim().Split(' ');

            decUConvSucceeded = Decimal.TryParse(mainStrSplit[0], out uD);

            decVConvSucceeded = Decimal.TryParse(mainStrSplit[1], out vD);

            if (decUConvSucceeded &&
                decVConvSucceeded)
            {
                if (uD == uNupDown.Value && vD == vNupDown.Value)
                {
                    uvChkBx.Checked = false;
                    if (cmtUdConvSucceeded &&
                        cmtVdConvSucceeded)
                    {
                        uNupDown.Value = cmtUd;

                        vNupDown.Value = cmtVd;
                    }
                }
                else
                {
                    uvChkBx.Checked = true;

                    uvChkBx.Select();

                    foreach (Control ctl in uvChkBx.Parent.Controls)
                    {
                        ctl.Enabled = true;
                    }

                    uNupDown.Value = uD;

                    vNupDown.Value = vD;
                }
            }
        }
        
        private void setScrollType(string line)
        {
            string[] lineSplit = line.Split('#');
            string scrollType = lineSplit[0].Trim();
            //Bounce as scroll type dose not work so only normal is set
            //scrollTypeCB.SelectedItem = scrollType;
            scrollTypeCB.SelectedItem = "Normal";
        }

        private void setAlphaMask(string line)
        {
            string[] lineSplit = line.Split('#');
            string val = lineSplit[0].Trim();
            this.extraOptionChBx.Checked = !val.Equals("0");
        }
       
        private void setExtratOptionData(string line)
        {
            string matFunName = this.Text;
            string[] lineSplit = line.Split('#');
            string val = lineSplit[0].Trim();
            if (matFunName.Equals("Mask"))
            {
                decimal weight = 1;
                bool dConvSuccess = Decimal.TryParse(val, out weight);
                if (dConvSuccess)
                {
                    weightDecNumUpDwn.Value = weight;
                    weightChBx.Checked = (weight != 1);
                }
            }
            else
            {
                if (!val.Equals("0"))
                {
                    extraOptionChBx.Checked = true;
                }
                else
                {
                    extraOptionChBx.Checked = false;
                }
            }

        }
        #endregion

        #region Get Data Functions

        public void getData(ref ArrayList data)
        {
            if (isFallBack)
            {
                getFallBackData(ref data);
            }
            else
            {
                getNoneFallBackData(ref data);
            }
        }

        private void getNoneFallBackData(ref ArrayList data)
        {
            getTextureData(ref data);
            getScrollData(ref data);
            getScaleData(ref data);
            getExtratOptionData(ref data);
        }

        public void getFallBackData(ref ArrayList data)
        {
            getTextureData(ref data);
            if (isFallBackBlend)
            {
                data.Add(string.Format("\t\tBlendType {0}", blendType.Text)); 
            }
        }

        private string getTextureDataStr()
        {
            string tabs = isFallBack ? "\t\t" : "\t";
            string matFunName = this.Text;
            string textureName = System.IO.Path.GetFileNameWithoutExtension(tgaFileNameTxBx.Text);
            string textureValStr = string.Format("{0} //{1}", textureName, tgaFileNameTxBx.Text);
            string textureCommentStr = string.Format("none //{0}", tgaFileNameTxBx.Text);
            string textureStr = useTextureChBx.Checked ? textureValStr : textureCommentStr;
            return string.Format("{0}{1} {2}", tabs, matFunName, textureStr);
        }

        public string getTexturePath()
        {
            return tgaFileNameTxBx.Text;
        }

        public string getName()
        {
            return (string)this.Text.Clone();
        }

        private void getTextureData(ref ArrayList data)
        {
            data.Add(getTextureDataStr());
        }

        private void getScrollData(ref ArrayList data)
        {
            string matFunName = this.Text;
            string scroll_u = scrollUnumDeciUpDwn.Value.ToString();
            string scroll_v = scrollVnumDeciUpDwn.Value.ToString();
            string scrollValStr = string.Format(" {0} {1}", scroll_u, scroll_v);
            string scrollCommentStr = string.Format(" 0 0 //{0}", scrollValStr);
            string scrollStr = useScrollChBx.Checked ? scrollValStr : scrollCommentStr;
            string scrollTypeValStr = scrollTypeCB.Text;
            string scrollTypeCommentStr = string.Format(" Normal //{0}", scrollTypeValStr);
            string scrollTypeStr = useScrollChBx.Checked ? scrollTypeValStr : scrollTypeCommentStr;

            data.Add(string.Format("\t{0}Scroll{1}", matFunName, scrollStr));
            data.Add(string.Format("\t{0}ScrollType {1}", matFunName, scrollTypeStr));
        }

        private void getScaleData(ref ArrayList data)
        {
            string matFunName = this.Text;
            string scale_u = scaleUnumDeciUpDwn.Value.ToString();
            string scale_v = scaleVnumDeciUpDwn.Value.ToString();
            string scaleValStr = string.Format(" {0} {1}", scale_u, scale_v);
            string scaleCommentStr = string.Format(" 1 1 //{0}", scaleValStr);
            string scaleStr = useScaleChBx.Checked ? scaleValStr : scaleCommentStr;
            data.Add(string.Format("\t{0}Scale {1}", matFunName, scaleStr));
        }

        private void getExtratOptionData(ref ArrayList data)
        {
            string matFunName = this.Text;

            int extraOpChBxInt = extraOptionChBx.Checked ? 1 : 0;
            if (matFunName.Equals("Multiply1"))
            {
                data.Add(string.Format("\tMultiply1Reflect {0} // Optional flag indicating this is a spheremap", extraOpChBxInt));
            }
            else if (matFunName.Equals("Multiply2"))
            {
                data.Add(string.Format("\tMultiply2Reflect {0} // Optional flag indicating this is a spheremap", extraOpChBxInt));
            }
            else if (matFunName.Equals("AddGlow1"))
            {
                //set Add Glow Material2 to a value of 1
                data.Add(string.Format("\tAddGlowMat2  {0} //<- applies the addglow effect to the second material instead of the first material", extraOpChBxInt));
                
                if (addGlowTintCkBx.Checked)
                {
                    data.Add("\tAddGlowTint 1");
                }
                else
                {
                    data.Add("\tAddGlowTint 0");
                }
            }
            else if (matFunName.Equals("Mask"))
            {
                string weight_v = weightDecNumUpDwn.Value.ToString();
                string weightValStr = string.Format(" {0}", weight_v);
                string weightCommentStr = string.Format(" 1 //{0}", weightValStr);
                string weightStr = weightChBx.Checked ? weightValStr : weightCommentStr;
                data.Add(string.Format("\t{0}Weight {1}", matFunName, weightStr));
                data.Add(string.Format("\tAlphaMask {0} //Optional flag indicating to use the ALPHA channel", extraOpChBxInt));
                data.Add("\t             // of the Mask texture instead of all 4 channels");
            }
        }
        #endregion 
    }

    class RemotingMatBlock : MarshalByRefObject
    {
        private ArrayList data;
        private string name;
        private string texturePath;

        public RemotingMatBlock(string n, string tPath, ArrayList d)
        {
            data = (ArrayList)d.Clone();
            name = (string)n.Clone();
            texturePath = (string)tPath.Clone();
        }
        public string getName()
        {
            return (string)name.Clone();
        }
        public ArrayList getData()
        {
            return (ArrayList)data.Clone();
        }
        public string getTexturePath()
        {
            return (string)texturePath.Clone();
        }
    }
}
