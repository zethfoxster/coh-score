namespace COH_CostumeUpdater.fxEditor
{
    partial class FX_FileWin
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
            this.fxHsplitContainer = new System.Windows.Forms.SplitContainer();
            this.parBhvrVsplitContainer = new System.Windows.Forms.SplitContainer();
            this.fxHsplitContainer.Panel1.SuspendLayout();
            this.fxHsplitContainer.Panel2.SuspendLayout();
            this.fxHsplitContainer.SuspendLayout();
            this.parBhvrVsplitContainer.Panel1.SuspendLayout();
            this.parBhvrVsplitContainer.Panel2.SuspendLayout();
            this.parBhvrVsplitContainer.SuspendLayout();
            this.SuspendLayout();
            // 
            // fxHsplitContainer
            // 
            this.fxHsplitContainer.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.fxHsplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.fxHsplitContainer.Location = new System.Drawing.Point(0, 0);
            this.fxHsplitContainer.Name = "fxHsplitContainer";
            this.fxHsplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // fxHsplitContainer.Panel1
            // 
            this.fxHsplitContainer.Panel1.Controls.Add(this.fxWin);
            // 
            // fxHsplitContainer.Panel2
            // 
            this.fxHsplitContainer.Panel2.Controls.Add(this.parBhvrVsplitContainer);
            this.fxHsplitContainer.Size = new System.Drawing.Size(1183, 724);
            this.fxHsplitContainer.SplitterDistance = 351;
            this.fxHsplitContainer.SplitterWidth = 20;
            this.fxHsplitContainer.TabIndex = 0;
            // 
            // fxWin
            // 
            this.fxWin.Dock = System.Windows.Forms.DockStyle.Fill;
            this.fxWin.Location = new System.Drawing.Point(0, 0);
            this.fxWin.Name = "fxWin";
            this.fxWin.Size = new System.Drawing.Size(1179, 347);
            this.fxWin.TabIndex = 0;
            this.fxWin.Text = "FxWin";
            // 
            // parBhvrVsplitContainer
            // 
            this.parBhvrVsplitContainer.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.parBhvrVsplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.parBhvrVsplitContainer.Location = new System.Drawing.Point(0, 0);
            this.parBhvrVsplitContainer.Name = "parBhvrVsplitContainer";
            // 
            // parBhvrVsplitContainer.Panel1
            // 
            this.parBhvrVsplitContainer.Panel1.Controls.Add(this.partWin);
            // 
            // parBhvrVsplitContainer.Panel2
            // 
            this.parBhvrVsplitContainer.Panel2.Controls.Add(this.bhvrWin);
            this.parBhvrVsplitContainer.Size = new System.Drawing.Size(1183, 353);
            this.parBhvrVsplitContainer.SplitterDistance = 562;
            this.parBhvrVsplitContainer.SplitterWidth = 20;
            this.parBhvrVsplitContainer.TabIndex = 0;
            // 
            // partWin
            // 
            this.partWin.Dock = System.Windows.Forms.DockStyle.Fill;
            this.partWin.Location = new System.Drawing.Point(0, 0);
            this.partWin.Name = "partWin";
            this.partWin.Size = new System.Drawing.Size(558, 349);
            this.partWin.TabIndex = 0;
            this.partWin.Text = "ParticalWin";
            // 
            // bhvrWin
            // 
            this.bhvrWin.Dock = System.Windows.Forms.DockStyle.Fill;
            this.bhvrWin.Location = new System.Drawing.Point(0, 0);
            this.bhvrWin.Name = "bhvrWin";
            this.bhvrWin.Size = new System.Drawing.Size(597, 349);
            this.bhvrWin.TabIndex = 0;
            this.bhvrWin.Text = "BehaviorWin";
            // 
            // FX_FileWin
            // 
            this.ClientSize = new System.Drawing.Size(1183, 724);
            this.Controls.Add(this.fxHsplitContainer);
            this.Name = "FX_FileWin";
            this.Text = "FX_FileWin";
            this.fxHsplitContainer.Panel1.ResumeLayout(false);
            this.fxHsplitContainer.Panel2.ResumeLayout(false);
            this.fxHsplitContainer.ResumeLayout(false);
            this.parBhvrVsplitContainer.Panel1.ResumeLayout(false);
            this.parBhvrVsplitContainer.Panel2.ResumeLayout(false);
            this.parBhvrVsplitContainer.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion
        private System.Windows.Forms.SplitContainer fxHsplitContainer;
        private System.Windows.Forms.SplitContainer parBhvrVsplitContainer;
        private COH_CostumeUpdater.fxEditor.BehaviorWin bhvrWin;
        private COH_CostumeUpdater.fxEditor.ParticalsWin partWin;
        private COH_CostumeUpdater.fxEditor.FXWin fxWin;
    }
}