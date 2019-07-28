using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.auras
{
    public partial class AurasPropagator : Panel//Form
    {
        public AurasPropagator(string file_name, ref System.Windows.Forms.RichTextBox rTextBx)
        {
            this.fileName = file_name;
            this.logTxBx = rTextBx;
            InitializeComponent();
            loadRegionCTM();
        }
        
        private bool validateUser()
        {
            string valid_aurasToolStripMenuItem_User = "user";
            string valid_aurasToolStripMenuItem_User2 = "user";

            System.Security.Principal.WindowsIdentity user;

            user = System.Security.Principal.WindowsIdentity.GetCurrent();
            
            if (user.Name.ToLower().EndsWith(valid_aurasToolStripMenuItem_User))
            {
                return true;
            }
            else if (user.Name.ToLower().EndsWith(valid_aurasToolStripMenuItem_User2))
            {
                return true;
            }
            else if (user.Name.ToLower().EndsWith("user"))
            {
                return true;
            }

            return false;
        }

        private void loadRegionCTM()
        {
            Regions r1 = new Regions(this.fileName);

            this.AurasTV_lable.Text = r1.filePath;

            COH_CostumeUpdater.auras.processAuras pa = new COH_CostumeUpdater.auras.processAuras(r1.filePath);

            this.logTxBx.SelectionColor = this.logTxBx.ForeColor;

            this.logTxBx.SelectedText = pa.createTreeViewNodes(r1, this.AurasTV, this.CostumeTV_NodeMouseDoubleClick);

            this.tvContextMenu.Popup += new EventHandler(tvContextMenue_Popup);
        }

        private void propagateAuras_Click(object sender, EventArgs e)
        {
            COH_CostumeUpdater.auras.PropagationForm propForm = new COH_CostumeUpdater.auras.PropagationForm();
            propForm.ShowDialog();
            System.Collections.ArrayList chkd = new System.Collections.ArrayList();

            foreach (object item in propForm.checkedItems)
            {
                chkd.Add(item.ToString());
            }

            propForm.Close();

            propForm.Dispose();

            string loadedFile = AurasTV_lable.Text;

            if (loadedFile.Length > 2 &&
                !loadedFile.Equals("Current File:") &&
                loadedFile.ToLower().EndsWith("regions.ctm"))
            {
                COH_CostumeUpdater.auras.processAuras pa = new COH_CostumeUpdater.auras.processAuras(loadedFile);

                this.logTxBx.SelectionColor = this.logTxBx.ForeColor;

                this.logTxBx.SelectedText = pa.copyAura(chkd);
            }

            else
            {
                MessageBox.Show("You need to load \"Regions.ctm\" file before Propagation!", "Warrning");
            }
        }

        void buildCostumeTVcontextMenu()
        {
            this.tvContextMenu.MenuItems.Clear();

            System.Windows.Forms.MenuItem propagateAuras = new System.Windows.Forms.MenuItem();

            propagateAuras.Name = "propagateAuras";

            propagateAuras.Text = "Propagate Auras";

            propagateAuras.Click += new EventHandler(propagateAuras_Click);

            this.tvContextMenu.MenuItems.Add(propagateAuras);
        }

        void tvContextMenue_Popup(object sender, EventArgs e)
        {

            Point p = this.AurasTV.PointToClient(Control.MousePosition);
            TreeNode tn = this.AurasTV.GetNodeAt(p);
            this.AurasTV.SelectedNode = tn;
            if (validateUser())
            {
                buildCostumeTVcontextMenu();
            }
            else
            {
                this.tvContextMenu.MenuItems.Clear();
            }            
        }

        void CostumeTV_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            try
            {
                if (e.Node.Tag != null)
                {
                    if (File.Exists(e.Node.Tag.ToString()))
                    {
                        System.Diagnostics.Process.Start(e.Node.Tag.ToString());
                    }
                    else
                    {
                        MessageBox.Show("File Does not Exists!", "Error");
                    }
                }
            }

            catch (Exception ex)
            {
                logTxBx.SelectionColor = logTxBx.ForeColor;

                logTxBx.SelectedText = ex.Message + "\r\n";
            }
        }
    }
}
