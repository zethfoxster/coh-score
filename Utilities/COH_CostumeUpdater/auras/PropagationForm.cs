using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.auras
{
    public partial class PropagationForm : Form
    {
        public PropagationForm()
        {
            InitializeComponent();
            checkedItems = new System.Collections.ArrayList();
            for (int i = 0; i < propagationChBxList.Items.Count; i++) 
            {
                propagationChBxList.SetItemChecked(i, true);
            }
        }

        private void propagateFiles_Click(object sender, EventArgs e)
        {
            checkedItems.Clear();
            foreach (object item in propagationChBxList.Items)
            {
                CheckState ckState = propagationChBxList.GetItemCheckState(propagationChBxList.Items.IndexOf(item));
                if (ckState == CheckState.Checked)
                    checkedItems.Add(item.ToString());
            }
                
            this.Hide();
        }
    }
}
