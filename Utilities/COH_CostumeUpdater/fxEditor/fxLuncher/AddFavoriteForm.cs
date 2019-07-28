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
    public partial class AddFavoriteForm : Form
    {
        public string favFx;

        public AddFavoriteForm(string fxPath)
        {
            favFx = fxPath;
            InitializeComponent();
            this.name_txtBx.Text = fxPath;
        }

        private void newFolder_btn_Click(object sender, System.EventArgs e)
        {
            AddFavFolderForm addFavFolder = new AddFavFolderForm();
            DialogResult dr = addFavFolder.ShowDialog();
            if (dr == DialogResult.OK)
            {
                this.favorite_comboBox.Items.Add(addFavFolder.folderName);
                this.favorite_comboBox.Text = addFavFolder.folderName;
            }
        }

    }
}
