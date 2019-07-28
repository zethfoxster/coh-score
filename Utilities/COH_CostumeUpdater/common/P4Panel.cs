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
    public partial class P4Panel : Panel
        //Form
    {
        public P4Panel()
        {
            InitializeComponent();
        }

        private void p4DepotDiff_btn_Click(object sender, System.EventArgs e)
        {
            string fileName = getFileName();

            if (ModifierKeys == Keys.Control)
                assetsMangement.CohAssetMang.p4vDiffDialog(fileName);
            else
                assetsMangement.CohAssetMang.p4vDepotDiff(fileName);
        }

        private void p4Add_btn_Click(object sender, System.EventArgs e)
        {
            string fileName = getFileName();

            string results = assetsMangement.CohAssetMang.addP4(fileName);

            MessageBox.Show(results);
        }

        private void p4CheckOut_btn_Click(object sender, System.EventArgs e)
        {
            string fileName = getFileName();

            string results = assetsMangement.CohAssetMang.checkout(fileName);

            MessageBox.Show(results);
        }

        private void p4CheckIn_btn_Click(object sender, System.EventArgs e)
        {
            string fileName = getFileName();

            string results = assetsMangement.CohAssetMang.p4CheckIn(fileName);

            if (results.ToLower().Contains("no files to submit"))
                results = fileName + " - unchanged, reverted\r\n" + results;

            MessageBox.Show(results);
        }

        private string getFileName()
        {
            COH_CostumeUpdaterForm cuf = (COH_CostumeUpdaterForm)this.FindForm();
            if (cuf != null)
                return cuf.currentLoadedFileName;

            return "";
        }
    }
}
