namespace COH_CostumeUpdater.fxEditor
{
    partial class EventSound
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
            this.soundFileBrowser_btn = new System.Windows.Forms.Button();
            this.sound_TxBx = new System.Windows.Forms.TextBox();
            this.radius_Label = new System.Windows.Forms.Label();
            this.volume_label = new System.Windows.Forms.Label();
            this.ramp_label = new System.Windows.Forms.Label();
            this.radius_num = new System.Windows.Forms.NumericUpDown();
            this.ramp_num = new System.Windows.Forms.NumericUpDown();
            this.volume_num = new System.Windows.Forms.NumericUpDown();
            this.addSound_btn = new System.Windows.Forms.Button();
            this.enable_checkBox = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.radius_num)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.ramp_num)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.volume_num)).BeginInit();
            this.SuspendLayout();
            // 
            // soundFileBrowser_btn
            // 
            this.soundFileBrowser_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.soundFileBrowser_btn.Location = new System.Drawing.Point(249, 3);
            this.soundFileBrowser_btn.Name = "soundFileBrowser_btn";
            this.soundFileBrowser_btn.Size = new System.Drawing.Size(40, 23);
            this.soundFileBrowser_btn.TabIndex = 1;
            this.soundFileBrowser_btn.Text = "...";
            this.soundFileBrowser_btn.UseVisualStyleBackColor = true;
            this.soundFileBrowser_btn.Click += new System.EventHandler(this.soundFileBrowser_btn_Click);
            // 
            // sound_TxBx
            // 
            this.sound_TxBx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.sound_TxBx.Location = new System.Drawing.Point(58, 5);
            this.sound_TxBx.Name = "sound_TxBx";
            this.sound_TxBx.Size = new System.Drawing.Size(190, 20);
            this.sound_TxBx.TabIndex = 2;
            this.sound_TxBx.Validated += new System.EventHandler(this.sound_ValueChanged);
            // 
            // radius_Label
            // 
            this.radius_Label.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.radius_Label.AutoSize = true;
            this.radius_Label.Location = new System.Drawing.Point(293, 8);
            this.radius_Label.Name = "radius_Label";
            this.radius_Label.Size = new System.Drawing.Size(43, 13);
            this.radius_Label.TabIndex = 3;
            this.radius_Label.Text = "Radius:";
            this.radius_Label.MouseClick += new System.Windows.Forms.MouseEventHandler(this.EventSound_MouseClick);
            // 
            // volume_label
            // 
            this.volume_label.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.volume_label.AutoSize = true;
            this.volume_label.Location = new System.Drawing.Point(495, 8);
            this.volume_label.Name = "volume_label";
            this.volume_label.Size = new System.Drawing.Size(45, 13);
            this.volume_label.TabIndex = 3;
            this.volume_label.Text = "Volume:";
            this.volume_label.MouseClick += new System.Windows.Forms.MouseEventHandler(this.EventSound_MouseClick);
            // 
            // ramp_label
            // 
            this.ramp_label.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.ramp_label.AutoSize = true;
            this.ramp_label.Location = new System.Drawing.Point(397, 8);
            this.ramp_label.Name = "ramp_label";
            this.ramp_label.Size = new System.Drawing.Size(38, 13);
            this.ramp_label.TabIndex = 3;
            this.ramp_label.Text = "Ramp:";
            this.ramp_label.MouseClick += new System.Windows.Forms.MouseEventHandler(this.EventSound_MouseClick);
            // 
            // radius_num
            // 
            this.radius_num.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.radius_num.Location = new System.Drawing.Point(332, 6);
            this.radius_num.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.radius_num.Name = "radius_num";
            this.radius_num.Size = new System.Drawing.Size(60, 20);
            this.radius_num.TabIndex = 2;
            this.radius_num.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.radius_num.ValueChanged += new System.EventHandler(this.sound_ValueChanged);
            // 
            // ramp_num
            // 
            this.ramp_num.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.ramp_num.Location = new System.Drawing.Point(432, 6);
            this.ramp_num.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.ramp_num.Name = "ramp_num";
            this.ramp_num.Size = new System.Drawing.Size(60, 20);
            this.ramp_num.TabIndex = 3;
            this.ramp_num.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.ramp_num.ValueChanged += new System.EventHandler(this.sound_ValueChanged);
            // 
            // volume_num
            // 
            this.volume_num.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.volume_num.DecimalPlaces = 4;
            this.volume_num.Increment = new decimal(new int[] {
            1,
            0,
            0,
            131072});
            this.volume_num.Location = new System.Drawing.Point(536, 6);
            this.volume_num.Maximum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.volume_num.Name = "volume_num";
            this.volume_num.Size = new System.Drawing.Size(60, 20);
            this.volume_num.TabIndex = 4;
            this.volume_num.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.volume_num.ValueChanged += new System.EventHandler(this.sound_ValueChanged);
            // 
            // addSound_btn
            // 
            this.addSound_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.addSound_btn.Image = global::COH_CostumeUpdater.Properties.Resources.Add1;
            this.addSound_btn.Location = new System.Drawing.Point(600, 3);
            this.addSound_btn.Name = "addSound_btn";
            this.addSound_btn.Size = new System.Drawing.Size(23, 23);
            this.addSound_btn.TabIndex = 5;
            this.addSound_btn.UseVisualStyleBackColor = true;
            // 
            // enable_checkBox
            // 
            this.enable_checkBox.AutoSize = true;
            this.enable_checkBox.Location = new System.Drawing.Point(2, 6);
            this.enable_checkBox.Name = "enable_checkBox";
            this.enable_checkBox.Size = new System.Drawing.Size(60, 17);
            this.enable_checkBox.TabIndex = 6;
            this.enable_checkBox.Text = "Sound:";
            this.enable_checkBox.UseVisualStyleBackColor = true;
            this.enable_checkBox.CheckedChanged += new System.EventHandler(this.enable_checkBox_CheckedChanged);
            this.enable_checkBox.MouseClick += new System.Windows.Forms.MouseEventHandler(EventSound_MouseClick);
            // 
            // EventSound
            // 
            this.ClientSize = new System.Drawing.Size(626, 26);
            this.Controls.Add(this.sound_TxBx);
            this.Controls.Add(this.enable_checkBox);
            this.Controls.Add(this.volume_num);
            this.Controls.Add(this.ramp_num);
            this.Controls.Add(this.radius_num);
            this.Controls.Add(this.ramp_label);
            this.Controls.Add(this.volume_label);
            this.Controls.Add(this.radius_Label);
            this.Controls.Add(this.soundFileBrowser_btn);
            this.Controls.Add(this.addSound_btn);
            this.Name = "EventSound";
            this.Text = "EventSound";
            ((System.ComponentModel.ISupportInitialize)(this.radius_num)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.ramp_num)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.volume_num)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }


        #endregion

        private System.Windows.Forms.OpenFileDialog openFileD;
        private System.Windows.Forms.Button soundFileBrowser_btn;
        public System.Windows.Forms.TextBox sound_TxBx;
        private System.Windows.Forms.Label radius_Label;
        private System.Windows.Forms.Label volume_label;
        private System.Windows.Forms.Label ramp_label;
        private System.Windows.Forms.NumericUpDown radius_num;
        private System.Windows.Forms.NumericUpDown ramp_num;
        private System.Windows.Forms.NumericUpDown volume_num;
        public System.Windows.Forms.Button addSound_btn;
        private System.Windows.Forms.CheckBox enable_checkBox;
    }
}