using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace FxLauncher
{
    public partial class AddFavFolderForm : Form
    {
        public string folderName;

        public AddFavFolderForm()
        {
            folderName = null;
            InitializeComponent();
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            string tx = textBox1.Text.Trim();
            if (tx.Length > 0)
            {
                create_btn.Enabled = true;
            }
        }

        private void create_btn_Click(object sender, EventArgs e)
        {
            folderName = textBox1.Text.Trim();
        }

    }
}
