using System;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    public partial class GeomSelectionForm : Form
    {
        public string geomList;

        public GeomSelectionForm()
        {
            geomList = "";
            InitializeComponent();
        }

        public void builGemoList(ArrayList geoms)
        {
            int x = 10;
            int y = 10;

            foreach (string geo in geoms)
            {
                CheckBox checkBox = new System.Windows.Forms.CheckBox();

                checkBox.AutoSize = true;

                checkBox.Location = new System.Drawing.Point(x, y);

                checkBox.Name = geo + "_CkBx";

                checkBox.Text = geo;

                checkBox.CheckedChanged += new EventHandler(checkBox_CheckedChanged);

                this.panel1.Controls.Add(checkBox);

                y += checkBox.Height + 10;
            }
        }

        void checkBox_CheckedChanged(object sender, EventArgs e)
        {
            geomList = "";

            foreach(CheckBox ck in this.panel1.Controls)
            {
                if (ck.Checked)
                    geomList += ck.Text + "; ";
            }

            geomList = geomList.Trim();
        }

        private void ok_btn_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.OK;
            this.Hide();
        }

        private void cancel_btn_Click(object sender, EventArgs e)
        {
            this.Hide();
        }
    }
}
