using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.assetsMangement
{
    public partial class P4SubmitWin : Form
    {
        public P4SubmitWin()
        {
            InitializeComponent();
        }
        public DialogResult showSubmitWin(string path, out bool keepCheckedOut, out string descriptionStr)
        {
            this.fileName_label.Text = System.IO.Path.GetFileName(path);
            this.filePath_label.Text = path;
            DialogResult dr = this.ShowDialog();
            keepCheckedOut = this.keepCheckedOut_ckbx.Checked;
            descriptionStr = (string)this.description_textBox.Text.Trim().Clone();
            return dr;
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            if ( this.description_textBox.Text.Trim().Length > 5 )
            {
                this.submit_btn.Enabled = true;
            }
            else
            {
                this.submit_btn.Enabled = false;
            }
        }
    }
}
