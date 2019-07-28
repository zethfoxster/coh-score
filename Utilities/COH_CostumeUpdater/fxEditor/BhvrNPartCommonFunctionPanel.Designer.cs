namespace COH_CostumeUpdater.fxEditor
{
    partial class BhvrNPartCommonFunctionPanel
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
            this.components = new System.ComponentModel.Container();
            this.comboBxs_panel = new System.Windows.Forms.Panel();
            this.eventsComboBx = new System.Windows.Forms.ComboBox();
            this.bhvr_label = new System.Windows.Forms.Label();
            this.eventsBhvrFilesMComboBx = new COH_CostumeUpdater.common.MComboBox();
            this.Bhvr4Event_label = new System.Windows.Forms.Label();
            this.fileNP4Btns_panel = new System.Windows.Forms.Panel();
            this.floatWin_btn = new System.Windows.Forms.Button();
            this.p4_label = new System.Windows.Forms.Label();
            this.saveAs_btn = new System.Windows.Forms.Button();
            this.p4CheckIn_btn = new System.Windows.Forms.Button();
            this.new_btn = new System.Windows.Forms.Button();
            this.p4CheckOut_btn = new System.Windows.Forms.Button();
            this.load_btn = new System.Windows.Forms.Button();
            this.save_btn = new System.Windows.Forms.Button();
            this.p4Add_btn = new System.Windows.Forms.Button();
            this.comboBxs_panel.SuspendLayout();
            this.fileNP4Btns_panel.SuspendLayout();
            this.SuspendLayout();
            // 
            // comboBxs_panel
            // 
            this.comboBxs_panel.BackColor = System.Drawing.SystemColors.Control;
            this.comboBxs_panel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.comboBxs_panel.Controls.Add(this.eventsComboBx);
            this.comboBxs_panel.Controls.Add(this.bhvr_label);
            this.comboBxs_panel.Controls.Add(this.eventsBhvrFilesMComboBx);
            this.comboBxs_panel.Controls.Add(this.Bhvr4Event_label);
            this.comboBxs_panel.Controls.Add(this.fileNP4Btns_panel);
            this.comboBxs_panel.Dock = System.Windows.Forms.DockStyle.Top;
            this.comboBxs_panel.Location = new System.Drawing.Point(0, 0);
            this.comboBxs_panel.Name = "comboBxs_panel";
            this.comboBxs_panel.Size = new System.Drawing.Size(564, 48);
            this.comboBxs_panel.TabIndex = 2;
            // 
            // eventsComboBx
            // 
            this.eventsComboBx.FormattingEnabled = true;
            this.eventsComboBx.Location = new System.Drawing.Point(111, 23);
            this.eventsComboBx.Name = "eventsComboBx";
            this.eventsComboBx.Size = new System.Drawing.Size(183, 21);
            this.eventsComboBx.TabIndex = 5;
            this.eventsComboBx.SelectedIndexChanged += new System.EventHandler(this.eventsComboBx_SelectedIndexChanged);
            // 
            // bhvr_label
            // 
            this.bhvr_label.AutoSize = true;
            this.bhvr_label.Location = new System.Drawing.Point(306, 26);
            this.bhvr_label.Name = "bhvr_label";
            this.bhvr_label.Size = new System.Drawing.Size(51, 13);
            this.bhvr_label.TabIndex = 4;
            this.bhvr_label.Text = "Bhvr File:";
            // 
            // eventsBhvrFilesMComboBx
            // 
            this.eventsBhvrFilesMComboBx.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.eventsBhvrFilesMComboBx.DrawMode = System.Windows.Forms.DrawMode.OwnerDrawFixed;
            this.eventsBhvrFilesMComboBx.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.eventsBhvrFilesMComboBx.Location = new System.Drawing.Point(359, 23);
            this.eventsBhvrFilesMComboBx.Name = "eventsBhvrFilesMComboBx";
            this.eventsBhvrFilesMComboBx.Size = new System.Drawing.Size(200, 21);
            this.eventsBhvrFilesMComboBx.TabIndex = 3;            
            this.eventsBhvrFilesMComboBx.SelectedIndexChanged += new System.EventHandler(this.eventsBhvrFilesMComboBx_SelectedIndexChanged);
            // 
            // Bhvr4Event_label
            // 
            this.Bhvr4Event_label.AutoSize = true;
            this.Bhvr4Event_label.Location = new System.Drawing.Point(3, 26);
            this.Bhvr4Event_label.Name = "Bhvr4Event_label";
            this.Bhvr4Event_label.Size = new System.Drawing.Size(106, 13);
            this.Bhvr4Event_label.TabIndex = 1;
            this.Bhvr4Event_label.Text = "Behaviors For Event:";
            // 
            // fileNP4Btns_panel
            // 
            this.fileNP4Btns_panel.BackColor = System.Drawing.SystemColors.GradientActiveCaption;
            this.fileNP4Btns_panel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.fileNP4Btns_panel.Controls.Add(this.floatWin_btn);
            this.fileNP4Btns_panel.Controls.Add(this.p4_label);
            this.fileNP4Btns_panel.Controls.Add(this.saveAs_btn);
            this.fileNP4Btns_panel.Controls.Add(this.p4CheckIn_btn);
            this.fileNP4Btns_panel.Controls.Add(this.new_btn);
            this.fileNP4Btns_panel.Controls.Add(this.p4CheckOut_btn);
            this.fileNP4Btns_panel.Controls.Add(this.load_btn);
            this.fileNP4Btns_panel.Controls.Add(this.save_btn);
            this.fileNP4Btns_panel.Controls.Add(this.p4Add_btn);
            this.fileNP4Btns_panel.Dock = System.Windows.Forms.DockStyle.Top;
            this.fileNP4Btns_panel.Location = new System.Drawing.Point(0, 0);
            this.fileNP4Btns_panel.Name = "fileNP4Btns_panel";
            this.fileNP4Btns_panel.Size = new System.Drawing.Size(562, 22);
            this.fileNP4Btns_panel.TabIndex = 2;
            // 
            // floatWin_btn
            // 
            this.floatWin_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.floatWin_btn.AutoEllipsis = true;
            this.floatWin_btn.BackColor = System.Drawing.SystemColors.ButtonFace;
            this.floatWin_btn.FlatAppearance.BorderColor = System.Drawing.Color.White;
            this.floatWin_btn.Image = global::COH_CostumeUpdater.Properties.Resources.EXE;
            this.floatWin_btn.Location = new System.Drawing.Point(527, 0);
            this.floatWin_btn.Name = "floatWin_btn";
            this.floatWin_btn.Size = new System.Drawing.Size(32, 20);
            this.floatWin_btn.TabIndex = 2;
            this.floatWin_btn.TabStop = false;
            this.floatWin_btn.Text = "Float";
            this.floatWin_btn.UseVisualStyleBackColor = false;
            this.floatWin_btn.Click += new System.EventHandler(this.floatWin_btn_Click);
            // 
            // p4_label
            // 
            this.p4_label.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4_label.AutoSize = true;
            this.p4_label.BackColor = System.Drawing.SystemColors.GradientActiveCaption;
            this.p4_label.Location = new System.Drawing.Point(302, 3);
            this.p4_label.Name = "p4_label";
            this.p4_label.Size = new System.Drawing.Size(23, 13);
            this.p4_label.TabIndex = 1;
            this.p4_label.Text = "P4:";
            // 
            // saveAs_btn
            // 
            this.saveAs_btn.Location = new System.Drawing.Point(188, 0);
            this.saveAs_btn.Name = "saveAs_btn";
            this.saveAs_btn.Size = new System.Drawing.Size(60, 20);
            this.saveAs_btn.TabIndex = 0;
            this.saveAs_btn.Text = "Save As";
            this.saveAs_btn.UseVisualStyleBackColor = true;
            this.saveAs_btn.Click += new System.EventHandler(this.saveAs_btn_Click);
            // 
            // p4CheckIn_btn
            // 
            this.p4CheckIn_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4CheckIn_btn.Location = new System.Drawing.Point(461, 0);
            this.p4CheckIn_btn.Name = "p4CheckIn_btn";
            this.p4CheckIn_btn.Size = new System.Drawing.Size(65, 20);
            this.p4CheckIn_btn.TabIndex = 0;
            this.p4CheckIn_btn.Text = "CheckIn";
            this.p4CheckIn_btn.UseVisualStyleBackColor = true;
            this.p4CheckIn_btn.Click += new System.EventHandler(this.p4CheckIn_btn_Click);
            // 
            // new_btn
            // 
            this.new_btn.Location = new System.Drawing.Point(2, 0);
            this.new_btn.Name = "new_btn";
            this.new_btn.Size = new System.Drawing.Size(60, 20);
            this.new_btn.TabIndex = 0;
            this.new_btn.Text = "New";
            this.new_btn.UseVisualStyleBackColor = true;
            this.new_btn.Click += new System.EventHandler(this.new_btn_Click);
            // 
            // p4CheckOut_btn
            // 
            this.p4CheckOut_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4CheckOut_btn.Location = new System.Drawing.Point(394, 0);
            this.p4CheckOut_btn.Name = "p4CheckOut_btn";
            this.p4CheckOut_btn.Size = new System.Drawing.Size(65, 20);
            this.p4CheckOut_btn.TabIndex = 0;
            this.p4CheckOut_btn.Text = "CheckOut";
            this.p4CheckOut_btn.UseVisualStyleBackColor = true;
            this.p4CheckOut_btn.Click += new System.EventHandler(this.p4CheckOut_btn_Click);
            // 
            // load_btn
            // 
            this.load_btn.Location = new System.Drawing.Point(64, 0);
            this.load_btn.Name = "load_btn";
            this.load_btn.Size = new System.Drawing.Size(60, 20);
            this.load_btn.TabIndex = 0;
            this.load_btn.Text = "Load";
            this.load_btn.UseVisualStyleBackColor = true;
            this.load_btn.Click += new System.EventHandler(this.load_btn_Click);
            // 
            // save_btn
            // 
            this.save_btn.Location = new System.Drawing.Point(126, 0);
            this.save_btn.Name = "save_btn";
            this.save_btn.Size = new System.Drawing.Size(60, 20);
            this.save_btn.TabIndex = 0;
            this.save_btn.Text = "Save";
            this.save_btn.UseVisualStyleBackColor = true;
            this.save_btn.Click += new System.EventHandler(this.save_btn_Click);
            // 
            // p4Add_btn
            // 
            this.p4Add_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.p4Add_btn.Location = new System.Drawing.Point(327, 0);
            this.p4Add_btn.Name = "p4Add_btn";
            this.p4Add_btn.Size = new System.Drawing.Size(65, 20);
            this.p4Add_btn.TabIndex = 0;
            this.p4Add_btn.Text = "Add";
            this.p4Add_btn.UseVisualStyleBackColor = true;
            this.p4Add_btn.Click += new System.EventHandler(this.p4Add_btn_Click);
            // 
            // BhvrNPartCommonFunctionPanel
            // 
            this.ClientSize = new System.Drawing.Size(564, 48);
            this.Controls.Add(this.comboBxs_panel);
            this.Name = "BhvrNPartCommonFunctionPanel";
            this.Text = "BhvrNPartCommonFunctionPanel";
            this.comboBxs_panel.ResumeLayout(false);
            this.comboBxs_panel.PerformLayout();
            this.fileNP4Btns_panel.ResumeLayout(false);
            this.fileNP4Btns_panel.PerformLayout();
            this.ResumeLayout(false);

        }



        #endregion


        public System.Windows.Forms.ComboBox eventsComboBx;
        public COH_CostumeUpdater.common.MComboBox eventsBhvrFilesMComboBx;
        private System.Windows.Forms.Panel comboBxs_panel;
        private System.Windows.Forms.Label bhvr_label;
        private System.Windows.Forms.Label Bhvr4Event_label;
        private System.Windows.Forms.Panel fileNP4Btns_panel;
        private System.Windows.Forms.Button floatWin_btn;
        private System.Windows.Forms.Label p4_label;
        private System.Windows.Forms.Button saveAs_btn;
        private System.Windows.Forms.Button p4CheckIn_btn;
        private System.Windows.Forms.Button new_btn;
        private System.Windows.Forms.Button p4CheckOut_btn;
        private System.Windows.Forms.Button load_btn;
        private System.Windows.Forms.Button save_btn;
        private System.Windows.Forms.Button p4Add_btn;
    }
}