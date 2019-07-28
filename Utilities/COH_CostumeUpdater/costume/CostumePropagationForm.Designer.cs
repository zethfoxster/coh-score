namespace COH_CostumeUpdater.costume
{
    partial class CostumePropagationForm
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
            this.propTV = new System.Windows.Forms.TreeView();
            this.cPropImageList = new System.Windows.Forms.ImageList(this.components);
            this.showOptionWin = new System.Windows.Forms.CheckBox();
            this.propPanel = new System.Windows.Forms.Panel();
            this.propagateBtn = new System.Windows.Forms.Button();
            this.processingFeedBack = new System.Windows.Forms.TextBox();
            this.propToolTip = new System.Windows.Forms.ToolTip(this.components);
            this.propPBar = new System.Windows.Forms.ProgressBar();
            this.feedBackPanel = new System.Windows.Forms.Panel();
            this.propPanel.SuspendLayout();
            this.feedBackPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // propTV
            // 
            this.propTV.CheckBoxes = true;
            this.propTV.Dock = System.Windows.Forms.DockStyle.Fill;
            this.propTV.ImageIndex = 0;
            this.propTV.ImageList = this.cPropImageList;
            this.propTV.Location = new System.Drawing.Point(0, 0);
            this.propTV.Name = "propTV";
            this.propTV.SelectedImageIndex = 0;
            this.propTV.ShowNodeToolTips = true;
            this.propTV.Size = new System.Drawing.Size(628, 464);
            this.propTV.TabIndex = 0;
            this.propTV.NodeMouseClick += new System.Windows.Forms.TreeNodeMouseClickEventHandler(this.propTV_NodeMouseClick);
            // 
            // cPropImageList
            // 
            this.cPropImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth8Bit;
            this.cPropImageList.ImageSize = new System.Drawing.Size(16, 16);
            this.cPropImageList.TransparentColor = System.Drawing.Color.Transparent;
            // 
            // showOptionWin
            // 
            this.showOptionWin.AutoSize = true;
            this.showOptionWin.Location = new System.Drawing.Point(12, 6);
            this.showOptionWin.Name = "showOptionWin";
            this.showOptionWin.Size = new System.Drawing.Size(134, 17);
            this.showOptionWin.TabIndex = 1;
            this.showOptionWin.Text = "Show Options Window";
            this.showOptionWin.UseVisualStyleBackColor = true;
            // 
            // propPanel
            // 
            this.propPanel.Controls.Add(this.feedBackPanel);
            this.propPanel.Controls.Add(this.propagateBtn);
            this.propPanel.Controls.Add(this.showOptionWin);
            this.propPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.propPanel.Location = new System.Drawing.Point(0, 464);
            this.propPanel.Name = "propPanel";
            this.propPanel.Size = new System.Drawing.Size(628, 207);
            this.propPanel.TabIndex = 2;
            // 
            // propagateBtn
            // 
            this.propagateBtn.Location = new System.Drawing.Point(163, 3);
            this.propagateBtn.Name = "propagateBtn";
            this.propagateBtn.Size = new System.Drawing.Size(430, 23);
            this.propagateBtn.TabIndex = 3;
            this.propagateBtn.Text = "Propagate Selection";
            this.propagateBtn.UseVisualStyleBackColor = true;
            this.propagateBtn.Click += new System.EventHandler(this.propagateBtn_Click);
            // 
            // processingFeedBack
            // 
            this.processingFeedBack.Dock = System.Windows.Forms.DockStyle.Fill;
            this.processingFeedBack.Location = new System.Drawing.Point(0, 0);
            this.processingFeedBack.Multiline = true;
            this.processingFeedBack.Name = "processingFeedBack";
            this.processingFeedBack.ReadOnly = true;
            this.processingFeedBack.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.processingFeedBack.Size = new System.Drawing.Size(628, 161);
            this.processingFeedBack.TabIndex = 2;
            // 
            // propToolTip
            // 
            this.propToolTip.AutoPopDelay = 50;
            this.propToolTip.InitialDelay = 50;
            this.propToolTip.ReshowDelay = 100;
            this.propToolTip.ShowAlways = true;
            // 
            // propPBar
            // 
            this.propPBar.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.propPBar.Location = new System.Drawing.Point(0, 161);
            this.propPBar.Name = "propPBar";
            this.propPBar.Size = new System.Drawing.Size(628, 17);
            this.propPBar.TabIndex = 4;
            // 
            // feedBackPanel
            // 
            this.feedBackPanel.Controls.Add(this.processingFeedBack);
            this.feedBackPanel.Controls.Add(this.propPBar);
            this.feedBackPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.feedBackPanel.Location = new System.Drawing.Point(0, 29);
            this.feedBackPanel.Name = "feedBackPanel";
            this.feedBackPanel.Size = new System.Drawing.Size(628, 178);
            this.feedBackPanel.TabIndex = 5;
            // 
            // CostumePropagationForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(628, 671);
            this.Controls.Add(this.propTV);
            this.Controls.Add(this.propPanel);
            this.Name = "CostumePropagationForm";
            this.Text = "Costume Propagation";
            this.propPanel.ResumeLayout(false);
            this.propPanel.PerformLayout();
            this.feedBackPanel.ResumeLayout(false);
            this.feedBackPanel.PerformLayout();
            this.ResumeLayout(false);

        }

  
        #endregion
        public string geosetName;
        private string sourceType;
        private string destType;
        private System.Collections.ArrayList fileContent;
        private System.Collections.ArrayList checkedItems;
        private System.Collections.ArrayList insertionBlock;
        private System.Windows.Forms.ListViewItem[] lvItems;
        private string fileName;
        private int gSetStartIndex;
        private int gSetEndIndex;
        private int startIndex;
        private int lastIndex;
        private int blockLength;
        private System.Windows.Forms.TreeView propTV;
        private System.Windows.Forms.ImageList cPropImageList;
        private System.Windows.Forms.CheckBox showOptionWin;
        private System.Windows.Forms.Panel propPanel;
        private System.Windows.Forms.TextBox processingFeedBack;
        private System.Windows.Forms.Button propagateBtn;
        private System.Windows.Forms.ToolTip propToolTip;
        private System.Windows.Forms.Panel feedBackPanel;
        private System.Windows.Forms.ProgressBar propPBar;
    }
}