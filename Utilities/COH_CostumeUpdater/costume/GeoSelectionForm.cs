using System;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.costume
{
    public partial class GeoSelectionForm : Form
    {
        public GeoSelectionForm()
        {
            InitializeComponent();
            okClicked = false;
            adjustColumnsSize();
        }

        public bool showGeoSelectionBox(ref System.Windows.Forms.ListViewItem [] lvItems)
        {
            
            addList(ref lvItems);
            this.ShowDialog();

            
            return okClicked;
        }

        private void adjustColumnsSize()
        {
            int typeClWidth = 100;
            int width = this.listView1.Width - typeClWidth;
            int oneThirdWidth = (int)((float)width / 3.0f);

            this.listView1.Columns[0].Width = oneThirdWidth * 2;
            this.listView1.Columns[1].Width = typeClWidth;
            this.listView1.Columns[2].Width = width - this.listView1.Columns[0].Width;
        }

        void okBtn_Click(object sender, System.EventArgs e)
        {
            okClicked = true;
            this.Close();
        }

        private void addList(ref System.Windows.Forms.ListViewItem[] lvItems)
        {
            this.listView1.Items.AddRange(lvItems);
        }

        private void fillComboBox(ArrayList cbItems)
        {
            this.lvComboBox.Items.Clear();
            foreach (object o in cbItems)
            {
                string str = (string)o;
                this.lvComboBox.Items.Add(str);
            }
        }

        void listView1_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            // Get the item
            lvItem = this.listView1.GetItemAt(e.X, e.Y);

            if (lvItem != null)
            {
                fillComboBox((ArrayList)lvItem.Tag);

                // Get the bounds
                System.Drawing.Rectangle ClickedItem = lvItem.SubItems[2].Bounds;

                //if cell out of view do nothing
                if ((ClickedItem.Left + this.listView1.Columns[2].Width) < 0)
                {
                    return;
                }

                // if cell is in view
                else if (ClickedItem.Left < 0)
                {
                    // Determine if column extends beyond right side of ListView.
                    if ((ClickedItem.Left + this.listView1.Columns[2].Width) > this.listView1.Width)
                    {
                        //match cb width to cell width
                        ClickedItem.Width = this.listView1.Width;
                    }
                    else
                    {
                        ClickedItem.Width = this.listView1.Columns[2].Width + ClickedItem.Left;
                    }
                }
                else if (this.listView1.Columns[2].Width > this.listView1.Width)
                {
                    ClickedItem.Width = this.listView1.Width;
                }
                else
                {
                    ClickedItem.Width = this.listView1.Columns[2].Width;
                }

                // Adjust the top to the location of the ListView.
                ClickedItem.Y += this.listView1.Top;
                ClickedItem.X += this.listView1.Left;

                // Assign calculated bounds to the ComboBox.
                this.lvComboBox.Bounds = ClickedItem;

                // Set default text for ComboBox to match the item that is clicked.
                this.lvComboBox.Text = lvItem.SubItems[2].Text;

                // Display the ComboBox, and make sure that it is on top with focus.
                this.lvComboBox.Visible = true;
                this.lvComboBox.BringToFront();
                this.lvComboBox.Focus();
            }

        }

        void cbListViewCombo_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
        {
            // user presses ESC.
            switch (e.KeyChar)
            {
                case (char)(int)System.Windows.Forms.Keys.Escape:
                    {
                        // Reset the original text value, and hide the ComboBox.
                        this.lvComboBox.Text = lvItem.SubItems[2].Text;
                        this.lvComboBox.Visible = false;
                        break;
                    }

                case (char)(int)System.Windows.Forms.Keys.Enter:
                    {
                        this.lvComboBox.Visible = false;
                        break;
                    }
            }

        }

        void listView1_SizeChanged(object sender, System.EventArgs e)
        {
            adjustColumnsSize();
        }

        void cbListViewCombo_Leave(object sender, System.EventArgs e)
        {
            lvItem.SubItems[2].Text = this.lvComboBox.Text;
            this.lvComboBox.Visible = false;
        }

        void cbListViewCombo_SelectedValueChanged(object sender, System.EventArgs e)
        {
            lvItem.SubItems[2].Text = this.lvComboBox.Text;
            this.lvComboBox.Visible = false;
        }
    }
}
