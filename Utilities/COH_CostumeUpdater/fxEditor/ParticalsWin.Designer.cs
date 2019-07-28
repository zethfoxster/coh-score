using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.fxEditor
{
    partial class ParticalsWin
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
            this.part_tabControl = new System.Windows.Forms.TabControl();
            this.partEmitter_tabPage = new System.Windows.Forms.TabPage();
            this.partMotion_tabPage = new System.Windows.Forms.TabPage();
            this.partPartical_tabPage = new System.Windows.Forms.TabPage();
            this.partTextureColor_tabPage = new System.Windows.Forms.TabPage();
            this.tabPage_ContextMenuStrip = new System.Windows.Forms.ContextMenuStrip();
            this.copyTabPageMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pasteTabPageMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.part_tabControl.SuspendLayout();
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
            // part_tabControl
            // 
            this.part_tabControl.Controls.Add(this.partEmitter_tabPage);
            this.part_tabControl.Controls.Add(this.partMotion_tabPage);
            this.part_tabControl.Controls.Add(this.partPartical_tabPage);
            this.part_tabControl.Controls.Add(this.partTextureColor_tabPage);
            this.part_tabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            this.part_tabControl.Location = new System.Drawing.Point(0, 0);
            this.part_tabControl.Name = "part_tabControl";
            this.part_tabControl.SelectedIndex = 0;
            this.part_tabControl.Size = new System.Drawing.Size(width, 343);
            this.part_tabControl.TabIndex = 0;
            this.part_tabControl.ContextMenuStrip = this.tabPage_ContextMenuStrip;
            this.part_tabControl.MouseWheel += new System.Windows.Forms.MouseEventHandler(part_tabControl_MouseWheel);
            // 
            // partEmitter_tabPage
            // 
            this.partEmitter_tabPage.AutoScroll = true;
            this.partEmitter_tabPage.Location = new System.Drawing.Point(4, 22);
            this.partEmitter_tabPage.Name = "partEmitter_tabPage";
            this.partEmitter_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.partEmitter_tabPage.Size = new System.Drawing.Size(width, 317);
            this.partEmitter_tabPage.TabIndex = 0;
            this.partEmitter_tabPage.Text = "Emitter";
            this.partEmitter_tabPage.UseVisualStyleBackColor = true;
            // 
            // partMotion_tabPage
            // 
            this.partMotion_tabPage.AutoScroll = true;
            this.partMotion_tabPage.Location = new System.Drawing.Point(4, 22);
            this.partMotion_tabPage.Name = "partMotion_tabPage";
            this.partMotion_tabPage.Padding = new System.Windows.Forms.Padding(3);
            this.partMotion_tabPage.Size = new System.Drawing.Size(width, 317);
            this.partMotion_tabPage.TabIndex = 1;
            this.partMotion_tabPage.Text = "Motion";
            this.partMotion_tabPage.UseVisualStyleBackColor = true;
            // 
            // partPartical_tabPage
            // 
            this.partPartical_tabPage.AutoScroll = true;
            this.partPartical_tabPage.Location = new System.Drawing.Point(4, 22);
            this.partPartical_tabPage.Name = "partPartical_tabPage";
            this.partPartical_tabPage.Size = new System.Drawing.Size(width, 317);
            this.partPartical_tabPage.TabIndex = 2;
            this.partPartical_tabPage.Text = "Particle";
            this.partPartical_tabPage.UseVisualStyleBackColor = true;
            // 
            // partTextureColor_tabPage
            // 
            this.partTextureColor_tabPage.AutoScroll = true;
            this.partTextureColor_tabPage.Location = new System.Drawing.Point(4, 22);
            this.partTextureColor_tabPage.Name = "partTextureColor_tabPage";
            this.partTextureColor_tabPage.Size = new System.Drawing.Size(width, 317);
            this.partTextureColor_tabPage.TabIndex = 3;
            this.partTextureColor_tabPage.Text = "Texture/Color";
            this.partTextureColor_tabPage.UseVisualStyleBackColor = true;
            // 
            // BehaviorWin
            //
            this.ClientSize = new System.Drawing.Size(width, 343);
            this.Controls.Add(this.part_tabControl);
            this.Name = "ParticalWin";
            this.Text = "ParticalWin";
            this.part_tabControl.ResumeLayout(false);
            this.ResumeLayout(false);

        }
       
        #endregion

        private System.Windows.Forms.TabControl part_tabControl;
        private System.Windows.Forms.TabPage partEmitter_tabPage;
        private System.Windows.Forms.TabPage partMotion_tabPage;
        private System.Windows.Forms.TabPage partPartical_tabPage;
        private System.Windows.Forms.TabPage partTextureColor_tabPage;

        private System.Windows.Forms.ContextMenuStrip tabPage_ContextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem copyTabPageMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pasteTabPageMenuItem;
    }
}
