using System;
using System.Windows.Forms;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.costume
{
    class CostumeUserInput:Panel
    {
        //private System.Windows.Forms.Panel cuiPanel; 
        private System.Windows.Forms.Label cuiLabel;
        private System.Windows.Forms.TextBox cuiTextBox;
        private System.Windows.Forms.Button bButton;
        private System.Windows.Forms.Button addBtn;
        private int yLocation;
        private int mWidth;
        private bool isFX;
        private bool hasAdd;
        private string rootPath;
        public string labelName;

        public CostumeUserInput(string rootDir, string cuiName, string label, int cuiYLocation, int width, bool isFxUI, bool has_add, EventHandler addBEH)
        {
            this.rootPath = rootDir;
            this.Name = cuiName;
            this.yLocation = cuiYLocation;
            this.labelName = label;
            this.isFX = isFxUI;
            this.hasAdd = has_add;
            this.mWidth = width - 4;
            InitializeComponent(addBEH);
        }
        private void InitializeComponent(EventHandler addBtnEventHandler)
        {           
            this.cuiLabel = new System.Windows.Forms.Label();
            this.cuiTextBox = new System.Windows.Forms.TextBox();

            if(isFX)
                this.bButton = new System.Windows.Forms.Button();

            if (hasAdd)
                this.addBtn = new System.Windows.Forms.Button();

            this.SuspendLayout();

            int width = this.isFX ? (this.mWidth - 240):(this.mWidth - 160);
            // 
            // KeyLable
            //
            this.cuiLabel.AutoSize = true;
            this.cuiLabel.Enabled = false;
            //this.cuiLabel.Anchor = AnchorStyles.Left;
            this.cuiLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.cuiLabel.Location = new System.Drawing.Point(3, 12);
            this.cuiLabel.Name = "cuiLable";
            this.cuiLabel.Size = new System.Drawing.Size(32, 17);
            this.cuiLabel.TabIndex = 100;
            this.cuiLabel.Text = this.labelName;
            this.cuiLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // addBtn
            // 
            if (hasAdd)
            {
                this.addBtn.BackColor = System.Drawing.SystemColors.Control;
                this.addBtn.FlatStyle = System.Windows.Forms.FlatStyle.Standard;
                this.addBtn.Image = global::COH_CostumeUpdater.Properties.Resources.Add1;
                this.addBtn.ImageAlign = System.Drawing.ContentAlignment.MiddleCenter;
                this.addBtn.Location = new System.Drawing.Point(100, 12);
                this.addBtn.Margin = new System.Windows.Forms.Padding(0);
                this.addBtn.Name = "addBtn";
                this.addBtn.Size = new System.Drawing.Size(20, 20);
                this.addBtn.TabStop = false;
                this.addBtn.TabIndex = 100;
                this.addBtn.UseVisualStyleBackColor = false;
                this.addBtn.Tag = this.cuiLabel.Text;
                this.addBtn.Click += new EventHandler(addBtnEventHandler);
            }
            // 
            // KeyTextBox
            // 
            this.cuiTextBox.AllowDrop = true;
            this.cuiTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)(System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right));
            this.cuiTextBox.Location = new System.Drawing.Point(128, 12);
            this.cuiTextBox.Name = "cuiTextBox";
            this.cuiTextBox.Size = new System.Drawing.Size(width, 20);
            this.cuiTextBox.TabIndex = 6;
            this.cuiTextBox.Text = "";
            this.cuiTextBox.WordWrap = false;
            //
            // bButto
            //
            if (isFX)
            {
                this.bButton.Anchor = ((System.Windows.Forms.AnchorStyles)(System.Windows.Forms.AnchorStyles.Right));
                this.bButton.Location = new System.Drawing.Point(width + 130, 11);
                this.bButton.Text = "Browse";
                this.bButton.Size = new System.Drawing.Size(80, 23);
                this.bButton.TabStop = false;
                this.bButton.TabIndex = 330;
                this.bButton.UseVisualStyleBackColor = true;
                this.bButton.Click += new EventHandler(bButton_Click);                
            }
            // 
            // keyPanel
            // 
            if (isFX)
                this.Controls.Add(this.bButton);

            if (hasAdd)
                this.Controls.Add(this.addBtn);

            this.Controls.Add(this.cuiTextBox);
            this.Controls.Add(this.cuiLabel);
            this.Location = new System.Drawing.Point(14, this.yLocation);
            this.Size = new System.Drawing.Size(this.mWidth, 45);
            this.TabIndex = 31;                                
            this.ResumeLayout(false);
            this.PerformLayout();
        }

        void bButton_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.InitialDirectory = this.rootPath + @"\data\Fx";
            ofd.Filter = "FX Files (*.FX) |*.FX";

            DialogResult dr = ofd.ShowDialog();
            if (dr == DialogResult.OK)
            {
                string fileName = ofd.FileName;
                this.cuiTextBox.Text = fileName;
            }        
        }

        public string getCUITextBox()
        {
            return this.cuiTextBox.Text;
        }
        public void setCUITextBox(string str)
        {
            this.cuiTextBox.Text = str;
        }
    }
}
