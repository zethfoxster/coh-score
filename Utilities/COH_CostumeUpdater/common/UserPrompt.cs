using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.common
{
    public partial class UserPrompt : Form
    {
        public string userInput;

        public UserPrompt()
        {
            InitializeComponent();
        }
        public UserPrompt(string title)
        {
            InitializeComponent();
            this.Text = title;
        }

        public UserPrompt(string title, string message)
        {
            InitializeComponent();
            this.Text = title;
            this.displayMessageLable.Text = message;
        }

        public UserPrompt(string title, string message, string input_lable)
        {
            InitializeComponent();
            this.Text = title;
            this.displayMessageLable.Text = message;
            this.inputLabel.Text = input_lable;
        }

        public void setTextBoxText(string valStr)
        {
            this.inputTextBox.Text = valStr;
        }

        public void hideTxBxNLable(ref TextBox altTxBx,Panel ctl)
        {
            this.prompSplitContainer.Panel1.Controls.Clear();

            this.Width = ctl.Width + 10;
            
            this.MaximumSize = new Size(ctl.Width + 10, this.Height);

            this.MinimumSize = this.MaximumSize;

            this.prompSplitContainer.Panel1.Controls.Add(ctl);

            this.prompSplitContainer.Panel1.Size = new Size(ctl.Width + 10, ctl.Height + 10); 

            this.prompSplitContainer.Panel1MinSize = ctl.Height + 10;

            ctl.Dock = DockStyle.Fill;
            
            this.inputTextBox = altTxBx;
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            userInput = this.inputTextBox.Text;
        }

        void formClosing(object sender, System.Windows.Forms.FormClosingEventArgs e)
        {
            if (this.inputTextBox.Text != null)
                this.okButton_Click(this.okButton, new System.EventArgs());
        }
    }
}
