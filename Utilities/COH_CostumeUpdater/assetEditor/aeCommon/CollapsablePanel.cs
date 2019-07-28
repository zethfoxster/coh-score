using System;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.assetEditor.aeCommon
{
    class CollapsablePanel:Panel
    {
        private System.Windows.Forms.Button colPanelBtn;
        private System.Windows.Forms.Panel colPanelContainer;

        private int containerHeight;

        private System.ComponentModel.IContainer components = null;

        public CollapsablePanel(string name)//, int height)
        {
            InitializeComponent();
            this.colPanelBtn.Text = "[-] " + name;
            this.Name = name.Replace(" ","_").Replace("/","_").Replace("-","_") + "_colPnl";
        }
        public void Add (Control cntrl)
        {
            this.colPanelContainer.Controls.Add(cntrl);
        }
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        private void InitializeComponent()
        {
            this.colPanelContainer = new System.Windows.Forms.Panel();
            this.colPanelBtn = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // colPanel
            // 
            this.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.Controls.Add(this.colPanelContainer);
            this.Controls.Add(this.colPanelBtn);
            this.Location = new System.Drawing.Point(0, 0);
            //this.Name = "colPanel";
            this.TabIndex = 0;
            this.LocationChanged += new System.EventHandler(this.colPanel_LocationChanged);
            // 
            // colPanelContainer
            // 
            this.colPanelContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.colPanelContainer.Location = new System.Drawing.Point(0, 24);
            this.colPanelContainer.Name = "colPanelContainer";
            this.colPanelContainer.TabIndex = 1;
            // 
            // colPanelBtn
            // 
            this.colPanelBtn.Dock = System.Windows.Forms.DockStyle.Top;
            this.colPanelBtn.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.colPanelBtn.Location = new System.Drawing.Point(0, 0);
            this.colPanelBtn.Name = "colPanelBtn";
            this.colPanelBtn.Size = new System.Drawing.Size(1000, 24);
            this.colPanelBtn.TabIndex = 0;
            this.colPanelBtn.Tag = this.colPanelContainer;
            this.colPanelBtn.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.colPanelBtn.UseVisualStyleBackColor = true;
            this.colPanelBtn.Click += new System.EventHandler(this.colPanelBtn_Click);
            // 
            // panelBlock
            // 
            this.Name = "matBlockForm";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        public void setContainerEnable(bool isEnabled)
        {
            foreach (Control ctl in colPanelContainer.Controls)
            {
                if (ctl.GetType() == typeof(materialTrick.FB_GlobalOptionsPnl))
                    ((materialTrick.FB_GlobalOptionsPnl)ctl).Enabled = isEnabled;

                else if(ctl.GetType() == typeof(MatBlock))
                    ((MatBlock)ctl).setEnabled(isEnabled);
            }
        }
        public void fixHeight()
        {
            int colBtnHeight = colPanelBtn.Height;
            this.containerHeight = 20;
            foreach (Control cntrl in colPanelContainer.Controls)
            {
                this.containerHeight += cntrl.Height;
            }

            this.SuspendLayout();

            colPanelContainer.Size = new Size(1000, containerHeight);
            this.ClientSize = new System.Drawing.Size(1000, colBtnHeight + this.containerHeight);

            this.ResumeLayout(false); 
            this.PerformLayout();

        }

        void colPanel_LocationChanged(object sender, System.EventArgs e)
        {
            Panel pnl = ((Panel)sender);
            Panel pC = (Panel)pnl.Tag;
            if (pC != null)
                pC.Location = new Point(pnl.Location.X, pnl.Location.Y + pnl.Height);
        }

        private void colPanelBtn_Click(object sender, EventArgs e)
        {
            Button bt = ((Button)sender);
            Panel pC = (Panel)bt.Tag;
            Panel parent = (Panel)bt.Parent;
            Panel nextCnt = null;

            if (parent.Tag != null)
                nextCnt = (Panel)parent.Tag;

            pC.Visible = !pC.Visible;


            if (pC.Visible)
            {
                bt.Text = bt.Text.Replace("[+]", "[-]");
                parent.Height = bt.Height + this.containerHeight;

                if (nextCnt != null)
                    nextCnt.Location = new Point(parent.Location.X, parent.Bottom);
            }
            else
            {
                bt.Text = bt.Text.Replace("[-]", "[+]");

                parent.Height = bt.Height ;

                if (nextCnt != null)
                    nextCnt.Location = new Point(parent.Location.X, parent.Bottom);
            }
        }

    }
}
