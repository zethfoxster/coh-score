namespace COH_CostumeUpdater.assetEditor.aeCommon
{
    partial class COH_ColorPicker
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


        private void initMsliders()
        {
            this.mSliderR = new COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider("R:");
            this.mSliderG = new COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider("G:");
            this.mSliderB = new COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider("B:");
            this.mSliderV = new COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider("V:");
        }
        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.panel1 = new System.Windows.Forms.Panel();
            this.panel3 = new System.Windows.Forms.Panel();
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.ColorSelection = new System.Windows.Forms.GroupBox();
            this.cStore20 = new System.Windows.Forms.Button();
            this.cStore48 = new System.Windows.Forms.Button();
            this.cStore15 = new System.Windows.Forms.Button();
            this.cStore42 = new System.Windows.Forms.Button();
            this.cStore25 = new System.Windows.Forms.Button();
            this.cStore10 = new System.Windows.Forms.Button();
            this.cStore36 = new System.Windows.Forms.Button();
            this.cStore5 = new System.Windows.Forms.Button();
            this.cStore30 = new System.Windows.Forms.Button();
            this.cStore47 = new System.Windows.Forms.Button();
            this.cStore29 = new System.Windows.Forms.Button();
            this.cStore19 = new System.Windows.Forms.Button();
            this.cStore41 = new System.Windows.Forms.Button();
            this.cStore46 = new System.Windows.Forms.Button();
            this.cStore28 = new System.Windows.Forms.Button();
            this.cStore14 = new System.Windows.Forms.Button();
            this.cStore27 = new System.Windows.Forms.Button();
            this.cStore40 = new System.Windows.Forms.Button();
            this.cStore24 = new System.Windows.Forms.Button();
            this.cStore35 = new System.Windows.Forms.Button();
            this.cStore26 = new System.Windows.Forms.Button();
            this.cStore9 = new System.Windows.Forms.Button();
            this.cStore34 = new System.Windows.Forms.Button();
            this.cStore4 = new System.Windows.Forms.Button();
            this.cStore18 = new System.Windows.Forms.Button();
            this.cStore45 = new System.Windows.Forms.Button();
            this.cStore17 = new System.Windows.Forms.Button();
            this.cStore44 = new System.Windows.Forms.Button();
            this.cStore13 = new System.Windows.Forms.Button();
            this.cStore12 = new System.Windows.Forms.Button();
            this.cStore39 = new System.Windows.Forms.Button();
            this.cStore23 = new System.Windows.Forms.Button();
            this.cStore38 = new System.Windows.Forms.Button();
            this.cStore8 = new System.Windows.Forms.Button();
            this.cStore22 = new System.Windows.Forms.Button();
            this.cStore16 = new System.Windows.Forms.Button();
            this.cStore43 = new System.Windows.Forms.Button();
            this.cStore7 = new System.Windows.Forms.Button();
            this.cStore11 = new System.Windows.Forms.Button();
            this.cStore37 = new System.Windows.Forms.Button();
            this.cStore33 = new System.Windows.Forms.Button();
            this.cStore21 = new System.Windows.Forms.Button();
            this.cStore3 = new System.Windows.Forms.Button();
            this.cStore32 = new System.Windows.Forms.Button();
            this.cStore6 = new System.Windows.Forms.Button();
            this.cStore31 = new System.Windows.Forms.Button();
            this.cStore2 = new System.Windows.Forms.Button();
            this.cStore1 = new System.Windows.Forms.Button();
            this.panel2 = new System.Windows.Forms.Panel();
            this.cwMarker = new System.Windows.Forms.Panel();
            this.colorWheelPicker = new System.Windows.Forms.PictureBox();
            this.lastColorBtn = new System.Windows.Forms.Button();
            this.panel1.SuspendLayout();
            this.panel3.SuspendLayout();
            this.ColorSelection.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.colorWheelPicker)).BeginInit();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.AutoScroll = true;
            this.panel1.AutoScrollMargin = new System.Drawing.Size(10, 0);
            this.panel1.AutoScrollMinSize = new System.Drawing.Size(10, 10);
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.panel3);
            this.panel1.Controls.Add(this.ColorSelection);
            this.panel1.Controls.Add(this.cwMarker);
            this.panel1.Controls.Add(this.colorWheelPicker);
            this.panel1.Controls.Add(this.mSliderV);
            this.panel1.Controls.Add(this.mSliderB);
            this.panel1.Controls.Add(this.mSliderG);
            this.panel1.Controls.Add(this.mSliderR);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(616, 302);
            this.panel1.TabIndex = 0;
            // 
            // panel3
            // 
            this.panel3.BackColor = System.Drawing.SystemColors.ControlLight;
            this.panel3.Controls.Add(this.lastColorBtn);
            this.panel3.Controls.Add(this.okButton);
            this.panel3.Controls.Add(this.cancelButton);
            this.panel3.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel3.Location = new System.Drawing.Point(0, 272);
            this.panel3.Name = "panel3";
            this.panel3.Size = new System.Drawing.Size(614, 28);
            this.panel3.TabIndex = 10;
            // 
            // okButton
            // 
            this.okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.okButton.Location = new System.Drawing.Point(444, 2);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 23);
            this.okButton.TabIndex = 9;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(525, 2);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 9;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            // 
            // ColorSelection
            // 
            this.ColorSelection.Controls.Add(this.cStore20);
            this.ColorSelection.Controls.Add(this.cStore48);
            this.ColorSelection.Controls.Add(this.cStore15);
            this.ColorSelection.Controls.Add(this.cStore42);
            this.ColorSelection.Controls.Add(this.cStore25);
            this.ColorSelection.Controls.Add(this.cStore10);
            this.ColorSelection.Controls.Add(this.cStore36);
            this.ColorSelection.Controls.Add(this.cStore5);
            this.ColorSelection.Controls.Add(this.cStore30);
            this.ColorSelection.Controls.Add(this.cStore47);
            this.ColorSelection.Controls.Add(this.cStore29);
            this.ColorSelection.Controls.Add(this.cStore19);
            this.ColorSelection.Controls.Add(this.cStore41);
            this.ColorSelection.Controls.Add(this.cStore46);
            this.ColorSelection.Controls.Add(this.cStore28);
            this.ColorSelection.Controls.Add(this.cStore14);
            this.ColorSelection.Controls.Add(this.cStore27);
            this.ColorSelection.Controls.Add(this.cStore40);
            this.ColorSelection.Controls.Add(this.cStore24);
            this.ColorSelection.Controls.Add(this.cStore35);
            this.ColorSelection.Controls.Add(this.cStore26);
            this.ColorSelection.Controls.Add(this.cStore9);
            this.ColorSelection.Controls.Add(this.cStore34);
            this.ColorSelection.Controls.Add(this.cStore4);
            this.ColorSelection.Controls.Add(this.cStore18);
            this.ColorSelection.Controls.Add(this.cStore45);
            this.ColorSelection.Controls.Add(this.cStore17);
            this.ColorSelection.Controls.Add(this.cStore44);
            this.ColorSelection.Controls.Add(this.cStore13);
            this.ColorSelection.Controls.Add(this.cStore12);
            this.ColorSelection.Controls.Add(this.cStore39);
            this.ColorSelection.Controls.Add(this.cStore23);
            this.ColorSelection.Controls.Add(this.cStore38);
            this.ColorSelection.Controls.Add(this.cStore8);
            this.ColorSelection.Controls.Add(this.cStore22);
            this.ColorSelection.Controls.Add(this.cStore16);
            this.ColorSelection.Controls.Add(this.cStore43);
            this.ColorSelection.Controls.Add(this.cStore7);
            this.ColorSelection.Controls.Add(this.cStore11);
            this.ColorSelection.Controls.Add(this.cStore37);
            this.ColorSelection.Controls.Add(this.cStore33);
            this.ColorSelection.Controls.Add(this.cStore21);
            this.ColorSelection.Controls.Add(this.cStore3);
            this.ColorSelection.Controls.Add(this.cStore32);
            this.ColorSelection.Controls.Add(this.cStore6);
            this.ColorSelection.Controls.Add(this.cStore31);
            this.ColorSelection.Controls.Add(this.cStore2);
            this.ColorSelection.Controls.Add(this.cStore1);
            this.ColorSelection.Controls.Add(this.panel2);
            this.ColorSelection.Location = new System.Drawing.Point(3, 3);
            this.ColorSelection.Name = "ColorSelection";
            this.ColorSelection.Size = new System.Drawing.Size(338, 168);
            this.ColorSelection.TabIndex = 8;
            this.ColorSelection.TabStop = false;
            this.ColorSelection.Text = "Color Selection";
            // 
            // cStore20
            // 
            this.cStore20.BackColor = System.Drawing.Color.Black;
            this.cStore20.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore20.ForeColor = System.Drawing.Color.Black;
            this.cStore20.Location = new System.Drawing.Point(304, 19);
            this.cStore20.Name = "cStore20";
            this.cStore20.Size = new System.Drawing.Size(22, 14);
            this.cStore20.TabIndex = 6;
            this.cStore20.UseVisualStyleBackColor = false;
            this.cStore20.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore48
            // 
            this.cStore48.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(128)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.cStore48.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore48.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(128)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.cStore48.Location = new System.Drawing.Point(304, 55);
            this.cStore48.Name = "cStore48";
            this.cStore48.Size = new System.Drawing.Size(22, 14);
            this.cStore48.TabIndex = 6;
            this.cStore48.UseVisualStyleBackColor = false;
            this.cStore48.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore15
            // 
            this.cStore15.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.cStore15.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore15.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.cStore15.Location = new System.Drawing.Point(304, 37);
            this.cStore15.Name = "cStore15";
            this.cStore15.Size = new System.Drawing.Size(22, 14);
            this.cStore15.TabIndex = 6;
            this.cStore15.UseVisualStyleBackColor = false;
            this.cStore15.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore42
            // 
            this.cStore42.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.cStore42.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore42.Location = new System.Drawing.Point(304, 109);
            this.cStore42.Name = "cStore42";
            this.cStore42.Size = new System.Drawing.Size(22, 14);
            this.cStore42.TabIndex = 6;
            this.cStore42.UseVisualStyleBackColor = false;
            this.cStore42.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore25
            // 
            this.cStore25.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(64)))), ((int)(((byte)(0)))));
            this.cStore25.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore25.Location = new System.Drawing.Point(304, 91);
            this.cStore25.Name = "cStore25";
            this.cStore25.Size = new System.Drawing.Size(22, 14);
            this.cStore25.TabIndex = 6;
            this.cStore25.UseVisualStyleBackColor = false;
            this.cStore25.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore10
            // 
            this.cStore10.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(0)))));
            this.cStore10.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore10.Location = new System.Drawing.Point(304, 73);
            this.cStore10.Name = "cStore10";
            this.cStore10.Size = new System.Drawing.Size(22, 14);
            this.cStore10.TabIndex = 6;
            this.cStore10.UseVisualStyleBackColor = false;
            this.cStore10.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore36
            // 
            this.cStore36.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(0)))), ((int)(((byte)(64)))));
            this.cStore36.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore36.Location = new System.Drawing.Point(303, 145);
            this.cStore36.Name = "cStore36";
            this.cStore36.Size = new System.Drawing.Size(22, 14);
            this.cStore36.TabIndex = 6;
            this.cStore36.UseVisualStyleBackColor = false;
            this.cStore36.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore5
            // 
            this.cStore5.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(64)))));
            this.cStore5.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore5.Location = new System.Drawing.Point(304, 127);
            this.cStore5.Name = "cStore5";
            this.cStore5.Size = new System.Drawing.Size(22, 14);
            this.cStore5.TabIndex = 6;
            this.cStore5.UseVisualStyleBackColor = false;
            this.cStore5.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore30
            // 
            this.cStore30.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.cStore30.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore30.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(64)))), ((int)(((byte)(64)))), ((int)(((byte)(64)))));
            this.cStore30.Location = new System.Drawing.Point(273, 19);
            this.cStore30.Name = "cStore30";
            this.cStore30.Size = new System.Drawing.Size(22, 14);
            this.cStore30.TabIndex = 6;
            this.cStore30.UseVisualStyleBackColor = false;
            this.cStore30.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore47
            // 
            this.cStore47.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(128)))), ((int)(((byte)(64)))), ((int)(((byte)(0)))));
            this.cStore47.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore47.Location = new System.Drawing.Point(273, 55);
            this.cStore47.Name = "cStore47";
            this.cStore47.Size = new System.Drawing.Size(22, 14);
            this.cStore47.TabIndex = 6;
            this.cStore47.UseVisualStyleBackColor = false;
            this.cStore47.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore29
            // 
            this.cStore29.BackColor = System.Drawing.Color.Maroon;
            this.cStore29.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore29.ForeColor = System.Drawing.Color.Maroon;
            this.cStore29.Location = new System.Drawing.Point(273, 37);
            this.cStore29.Name = "cStore29";
            this.cStore29.Size = new System.Drawing.Size(22, 14);
            this.cStore29.TabIndex = 6;
            this.cStore29.UseVisualStyleBackColor = false;
            this.cStore29.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore19
            // 
            this.cStore19.BackColor = System.Drawing.Color.Gray;
            this.cStore19.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore19.ForeColor = System.Drawing.Color.Gray;
            this.cStore19.Location = new System.Drawing.Point(242, 19);
            this.cStore19.Name = "cStore19";
            this.cStore19.Size = new System.Drawing.Size(22, 14);
            this.cStore19.TabIndex = 6;
            this.cStore19.UseVisualStyleBackColor = false;
            this.cStore19.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore41
            // 
            this.cStore41.BackColor = System.Drawing.Color.Teal;
            this.cStore41.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore41.Location = new System.Drawing.Point(273, 109);
            this.cStore41.Name = "cStore41";
            this.cStore41.Size = new System.Drawing.Size(22, 14);
            this.cStore41.TabIndex = 6;
            this.cStore41.UseVisualStyleBackColor = false;
            this.cStore41.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore46
            // 
            this.cStore46.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(64)))), ((int)(((byte)(0)))));
            this.cStore46.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore46.Location = new System.Drawing.Point(242, 55);
            this.cStore46.Name = "cStore46";
            this.cStore46.Size = new System.Drawing.Size(22, 14);
            this.cStore46.TabIndex = 6;
            this.cStore46.UseVisualStyleBackColor = false;
            this.cStore46.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore28
            // 
            this.cStore28.BackColor = System.Drawing.Color.Green;
            this.cStore28.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore28.Location = new System.Drawing.Point(273, 91);
            this.cStore28.Name = "cStore28";
            this.cStore28.Size = new System.Drawing.Size(22, 14);
            this.cStore28.TabIndex = 6;
            this.cStore28.UseVisualStyleBackColor = false;
            this.cStore28.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore14
            // 
            this.cStore14.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.cStore14.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore14.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(0)))), ((int)(((byte)(0)))));
            this.cStore14.Location = new System.Drawing.Point(242, 37);
            this.cStore14.Name = "cStore14";
            this.cStore14.Size = new System.Drawing.Size(22, 14);
            this.cStore14.TabIndex = 6;
            this.cStore14.UseVisualStyleBackColor = false;
            this.cStore14.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore27
            // 
            this.cStore27.BackColor = System.Drawing.Color.Olive;
            this.cStore27.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore27.Location = new System.Drawing.Point(273, 73);
            this.cStore27.Name = "cStore27";
            this.cStore27.Size = new System.Drawing.Size(22, 14);
            this.cStore27.TabIndex = 6;
            this.cStore27.UseVisualStyleBackColor = false;
            this.cStore27.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore40
            // 
            this.cStore40.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.cStore40.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore40.Location = new System.Drawing.Point(242, 109);
            this.cStore40.Name = "cStore40";
            this.cStore40.Size = new System.Drawing.Size(22, 14);
            this.cStore40.TabIndex = 6;
            this.cStore40.UseVisualStyleBackColor = false;
            this.cStore40.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore24
            // 
            this.cStore24.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(0)))));
            this.cStore24.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore24.Location = new System.Drawing.Point(242, 91);
            this.cStore24.Name = "cStore24";
            this.cStore24.Size = new System.Drawing.Size(22, 14);
            this.cStore24.TabIndex = 6;
            this.cStore24.UseVisualStyleBackColor = false;
            this.cStore24.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore35
            // 
            this.cStore35.BackColor = System.Drawing.Color.Purple;
            this.cStore35.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore35.Location = new System.Drawing.Point(272, 145);
            this.cStore35.Name = "cStore35";
            this.cStore35.Size = new System.Drawing.Size(22, 14);
            this.cStore35.TabIndex = 6;
            this.cStore35.UseVisualStyleBackColor = false;
            this.cStore35.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore26
            // 
            this.cStore26.BackColor = System.Drawing.Color.Navy;
            this.cStore26.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore26.Location = new System.Drawing.Point(273, 127);
            this.cStore26.Name = "cStore26";
            this.cStore26.Size = new System.Drawing.Size(22, 14);
            this.cStore26.TabIndex = 6;
            this.cStore26.UseVisualStyleBackColor = false;
            this.cStore26.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore9
            // 
            this.cStore9.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(192)))), ((int)(((byte)(0)))));
            this.cStore9.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore9.Location = new System.Drawing.Point(242, 73);
            this.cStore9.Name = "cStore9";
            this.cStore9.Size = new System.Drawing.Size(22, 14);
            this.cStore9.TabIndex = 6;
            this.cStore9.UseVisualStyleBackColor = false;
            this.cStore9.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore34
            // 
            this.cStore34.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(0)))), ((int)(((byte)(192)))));
            this.cStore34.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore34.Location = new System.Drawing.Point(241, 145);
            this.cStore34.Name = "cStore34";
            this.cStore34.Size = new System.Drawing.Size(22, 14);
            this.cStore34.TabIndex = 6;
            this.cStore34.UseVisualStyleBackColor = false;
            this.cStore34.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore4
            // 
            this.cStore4.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(192)))));
            this.cStore4.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore4.Location = new System.Drawing.Point(242, 127);
            this.cStore4.Name = "cStore4";
            this.cStore4.Size = new System.Drawing.Size(22, 14);
            this.cStore4.TabIndex = 6;
            this.cStore4.UseVisualStyleBackColor = false;
            this.cStore4.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore18
            // 
            this.cStore18.BackColor = System.Drawing.Color.Silver;
            this.cStore18.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore18.ForeColor = System.Drawing.Color.Silver;
            this.cStore18.Location = new System.Drawing.Point(211, 19);
            this.cStore18.Name = "cStore18";
            this.cStore18.Size = new System.Drawing.Size(22, 14);
            this.cStore18.TabIndex = 6;
            this.cStore18.UseVisualStyleBackColor = false;
            this.cStore18.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore45
            // 
            this.cStore45.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(128)))), ((int)(((byte)(0)))));
            this.cStore45.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore45.Location = new System.Drawing.Point(211, 55);
            this.cStore45.Name = "cStore45";
            this.cStore45.Size = new System.Drawing.Size(22, 14);
            this.cStore45.TabIndex = 6;
            this.cStore45.UseVisualStyleBackColor = false;
            this.cStore45.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore17
            // 
            this.cStore17.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.cStore17.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore17.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(224)))), ((int)(((byte)(224)))), ((int)(((byte)(224)))));
            this.cStore17.Location = new System.Drawing.Point(180, 19);
            this.cStore17.Name = "cStore17";
            this.cStore17.Size = new System.Drawing.Size(22, 14);
            this.cStore17.TabIndex = 6;
            this.cStore17.UseVisualStyleBackColor = false;
            this.cStore17.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore44
            // 
            this.cStore44.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(192)))), ((int)(((byte)(128)))));
            this.cStore44.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore44.Location = new System.Drawing.Point(180, 55);
            this.cStore44.Name = "cStore44";
            this.cStore44.Size = new System.Drawing.Size(22, 14);
            this.cStore44.TabIndex = 6;
            this.cStore44.UseVisualStyleBackColor = false;
            this.cStore44.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore13
            // 
            this.cStore13.BackColor = System.Drawing.Color.Red;
            this.cStore13.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore13.ForeColor = System.Drawing.Color.Red;
            this.cStore13.Location = new System.Drawing.Point(211, 37);
            this.cStore13.Name = "cStore13";
            this.cStore13.Size = new System.Drawing.Size(22, 14);
            this.cStore13.TabIndex = 6;
            this.cStore13.UseVisualStyleBackColor = false;
            this.cStore13.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore12
            // 
            this.cStore12.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(128)))), ((int)(((byte)(128)))));
            this.cStore12.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore12.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(128)))), ((int)(((byte)(128)))));
            this.cStore12.Location = new System.Drawing.Point(180, 37);
            this.cStore12.Name = "cStore12";
            this.cStore12.Size = new System.Drawing.Size(22, 14);
            this.cStore12.TabIndex = 6;
            this.cStore12.UseVisualStyleBackColor = false;
            this.cStore12.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore39
            // 
            this.cStore39.BackColor = System.Drawing.Color.Cyan;
            this.cStore39.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore39.Location = new System.Drawing.Point(211, 109);
            this.cStore39.Name = "cStore39";
            this.cStore39.Size = new System.Drawing.Size(22, 14);
            this.cStore39.TabIndex = 6;
            this.cStore39.UseVisualStyleBackColor = false;
            this.cStore39.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore23
            // 
            this.cStore23.BackColor = System.Drawing.Color.Lime;
            this.cStore23.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore23.Location = new System.Drawing.Point(211, 91);
            this.cStore23.Name = "cStore23";
            this.cStore23.Size = new System.Drawing.Size(22, 14);
            this.cStore23.TabIndex = 6;
            this.cStore23.UseVisualStyleBackColor = false;
            this.cStore23.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore38
            // 
            this.cStore38.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(128)))), ((int)(((byte)(255)))), ((int)(((byte)(255)))));
            this.cStore38.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore38.Location = new System.Drawing.Point(180, 109);
            this.cStore38.Name = "cStore38";
            this.cStore38.Size = new System.Drawing.Size(22, 14);
            this.cStore38.TabIndex = 6;
            this.cStore38.UseVisualStyleBackColor = false;
            this.cStore38.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore8
            // 
            this.cStore8.BackColor = System.Drawing.Color.Yellow;
            this.cStore8.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore8.Location = new System.Drawing.Point(211, 73);
            this.cStore8.Name = "cStore8";
            this.cStore8.Size = new System.Drawing.Size(22, 14);
            this.cStore8.TabIndex = 6;
            this.cStore8.UseVisualStyleBackColor = false;
            this.cStore8.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore22
            // 
            this.cStore22.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(128)))), ((int)(((byte)(255)))), ((int)(((byte)(128)))));
            this.cStore22.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore22.Location = new System.Drawing.Point(180, 91);
            this.cStore22.Name = "cStore22";
            this.cStore22.Size = new System.Drawing.Size(22, 14);
            this.cStore22.TabIndex = 6;
            this.cStore22.UseVisualStyleBackColor = false;
            this.cStore22.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore16
            // 
            this.cStore16.BackColor = System.Drawing.Color.White;
            this.cStore16.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore16.ForeColor = System.Drawing.Color.White;
            this.cStore16.Location = new System.Drawing.Point(149, 19);
            this.cStore16.Name = "cStore16";
            this.cStore16.Size = new System.Drawing.Size(22, 14);
            this.cStore16.TabIndex = 6;
            this.cStore16.UseVisualStyleBackColor = false;
            this.cStore16.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore43
            // 
            this.cStore43.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(224)))), ((int)(((byte)(192)))));
            this.cStore43.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore43.Location = new System.Drawing.Point(149, 55);
            this.cStore43.Name = "cStore43";
            this.cStore43.Size = new System.Drawing.Size(22, 14);
            this.cStore43.TabIndex = 6;
            this.cStore43.UseVisualStyleBackColor = false;
            this.cStore43.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore7
            // 
            this.cStore7.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(255)))), ((int)(((byte)(128)))));
            this.cStore7.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore7.Location = new System.Drawing.Point(180, 73);
            this.cStore7.Name = "cStore7";
            this.cStore7.Size = new System.Drawing.Size(22, 14);
            this.cStore7.TabIndex = 6;
            this.cStore7.UseVisualStyleBackColor = false;
            this.cStore7.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore11
            // 
            this.cStore11.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.cStore11.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore11.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.cStore11.Location = new System.Drawing.Point(149, 37);
            this.cStore11.Name = "cStore11";
            this.cStore11.Size = new System.Drawing.Size(22, 14);
            this.cStore11.TabIndex = 6;
            this.cStore11.UseVisualStyleBackColor = false;
            this.cStore11.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore37
            // 
            this.cStore37.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(255)))), ((int)(((byte)(255)))));
            this.cStore37.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore37.Location = new System.Drawing.Point(149, 109);
            this.cStore37.Name = "cStore37";
            this.cStore37.Size = new System.Drawing.Size(22, 14);
            this.cStore37.TabIndex = 6;
            this.cStore37.UseVisualStyleBackColor = false;
            this.cStore37.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore33
            // 
            this.cStore33.BackColor = System.Drawing.Color.Fuchsia;
            this.cStore33.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore33.Location = new System.Drawing.Point(210, 145);
            this.cStore33.Name = "cStore33";
            this.cStore33.Size = new System.Drawing.Size(22, 14);
            this.cStore33.TabIndex = 6;
            this.cStore33.UseVisualStyleBackColor = false;
            this.cStore33.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore21
            // 
            this.cStore21.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
            this.cStore21.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore21.Location = new System.Drawing.Point(149, 91);
            this.cStore21.Name = "cStore21";
            this.cStore21.Size = new System.Drawing.Size(22, 14);
            this.cStore21.TabIndex = 6;
            this.cStore21.UseVisualStyleBackColor = false;
            this.cStore21.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore3
            // 
            this.cStore3.BackColor = System.Drawing.Color.Blue;
            this.cStore3.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore3.Location = new System.Drawing.Point(211, 127);
            this.cStore3.Name = "cStore3";
            this.cStore3.Size = new System.Drawing.Size(22, 14);
            this.cStore3.TabIndex = 6;
            this.cStore3.UseVisualStyleBackColor = false;
            this.cStore3.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore32
            // 
            this.cStore32.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(128)))), ((int)(((byte)(255)))));
            this.cStore32.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore32.Location = new System.Drawing.Point(179, 145);
            this.cStore32.Name = "cStore32";
            this.cStore32.Size = new System.Drawing.Size(22, 14);
            this.cStore32.TabIndex = 6;
            this.cStore32.UseVisualStyleBackColor = false;
            this.cStore32.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore6
            // 
            this.cStore6.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(255)))), ((int)(((byte)(192)))));
            this.cStore6.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore6.Location = new System.Drawing.Point(149, 73);
            this.cStore6.Name = "cStore6";
            this.cStore6.Size = new System.Drawing.Size(22, 14);
            this.cStore6.TabIndex = 6;
            this.cStore6.UseVisualStyleBackColor = false;
            this.cStore6.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore31
            // 
            this.cStore31.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(192)))), ((int)(((byte)(255)))));
            this.cStore31.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore31.Location = new System.Drawing.Point(149, 145);
            this.cStore31.Name = "cStore31";
            this.cStore31.Size = new System.Drawing.Size(22, 14);
            this.cStore31.TabIndex = 6;
            this.cStore31.UseVisualStyleBackColor = false;
            this.cStore31.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore2
            // 
            this.cStore2.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(128)))), ((int)(((byte)(128)))), ((int)(((byte)(255)))));
            this.cStore2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore2.Location = new System.Drawing.Point(180, 127);
            this.cStore2.Name = "cStore2";
            this.cStore2.Size = new System.Drawing.Size(22, 14);
            this.cStore2.TabIndex = 6;
            this.cStore2.UseVisualStyleBackColor = false;
            this.cStore2.Click += new System.EventHandler(this.cStore_Click);
            // 
            // cStore1
            // 
            this.cStore1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(192)))), ((int)(((byte)(192)))), ((int)(((byte)(255)))));
            this.cStore1.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.cStore1.Location = new System.Drawing.Point(149, 127);
            this.cStore1.Name = "cStore1";
            this.cStore1.Size = new System.Drawing.Size(22, 14);
            this.cStore1.TabIndex = 6;
            this.cStore1.UseVisualStyleBackColor = false;
            this.cStore1.Click += new System.EventHandler(this.cStore_Click);
            // 
            // panel2
            // 
            this.panel2.BackColor = System.Drawing.Color.White;
            this.panel2.Location = new System.Drawing.Point(6, 19);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(138, 143);
            this.panel2.TabIndex = 5;
            // 
            // cwMarker
            // 
            this.cwMarker.BackColor = System.Drawing.SystemColors.Control;
            this.cwMarker.Enabled = false;
            this.cwMarker.Location = new System.Drawing.Point(477, 76);
            this.cwMarker.Name = "cwMarker";
            this.cwMarker.Size = new System.Drawing.Size(16, 16);
            this.cwMarker.TabIndex = 7;
            // 
            // colorWheelPicker
            // 
            this.colorWheelPicker.BackColor = System.Drawing.SystemColors.Control;
            this.colorWheelPicker.Location = new System.Drawing.Point(348, 11);
            this.colorWheelPicker.Name = "colorWheelPicker";
            this.colorWheelPicker.Size = new System.Drawing.Size(256, 256);
            this.colorWheelPicker.TabIndex = 1;
            this.colorWheelPicker.TabStop = false;
            this.colorWheelPicker.MouseLeave += new System.EventHandler(this.colorWheelPicker_MouseLeave);
            this.colorWheelPicker.MouseMove += new System.Windows.Forms.MouseEventHandler(this.colorWheelPicker_MouseMove);
            this.colorWheelPicker.Click += new System.EventHandler(this.colorWheelPicker_Click);
            this.colorWheelPicker.MouseEnter += new System.EventHandler(this.colorWheelPicker_MouseEnter);
            // 
            // mSliderV
            // 
            this.mSliderV.Location = new System.Drawing.Point(3, 246);
            this.mSliderV.Name = "mSliderV";
            this.mSliderV.Size = new System.Drawing.Size(338, 22);
            this.mSliderV.TabIndex = 4;
            this.mSliderV.Text = "cSlider";
            this.mSliderV.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
            this.mSliderV.ValueChanged += new System.EventHandler(this.mcSlider_ValueChanged);
            // 
            // mSliderB
            // 
            this.mSliderB.Location = new System.Drawing.Point(3, 223);
            this.mSliderB.Name = "mSliderB";
            this.mSliderB.Size = new System.Drawing.Size(338, 22);
            this.mSliderB.TabIndex = 4;
            this.mSliderB.Text = "cSlider";
            this.mSliderB.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
            this.mSliderB.ValueChanged += new System.EventHandler(this.mcSlider_ValueChanged);
            // 
            // mSliderG
            // 
            this.mSliderG.Location = new System.Drawing.Point(3, 200);
            this.mSliderG.Name = "mSliderG";
            this.mSliderG.Size = new System.Drawing.Size(338, 22);
            this.mSliderG.TabIndex = 4;
            this.mSliderG.Text = "cSlider";
            this.mSliderG.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
            this.mSliderG.ValueChanged += new System.EventHandler(this.mcSlider_ValueChanged);
            // 
            // mSliderR
            // 
            this.mSliderR.Location = new System.Drawing.Point(3, 177);
            this.mSliderR.Name = "mSliderR";
            this.mSliderR.Size = new System.Drawing.Size(338, 22);
            this.mSliderR.TabIndex = 4;
            this.mSliderR.Text = "cSlider";
            this.mSliderR.Value = new decimal(new int[] {
            0,
            0,
            0,
            0});
            this.mSliderR.ValueChanged += new System.EventHandler(this.mcSlider_ValueChanged);
            // 
            // lastColorBtn
            // 
            this.lastColorBtn.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.lastColorBtn.Location = new System.Drawing.Point(7, 2);
            this.lastColorBtn.Name = "lastColorBtn";
            this.lastColorBtn.Size = new System.Drawing.Size(24, 24);
            this.lastColorBtn.TabIndex = 10;
            this.lastColorBtn.UseVisualStyleBackColor = true;
            this.lastColorBtn.BackColor = System.Drawing.Color.White;
            this.lastColorBtn.Click += new System.EventHandler(this.cStore_Click);
            // 
            // COH_ColorPicker
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(616, 302);
            this.Controls.Add(this.panel1);
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(632, 340);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(632, 340);
            this.Name = "COH_ColorPicker";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "COH_ColorPicker";
            this.panel1.ResumeLayout(false);
            this.panel3.ResumeLayout(false);
            this.ColorSelection.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.colorWheelPicker)).EndInit();
            this.ResumeLayout(false);

        }


        #endregion


        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.PictureBox colorWheelPicker;
        private bool doSliderValueChange;

        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.Panel cwMarker;
        private System.Windows.Forms.GroupBox ColorSelection;

        private COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider mSliderR;
        private COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider mSliderG;
        private COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider mSliderB;
        private COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider mSliderV;

        private System.Drawing.Color oldColor;
        private System.Windows.Forms.Button cStore5;
        private System.Windows.Forms.Button cStore4;
        private System.Windows.Forms.Button cStore3;
        private System.Windows.Forms.Button cStore2;
        private System.Windows.Forms.Button cStore1;
        private System.Windows.Forms.Button cStore20;
        private System.Windows.Forms.Button cStore15;
        private System.Windows.Forms.Button cStore10;
        private System.Windows.Forms.Button cStore19;
        private System.Windows.Forms.Button cStore14;
        private System.Windows.Forms.Button cStore9;
        private System.Windows.Forms.Button cStore18;
        private System.Windows.Forms.Button cStore17;
        private System.Windows.Forms.Button cStore13;
        private System.Windows.Forms.Button cStore12;
        private System.Windows.Forms.Button cStore8;
        private System.Windows.Forms.Button cStore16;
        private System.Windows.Forms.Button cStore7;
        private System.Windows.Forms.Button cStore11;
        private System.Windows.Forms.Button cStore6;
        private System.Windows.Forms.Button cStore25;
        private System.Windows.Forms.Button cStore30;
        private System.Windows.Forms.Button cStore29;
        private System.Windows.Forms.Button cStore28;
        private System.Windows.Forms.Button cStore27;
        private System.Windows.Forms.Button cStore24;
        private System.Windows.Forms.Button cStore26;
        private System.Windows.Forms.Button cStore23;
        private System.Windows.Forms.Button cStore22;
        private System.Windows.Forms.Button cStore21;
        private System.Windows.Forms.Button cStore42;
        private System.Windows.Forms.Button cStore36;
        private System.Windows.Forms.Button cStore41;
        private System.Windows.Forms.Button cStore40;
        private System.Windows.Forms.Button cStore35;
        private System.Windows.Forms.Button cStore34;
        private System.Windows.Forms.Button cStore39;
        private System.Windows.Forms.Button cStore38;
        private System.Windows.Forms.Button cStore37;
        private System.Windows.Forms.Button cStore33;
        private System.Windows.Forms.Button cStore32;
        private System.Windows.Forms.Button cStore31;
        private System.Windows.Forms.Button cStore48;
        private System.Windows.Forms.Button cStore47;
        private System.Windows.Forms.Button cStore46;
        private System.Windows.Forms.Button cStore45;
        private System.Windows.Forms.Button cStore44;
        private System.Windows.Forms.Button cStore43;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Panel panel3;
        private System.Windows.Forms.Button lastColorBtn; 
    }
}