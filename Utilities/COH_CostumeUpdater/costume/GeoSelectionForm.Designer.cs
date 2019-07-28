using System.Drawing;
namespace COH_CostumeUpdater.costume
{
    partial class GeoSelectionForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.geoSelectionPanel = new System.Windows.Forms.Panel();
            this.lvComboBox = new System.Windows.Forms.ComboBox();
            this.listView1 = new COH_CostumeUpdater.costume.ListViewComboBox();
            this.filePathColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.fileTypeColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.geoColumnHeader = new System.Windows.Forms.ColumnHeader();
            this.okBtn = new System.Windows.Forms.Button();
            this.panel2 = new System.Windows.Forms.Panel();
            this.geoSelectionPanel.SuspendLayout();
            this.panel2.SuspendLayout();
            this.SuspendLayout();
            // 
            // geoSelectionPanel
            // 
            this.geoSelectionPanel.Controls.Add(this.lvComboBox);
            this.geoSelectionPanel.Controls.Add(this.listView1);
            this.geoSelectionPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            this.geoSelectionPanel.Location = new System.Drawing.Point(0, 0);
            this.geoSelectionPanel.Name = "geoSelectionPanel";
            this.geoSelectionPanel.Size = new System.Drawing.Size(1033, 475);
            this.geoSelectionPanel.TabIndex = 0;
            // 
            // lvComboBox
            // 
            this.lvComboBox.FormattingEnabled = true;
            this.lvComboBox.Location = new System.Drawing.Point(0, 0);
            this.lvComboBox.Name = "lvComboBox";
            this.lvComboBox.Size = new System.Drawing.Size(121, 21);
            this.lvComboBox.TabIndex = 0;
            this.lvComboBox.TabStop = false;
            this.lvComboBox.Visible = false;
            this.lvComboBox.Leave += new System.EventHandler(this.cbListViewCombo_Leave);
            this.lvComboBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.cbListViewCombo_KeyPress);
            this.lvComboBox.SelectedValueChanged += new System.EventHandler(this.cbListViewCombo_SelectedValueChanged);
            // 
            // listView1
            // 
            this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.filePathColumnHeader,
            this.fileTypeColumnHeader,
            this.geoColumnHeader});
            this.listView1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listView1.FullRowSelect = true;
            this.listView1.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listView1.Location = new System.Drawing.Point(0, 0);
            this.listView1.MultiSelect = false;
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(1033, 475);
            this.listView1.TabIndex = 1;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            this.listView1.SizeChanged += new System.EventHandler(this.listView1_SizeChanged);
            this.listView1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.listView1_MouseUp);
            // 
            // filePathColumnHeader
            // 
            this.filePathColumnHeader.Text = "File Path";
            this.filePathColumnHeader.Width = 424;
            // 
            // fileTypeColumnHeader
            // 
            this.fileTypeColumnHeader.Text = "Character Type";
            this.fileTypeColumnHeader.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.fileTypeColumnHeader.Width = 92;
            // 
            // geoColumnHeader
            // 
            this.geoColumnHeader.Text = "Geo Selection";
            this.geoColumnHeader.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.geoColumnHeader.Width = 175;
            // 
            // okBtn
            // 
            this.okBtn.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.okBtn.Location = new System.Drawing.Point(479, 3);
            this.okBtn.Name = "okBtn";
            this.okBtn.Size = new System.Drawing.Size(75, 23);
            this.okBtn.TabIndex = 1;
            this.okBtn.Text = "OK";
            this.okBtn.UseVisualStyleBackColor = true;
            this.okBtn.Click += new System.EventHandler(this.okBtn_Click);
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.okBtn);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel2.Location = new System.Drawing.Point(0, 475);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(1033, 30);
            this.panel2.TabIndex = 2;
            // 
            // GeoSelectionForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1033, 505);
            this.Controls.Add(this.geoSelectionPanel);
            this.Controls.Add(this.panel2);
            this.Name = "GeoSelectionForm";
            this.Text = "GeoSelectionForm";
            this.geoSelectionPanel.ResumeLayout(false);
            this.panel2.ResumeLayout(false);
            this.ResumeLayout(false);

        }


        #endregion
        private bool okClicked;
        private System.Windows.Forms.Panel geoSelectionPanel;
        private ListViewComboBox listView1;
        private System.Windows.Forms.ListViewItem lvItem;
        private System.Windows.Forms.ColumnHeader filePathColumnHeader;
        private System.Windows.Forms.ColumnHeader fileTypeColumnHeader;
        private System.Windows.Forms.ColumnHeader geoColumnHeader;
        private System.Windows.Forms.ListViewItem listviewitem;
        private System.Windows.Forms.Button okBtn;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.ComboBox lvComboBox;
    }
}