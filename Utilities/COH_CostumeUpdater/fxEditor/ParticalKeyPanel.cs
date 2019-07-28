using System;
using System.Collections.Generic;
using System.ComponentModel;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    class ParticalKeyPanel:BehaviorKeyPanel
    {
        private System.Windows.Forms.PictureBox alpha_picBx;
        
        private System.Windows.Forms.PictureBox rgb_picBx;
        
        private System.Windows.Forms.Button browserBtn;
        
        private System.Windows.Forms.TextBox textureNamePath_txBx;
        
        private System.Windows.Forms.ContextMenu iconCntxMn;
        
        private System.Windows.Forms.MenuItem sendToPhotoShop_menuItem;
        
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        
        private string rootPath;

        private int fieldsCount;
        
        private int decimal_places;
        
        private bool isColor;
        
        private bool isFlag;
        
        private bool isCkbx;
        
        private string partOriginalValue;
        
        private decimal [] defaultValues;

        public ParticalKeyPanel(object ownerPnl, string name, string key_label, int numFields, int min, int max, decimal[] defaultVals, int decimalPlaces,
            bool isColorField, bool isFlagField, object[] comboItems, int pWidth, bool isCkbxField, string rootpath, string grp, string subGrp) :
            base(ownerPnl, name, key_label, numFields, min, max, defaultVals, decimalPlaces, true, isColorField, isFlagField, comboItems, pWidth, isCkbxField, grp, subGrp)
        {
            
            resetPanel = false;
            fieldsCount = numFields;
            isColor = isColorField;
            isFlag = isFlagField;
            isCkbx = isCkbxField;
            decimal_places = decimalPlaces;
            defaultValues = null; 

            this.Name = name + "_ParticalKeyPanel";
            this.rgb_picBx = null;
            this.alpha_picBx = null;
            rootPath = rootpath.EndsWith(@"\") ? rootpath : rootpath + @"\";

            // 
            // overKeyName_ckbx
            // 
            this.overKeyName_ckbx.AutoSize = true;
            this.overKeyName_ckbx.Name = "overKeyName_ckbx";
            this.overKeyName_ckbx.Size = new System.Drawing.Size(101, 17);
            this.overKeyName_ckbx.TabIndex = 501;
            this.overKeyName_ckbx.Text = name;
            this.overKeyName_ckbx.UseVisualStyleBackColor = true;
            this.overKeyName_ckbx.Font = new System.Drawing.Font(this.Font, name.Equals(subGroup)? FontStyle.Bold:FontStyle.Regular);
            this.overKeyName_ckbx.CheckedChanged += new System.EventHandler(this.overKeyName_ckbx_CheckedChanged);       
            
            if (name.ToLower().StartsWith("texture"))
            {
                this.Height = 58;
                initializePartTexture(name);
            }
            else if (name.ToLower().StartsWith("dielikethis"))
            {
                initializeBroweserField();
                this.bhvr_combobx.Visible = false;
                this.units_label.Visible = false;
            }
            else
            {
                this.Height = 26;
                fixOverKeyNcombobx(name, comboItems, (int)defaultVals[0]);
            }
            hideEnableCkbs(!name.ToLower().StartsWith("texture"));
            this.BackColor = mainBackColor;
            this.Focus();
        }
        
        public void setOverText(string overName)
        {
            this.overKeyName_ckbx.Text = overName;
        }

        public override string getVal()
        {
            string val = "";

            bool cEnable = this.overKeyName_ckbx.Checked;
            if (!cEnable)
            {
                val = "#";
            }

            val += this.overKeyName_ckbx.Text + " ";

            string[] vals = { null, null, null, null };

            switch (fieldsCount)
            {
                case 0:
                    //if (!this.bhvr_combobx.Visible && textureNamePath_txBx != null)
                    if (!this.isCombo && textureNamePath_txBx != null)
                        vals[0] = textureNamePath_txBx.Text;
                    break;

                case 1:
                    vals[0] = keyVal1_numericUpDown.Value.ToString();
                    break;

                case 2:
                    vals[0] = keyVal1_numericUpDown.Value.ToString();
                    vals[1] = keyVal2_numericUpDown.Value.ToString();
                    break;

                case 3:
                    vals[0] = keyVal1_numericUpDown.Value.ToString();
                    vals[1] = keyVal2_numericUpDown.Value.ToString();
                    vals[2] = keyVal3_numericUpDown.Value.ToString();
                    break;

                case 5:
                    vals[0] = keyVal1_numericUpDown.Value.ToString();
                    vals[1] = keyVal2_numericUpDown.Value.ToString();
                    vals[2] = bhvr_combobx.Text;

                    if (this.rotateX_ckbx.Checked)
                        vals[3] = rotateX_ckbx.Text;
                    break;
            }

            if (isFlag && !isCombo)//!bhvr_combobx.Visible)
            {
                //this.flag_ckbx.Visible = false;
                //this.overKeyName_ckbx.Visible = false;
                //this.flag_ckbx.Text = name;
            }

            if (isCombo &&
                fieldsCount == 0)
            {
                if(this.overKeyName_ckbx.Text.Equals("Blend_mode"))
                    vals[0] = bhvr_combobx.SelectedIndex.ToString();
                else
                    vals[0] = bhvr_combobx.Text.Replace("(","# (");
            }

            foreach (string str in vals)
            {
                if (str != null)
                {
                    decimal d;

                    if (decimal.TryParse(str, out d))
                    {
                        if (isColor)
                        {
                            int c = (int)Math.Ceiling(d);

                            val += c.ToString() + " ";
                        }
                        else if (decimal_places == 0)
                        {
                            int c = (int)Math.Ceiling(d);

                            val += c.ToString() + " ";
                        }
                        else
                        {
                            if (str.Contains("."))
                            {
                                string[] valSplit = str.Split('.');

                                int m = 0;

                                int.TryParse(valSplit[1], out m);

                                if (m == 0)
                                    val += valSplit[0] + " ";
                                else
                                    val += d.ToString() + " ";
                            }
                            else
                                val += d.ToString() + " ";
                        }
                    }
                    else
                    {
                        val += str + " ";
                    }
                }
            }
            /*
            if (isCkbx)
            {
                if (rotateX_ckbx.Checked) val += "RotateX ";
                if (rotateY_ckbx.Checked) val += "RotateY ";
                if (rotateZ_ckbx.Checked) val += "RotateZ ";
                if (translateX_ckbx.Checked) val += "TranslateX ";
                if (translateY_ckbx.Checked) val += "TranslateY ";
                if (translateZ_ckbx.Checked) val += "TranslateZ ";
            }
             */
            

            return val.Trim();
        }

        public void setVal(object sender, string val, bool lockedFile)
        {
            reset();

            isLocked = lockedFile;

            isDirty = ((Partical)this.Tag).isDirty;

            //to avoid loop when overKeyName_ckbox.Checked and numValue changes
            resetPanel = true;

            partOriginalValue = val;

            bool cEnable = val != null;

            isUsed = cEnable;
            
            if (cEnable)
            {
                if (val.StartsWith("#"))
                {
                    cEnable = false;
                    val = val.Trim().Substring(1);
                }

                if (val.Contains("#"))
                {
                    val = val.Trim().Substring(0, val.IndexOf('#') - 1);
                }

                string[] vals = val.Trim().Split(' ');

                decimal v1, v2, v3;

                bool success = false;

                switch (fieldsCount)
                {
                    case 0:
                        success = decimal.TryParse(vals[0], out v1);
                        if (success)
                        {
                            if (isCombo)
                                setComboBox(val, v1);
                            else
                            {
                                if (cEnable && 
                                    !overKeyName_ckbx.Text.ToLower().Equals("TextureName2".ToLower()))
                                    MessageBox.Show("name: " + overKeyName_ckbx.Text + "\nval: " + val);
                                
                                cEnable = false;
                            }
                        }
                        else
                        {
                            if (overKeyName_ckbx.Text.StartsWith("TextureName"))
                            {
                                if (val != null &&
                                   val.Length > 0 &&
                                   System.IO.Path.HasExtension(val))
                                {
                                    this.openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(val);
                                }
                                this.textureNamePath_txBx.Tag = val;

                                this.textureNamePath_txBx.Text =  System.IO.Path.GetFileName(val);
                            }
                            else if (this.overKeyName_ckbx.Text.ToLower().Equals("dielikethis") &&
                                    val.ToLower().EndsWith(".part"))
                            {
                                this.textureNamePath_txBx.Text = val;
                            }
                            else if (isCombo)
                                setComboBox(val, v1);
                            else
                                MessageBox.Show("name: " + overKeyName_ckbx.Text + "\nval: " + val);
                        }
                        break;

                    case 1:
                        success = decimal.TryParse(vals[0], out v1);
                        if (success)
                        {
                            fixMinMax(v1, keyVal1_numericUpDown);
                            keyVal1_numericUpDown.Value = v1;
                        }
                        break;

                    case 2://testing the vals length due to errors in file used in the game and everyone is afraid to fix the error!!!!!!!!!
                        if (vals.Length > 0)
                        {
                            success = decimal.TryParse(vals[0], out v1);
                            if (success)
                            {
                                fixMinMax(v1, keyVal1_numericUpDown);
                                keyVal1_numericUpDown.Value = v1;
                            }
                        }
                        if (vals.Length > 2)
                        {
                            //used to be vals[2] no one found this bug, so I chaged it on 6/8/2011
                            success = decimal.TryParse(vals[1], out v2);
                            if (success)
                            {
                                fixMinMax(v2, keyVal2_numericUpDown);
                                keyVal2_numericUpDown.Value = v2;
                            }
                        }
                        break;

                    case 3://testing the vals length due to errors in file used in the game and everyone is afraid to fix the error!!!!!!!!!
                        if (vals.Length > 0)
                        {
                            success = decimal.TryParse(vals[0], out v1);
                            if (success)
                            {
                                fixMinMax(v1, keyVal1_numericUpDown);
                                keyVal1_numericUpDown.Value = v1;
                            }
                        }
                        if (vals.Length > 1)
                        {
                            success = decimal.TryParse(vals[1], out v2);
                            if (success)
                            {
                                fixMinMax(v2, keyVal2_numericUpDown);
                                keyVal2_numericUpDown.Value = v2;
                            }
                        }
                        if (vals.Length > 2)
                        {
                            success = decimal.TryParse(vals[2], out v3);
                            if (success)
                            {
                                fixMinMax(v3, keyVal3_numericUpDown);
                                keyVal3_numericUpDown.Value = v3;
                            }
                        }
                        if (isColor)
                            setColorBox();

                        break;

                    case 5://testing the vals length due to errors in file used in the game and everyone is afraid to fix the error!!!!!!!!!
                        if (vals.Length > 0)
                        {
                            success = decimal.TryParse(vals[0], out v1);
                            if (success)
                            {
                                fixMinMax(v1, keyVal1_numericUpDown);
                                keyVal1_numericUpDown.Value = v1;
                            }
                        }
                        if (vals.Length > 1)
                        {
                            success = decimal.TryParse(vals[1], out v2);
                            if (success)
                            {
                                fixMinMax(v2, keyVal2_numericUpDown);
                                keyVal2_numericUpDown.Value = v2;
                            }
                        }
                        if (vals.Length > 2)
                        {
                            if (bhvr_combobx.Items.Contains(vals[2]))
                                bhvr_combobx.Text = vals[2];
                        }
                        if (vals.Length == 4 && rotateX_ckbx.Text.Equals(vals[3]))
                            this.rotateX_ckbx.Checked = true;
                        break;
                }

                this.overKeyName_ckbx.Checked = cEnable;
            }

            resetPanel = false;
        }

        private void reset()
        {
            resetPanel = true;

            partOriginalValue = null;

            setDefaultVal(defaultValues);

            if (textureNamePath_txBx != null)
            {
                textureNamePath_txBx.Text = "";

                textureNamePath_txBx.Tag = null;
            }

            if (flag_ckbx != null)
                flag_ckbx.Checked = false;

            if (bhvr_combobx != null && bhvr_combobx.Items.Count > 0)
                bhvr_combobx.Text = (string)bhvr_combobx.Items[0];

            this.overKeyName_ckbx.Checked = false;

            resetPanel = false;
        }

        private void setComboBox(string val, decimal v1)
        {
            if (char.IsNumber(val, 0) && this.bhvr_combobx.Items.Count > v1)
            {
                this.bhvr_combobx.Text = (string)this.bhvr_combobx.Items[(int)v1];
            }
            else
            {
                foreach (string strItem in this.bhvr_combobx.Items)
                {
                    int startInd = strItem.Contains("(") ? 1 : 0;

                    string innerStr = strItem.Replace("(", "").Replace(")", "").Trim().Substring(startInd).Trim().ToLower();

                    if (strItem.ToLower().StartsWith(val))
                    {
                        this.bhvr_combobx.Text = strItem;
                        return;
                    }
                    else if (innerStr.Equals(val.ToLower()))
                    {
                        this.bhvr_combobx.Text = strItem;
                        return;
                    }
                }
            }
        }
        
        private void fixMinMax(decimal dValue, NumericUpDown numUpDwn)
        {
            if (dValue > numUpDwn.Maximum)
                numUpDwn.Maximum = dValue;

            if (dValue < numUpDwn.Minimum)
                numUpDwn.Minimum = dValue;

            int dPalces;

            if (!isColor && dValue.ToString().IndexOf('.') > -1)
            {
                dPalces = dValue.ToString().Substring(dValue.ToString().IndexOf('.')).Length;
                numUpDwn.DecimalPlaces = dPalces;
            }
        }

        private void setDefaultVal(decimal[] dVals)
        {
            if (dVals != null)
            {
                this.keyVal1_numericUpDown.Value = dVals[0];
                if (dVals.Length > 1)
                    this.keyVal2_numericUpDown.Value = dVals[1];
                if (dVals.Length > 2)
                    this.keyVal3_numericUpDown.Value = dVals[2];
            }
        }

        private void setMinMax(int min, int max, int decimalPlaces)
        {
            this.keyVal1_numericUpDown.Maximum = new decimal(max);
            this.keyVal1_numericUpDown.Minimum = new decimal(min);
            this.keyVal1_numericUpDown.DecimalPlaces = decimalPlaces;

            this.keyVal2_numericUpDown.Maximum = new decimal(max);
            this.keyVal2_numericUpDown.Minimum = new decimal(min);
            this.keyVal2_numericUpDown.DecimalPlaces = decimalPlaces;

            this.keyVal3_numericUpDown.Maximum = new decimal(max);
            this.keyVal3_numericUpDown.Minimum = new decimal(min);
            this.keyVal3_numericUpDown.DecimalPlaces = decimalPlaces;
        }

        public override void enable(bool enableControls)
        {
            foreach (Control ctl in this.Controls)
            {
                ctl.Enabled = enableControls;
            }

            overKeyName_ckbx.Enabled = true;

            if (!overKeyName_ckbx.Visible)
            {
                flag_ckbx.Enabled = true;
            }

            if (enableControls)
            {
                this.BackColor = mainBackColor;
            }
            else
            {
                this.BackColor = Color.FromArgb(Math.Max(0, (int)mainBackColor.R - 50),
                                                Math.Max(0, (int)mainBackColor.G),
                                                Math.Max(0, (int)mainBackColor.B - 50));
            }
        }

        private void fixOverKeyNcombobx(string name, object [] comboItems, int defaultVal)
        {
            this.overKeyName_ckbx.Location = new System.Drawing.Point(3, 5);

            if (comboItems != null)
                this.bhvr_combobx.Text = (string)comboItems[defaultVal];

            this.bhvr_combobx.Size = new Size(200, 21);
            this.bhvr_combobx.AutoCompleteMode = AutoCompleteMode.SuggestAppend;
            bhvr_combobx.AutoCompleteSource = AutoCompleteSource.ListItems;
            this.Focus();
        }

        private void initializeBroweserField()
        {
            this.textureNamePath_txBx = new System.Windows.Forms.TextBox();
            this.browserBtn = new System.Windows.Forms.Button();
            this.openFileDialog1 = new OpenFileDialog();

            this.openFileDialog1.InitialDirectory = rootPath + @"data\fx\";
            openFileDialog1.Filter = "partical file (*.part)|*.part";
            this.SuspendLayout();
            // 
            // textureNamePath_txBx
            // 
            this.textureNamePath_txBx.Location = new System.Drawing.Point(111, 3);
            this.textureNamePath_txBx.Name = "textureNamePath_txBx";
            this.textureNamePath_txBx.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.textureNamePath_txBx.Size = new System.Drawing.Size(410, 20);
            this.textureNamePath_txBx.TabIndex = 1;
            this.textureNamePath_txBx.TextChanged += new EventHandler(textureNamePath_txBx_TextChanged);
            // 
            // browserBtn
            // 
            this.browserBtn.Location = new System.Drawing.Point(522, 3);
            this.browserBtn.Name = "browserBtn";
            this.browserBtn.Size = new System.Drawing.Size(45, 20);
            this.browserBtn.TabIndex = 2;
            this.browserBtn.Text = "...";
            this.browserBtn.UseVisualStyleBackColor = true;
            this.browserBtn.Click += new EventHandler(browserBtn_Click);


            this.Controls.Add(this.browserBtn);
            this.Controls.Add(this.textureNamePath_txBx);

            this.ResumeLayout(false);

        }
        
        private void initializePartTexture(string name)
        {
            this.textureNamePath_txBx = new System.Windows.Forms.TextBox();
            this.browserBtn = new System.Windows.Forms.Button();
            this.rgb_picBx = new System.Windows.Forms.PictureBox();
            this.alpha_picBx = new System.Windows.Forms.PictureBox();
            this.openFileDialog1 = new OpenFileDialog();
            this.iconCntxMn = new System.Windows.Forms.ContextMenu();
            this.sendToPhotoShop_menuItem = new System.Windows.Forms.MenuItem();


            ((System.ComponentModel.ISupportInitialize)(this.rgb_picBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.alpha_picBx)).BeginInit();
            this.SuspendLayout();

            this.openFileDialog1.InitialDirectory = rootPath + @"src\Texture_Library\";
            openFileDialog1.Filter = "TGA file (*.tga)|*.tga";

            this.iconCntxMn.MenuItems.Add(this.sendToPhotoShop_menuItem);
            // 
            // sendToPhotoShop_menuItem
            // 
            this.sendToPhotoShop_menuItem.Index = 0;
            this.sendToPhotoShop_menuItem.Text = "Send To Photoshop";
            this.sendToPhotoShop_menuItem.Click += new System.EventHandler(sendToPhotoShop_menuItem_Click);
            // 
            // textureNamePath_txBx
            // 
            this.textureNamePath_txBx.Location = new System.Drawing.Point(111, 33);
            this.textureNamePath_txBx.Name = "textureNamePath_txBx";
            this.textureNamePath_txBx.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.textureNamePath_txBx.Size = new System.Drawing.Size(410, 20);
            this.textureNamePath_txBx.TabIndex = 1;
            this.textureNamePath_txBx.TextChanged += new EventHandler(textureNamePath_txBx_TextChanged);
            // 
            // browserBtn
            // 
            this.browserBtn.Location = new System.Drawing.Point(522, 31);
            this.browserBtn.Name = "browserBtn";
            this.browserBtn.Size = new System.Drawing.Size(45, 23);
            this.browserBtn.TabIndex = 2;
            this.browserBtn.Text = "...";
            this.browserBtn.UseVisualStyleBackColor = true;
            this.browserBtn.Click += new EventHandler(browserBtn_Click);
            // 
            // rgb_picBx
            // 
            this.rgb_picBx.BackgroundImage = global::COH_CostumeUpdater.Properties.Resources.picBoxBkGrndImg;
            this.rgb_picBx.Location = new System.Drawing.Point(5, 5);
            this.rgb_picBx.Name = "rgb_picBx";
            this.rgb_picBx.Size = new System.Drawing.Size(48, 48);
            this.rgb_picBx.TabIndex = 3;
            this.rgb_picBx.TabStop = false;
            this.rgb_picBx.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.rgb_picBx.ContextMenu = this.iconCntxMn;
            this.rgb_picBx.MouseDoubleClick += new MouseEventHandler(tgaIconPicBx_MouseDoubleClick);
            // 
            // alpha_picBx
            // 
            this.alpha_picBx.BackgroundImage = global::COH_CostumeUpdater.Properties.Resources.picBoxBkGrndImg;
            this.alpha_picBx.InitialImage = null;
            this.alpha_picBx.Location = new System.Drawing.Point(59, 5);
            this.alpha_picBx.Name = "alpha_picBx";
            this.alpha_picBx.Size = new System.Drawing.Size(48, 48);
            this.alpha_picBx.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.alpha_picBx.TabIndex = 3;
            this.alpha_picBx.TabStop = false;
            // 
            // overKeyName_ckbx
            // 
            this.overKeyName_ckbx.Location = new System.Drawing.Point(115, 10);
            this.overKeyName_ckbx.Visible = true;
            this.flag_ckbx.Visible = false;
            
            this.Controls.Add(this.browserBtn);
            this.Controls.Add(this.textureNamePath_txBx);
            this.Controls.Add(this.alpha_picBx);
            this.Controls.Add(this.rgb_picBx);

            
            ((System.ComponentModel.ISupportInitialize)(this.rgb_picBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.alpha_picBx)).EndInit();
            this.ResumeLayout(false);
        }

        private void textureNamePath_txBx_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Execute)
                textureNamePath_txBx_TextChanged(textureNamePath_txBx, e);
        }

        public void showCommentCheckBx()
        {
            this.overKeyName_ckbx.Visible = true;
            this.overKeyName_ckbx.Text = "";
            this.flag_ckbx.Location = new System.Drawing.Point(43, 5);
        }
        
        public void setChecked(bool keyChecked)
        {
            overKeyName_ckbx.Checked = keyChecked;
        }

        private void browserBtn_Click(object sender, EventArgs e)
        {
            string oldPath = openFileDialog1.InitialDirectory;
            string oldFileName = openFileDialog1.FileName;

            if (System.IO.File.Exists(textureNamePath_txBx.Text))
            {
                openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(textureNamePath_txBx.Text);
            }
            else if (textureNamePath_txBx.Tag != null && System.IO.File.Exists((string)textureNamePath_txBx.Tag))
            {
                openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName((string)textureNamePath_txBx.Tag);
            }
            else if (rootPath != null)
            {
                string root = rootPath;

                if(root.Contains("data\\"))
                    root = root.Substring(0, root.IndexOf("data\\"));

                openFileDialog1.InitialDirectory = root + @"src\texture_library";
            }
            
            DialogResult dr = openFileDialog1.ShowDialog();

            if (dr == DialogResult.OK)
            {
                textureNamePath_txBx.Tag = openFileDialog1.FileName;
                
                textureNamePath_txBx.Text = System.IO.Path.GetFileName(openFileDialog1.FileName);
                
                ParticalsWin pWin = (ParticalsWin)ownerPanel;

                if (pWin != null)
                {
                    string path = pWin.getTexturePath(textureNamePath_txBx.Text);
                    
                    if (path == null)
                    {
                        pWin.addToTextureDic(openFileDialog1.FileName);
                    }
                }
            }

            if ((oldPath == null || oldPath.Trim().Length == 0) && oldFileName != null)
                oldPath = System.IO.Path.GetDirectoryName(oldFileName);

            openFileDialog1.InitialDirectory = oldPath;
        }

        private void textureNamePath_txBx_TextChanged(object sender, EventArgs e)
        {
            if (rgb_picBx != null)
            {
                rgb_picBx.Image = null;
                alpha_picBx.Image = null;
            }            

            if (textureNamePath_txBx.Text.ToLower().EndsWith(".tga"))
            {
                string path = null;

                if (textureNamePath_txBx.Tag != null &&
                    ((string)textureNamePath_txBx.Tag).EndsWith(textureNamePath_txBx.Text))
                {
                    path = (string)textureNamePath_txBx.Tag;
                }
                else
                {
                    ParticalsWin pWin = (ParticalsWin)ownerPanel;

                    if (pWin != null)
                    {
                        path = pWin.getTexturePath(textureNamePath_txBx.Text);
                        
                        textureNamePath_txBx.Tag = path;

                        if (path == null)
                        {
                            pWin.addToTextureDic(openFileDialog1.FileName);
                        }
                    }
                }

                if (rgb_picBx != null && path != null)
                    setIconImage(path);
            }

            base.keyVal_numericUpDown_ValueChanged(sender, e);
        }

        private void setIconImage(string path)
        {
            try
            {
                PresentParameters pp = new PresentParameters();
                
                pp.Windowed = true;
                
                pp.SwapEffect = SwapEffect.Copy;
                
                Device device = new Device(0, DeviceType.Hardware, this.rgb_picBx, CreateFlags.HardwareVertexProcessing, pp);
                
                Microsoft.DirectX.Direct3D.Texture tx = TextureLoader.FromFile(device, path);
                
                Microsoft.DirectX.GraphicsStream gs = TextureLoader.SaveToStream(ImageFileFormat.Dib, tx);

                Bitmap RGB_bitmap = new Bitmap(gs);
                
                Bitmap alpha_bitmap = new Bitmap(gs);

                rgb_picBx.Image = RGB_bitmap;

                //alpha_bitmap.MakeTransparent();
                gs.Seek(0, 0);

                System.Drawing.Imaging.BitmapData bmpData = alpha_bitmap.LockBits(new Rectangle(0, 0, alpha_bitmap.Width, alpha_bitmap.Height),
                                                                            System.Drawing.Imaging.ImageLockMode.ReadWrite,
                                                                            alpha_bitmap.PixelFormat);

                // Get the address of the first line.
                IntPtr ptr = bmpData.Scan0;

                // Declare an array to hold the bytes of the bitmap.
                int bytes = Math.Abs(bmpData.Stride) * alpha_bitmap.Height;

                int gsLen = (int)gs.Length;

                byte[] rgbaValues = new byte[bytes];

                byte[] gsBytes = new byte[gsLen];

                gs.Read(gsBytes, 0, gsLen);

                int gsLastI = gsLen - 1;

                // Copy the RGB values into the array.
                System.Runtime.InteropServices.Marshal.Copy(ptr, rgbaValues, 0, bytes);
                
                int i = 0;
                
                for (int y = 0; y < bmpData.Height; y++)
                {
                    for (int x = 0; x < bmpData.Width; x++)
                    {
                        byte g = gsBytes[gsLastI - i++],
                             b = gsBytes[gsLastI - i++],
                             a = gsBytes[gsLastI - i++],
                             r = gsBytes[gsLastI - i++];

                        Color pClr = Color.FromArgb(255, a, a, a);

                        System.Runtime.InteropServices.Marshal.WriteInt32(bmpData.Scan0, (bmpData.Stride * y) + (4 * x), pClr.ToArgb());
                    }

                }

                alpha_bitmap.UnlockBits(bmpData);

                alpha_bitmap.RotateFlip(RotateFlipType.Rotate180FlipY);

                alpha_picBx.Image = alpha_bitmap;
                
                gs.Dispose();
                
                tx.Dispose();
                
                device.Dispose();
            }

            catch (Exception ex)
            { 
            }
        }

        private void sendToPhotoShop_menuItem_Click(object sender, System.EventArgs e)
        {
            string path = (string)textureNamePath_txBx.Tag;
            
            if (System.IO.File.Exists(path))
            {
                System.Diagnostics.Process.Start(path);
            }
        }
        
        private void tgaIconPicBx_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            try
            {
                viewTga((string)textureNamePath_txBx.Tag, true);
            }
            catch (Exception ex)
            {
            }
        }

        private string viewTga(string path, bool show)
        {
            string results = "";
            
            common.TgaBox tbx = new common.TgaBox(path, true);
            
            if (show)
                tbx.ShowDialog();
            else
            {
                results = tbx.getImageSize();
            }
            return results;
        }
        
        protected override void overKeyName_ckbx_CheckedChanged(object sender, EventArgs e)
        {
            if (!resetPanel)
            {
                enable(this.overKeyName_ckbx.Checked);

                //if (Name.StartsWith("AlwaysDraw") || Name.StartsWith("IgnoreFxTint"))
                {
                    if (!isDirty && !resetPanel)
                    {
                        isDirty = true;
                        this.invokeIsDirtyChanged();
                    }
                }
            }
        }
    }
}
