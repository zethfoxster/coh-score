using System;
using System.ComponentModel;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.assetEditor.materialTrick
{
    class FB_GlobalOptionsPnl:GroupBox
    {
        private decimal dIncrement;
        private decimal dMaximum;
        private decimal dMinimum;
        private System.Windows.Forms.GroupBox scaleSettingGBx;
        private System.Windows.Forms.GroupBox scaleST0_GBx;
        private System.Windows.Forms.NumericUpDown uScaleST0numUpDwn;
        private System.Windows.Forms.GroupBox scaleST1_GBx;
        private System.Windows.Forms.Label vScaleST1Label;
        private System.Windows.Forms.Label uScaleST1Label;
        private System.Windows.Forms.NumericUpDown vScaleST1numUpDwn;
        private System.Windows.Forms.NumericUpDown uScaleST1numUpDwn;
        private System.Windows.Forms.Label vSacaleST0Label;
        private System.Windows.Forms.Label uScaleST0Label;
        private System.Windows.Forms.NumericUpDown vScaleST0numUpDwn;
        private System.Windows.Forms.GroupBox lightingSettingGBx;
        private System.Windows.Forms.GroupBox ambientScaleGBx;
        private System.Windows.Forms.NumericUpDown ambientScaleNumUpDwn;
        private System.Windows.Forms.GroupBox diffuseScale;
        private System.Windows.Forms.NumericUpDown diffuseScaleNumUpDwn;

        private System.ComponentModel.IContainer components = null;


        public FB_GlobalOptionsPnl()
        {
            this.dIncrement = new decimal(new int[] { 5, 0, 0, 65536 });
            this.dMaximum = new decimal(new int[] { 10000000, 0, 0, 0 });
            this.dMinimum = new decimal(new int[] { 10000000, 0, 0, -2147483648 });
            this.InitializeComponent();
        }
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {       
            this.scaleSettingGBx = new System.Windows.Forms.GroupBox();
            this.uScaleST0numUpDwn = new System.Windows.Forms.NumericUpDown();
            this.scaleST0_GBx = new System.Windows.Forms.GroupBox();
            this.vScaleST0numUpDwn = new System.Windows.Forms.NumericUpDown();
            this.uScaleST0Label = new System.Windows.Forms.Label();
            this.vSacaleST0Label = new System.Windows.Forms.Label();
            this.scaleST1_GBx = new System.Windows.Forms.GroupBox();
            this.vScaleST1Label = new System.Windows.Forms.Label();
            this.uScaleST1Label = new System.Windows.Forms.Label();
            this.vScaleST1numUpDwn = new System.Windows.Forms.NumericUpDown();
            this.uScaleST1numUpDwn = new System.Windows.Forms.NumericUpDown();
            this.lightingSettingGBx = new System.Windows.Forms.GroupBox();
            this.ambientScaleGBx = new System.Windows.Forms.GroupBox();
            this.ambientScaleNumUpDwn = new System.Windows.Forms.NumericUpDown();
            this.diffuseScale = new System.Windows.Forms.GroupBox();
            this.diffuseScaleNumUpDwn = new System.Windows.Forms.NumericUpDown();

            this.SuspendLayout();         

            this.scaleSettingGBx.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.uScaleST0numUpDwn)).BeginInit();
            this.scaleST0_GBx.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.vScaleST0numUpDwn)).BeginInit();
            this.scaleST1_GBx.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.vScaleST1numUpDwn)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.uScaleST1numUpDwn)).BeginInit();
            this.lightingSettingGBx.SuspendLayout();
            this.ambientScaleGBx.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.ambientScaleNumUpDwn)).BeginInit();
            this.diffuseScale.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.diffuseScaleNumUpDwn)).BeginInit();
            this.SuspendLayout();
            // 
            // fbGlobalOptionsPanel
            // 
            this.Controls.Add(this.lightingSettingGBx);
            this.Controls.Add(this.scaleSettingGBx);
            this.Dock = System.Windows.Forms.DockStyle.None;
            this.Location = new System.Drawing.Point(10, 20);
            this.Name = "FB_GlobalOptionsPnl";
            this.Text = "Fallback Global Settings";
            this.RightToLeft = RightToLeft.Yes;
            this.Size = new System.Drawing.Size(970, 115);
            this.TabIndex = 0;
            // 
            // scaleSettingGBx
            // 
            this.scaleSettingGBx.Controls.Add(this.scaleST1_GBx);
            this.scaleSettingGBx.Controls.Add(this.scaleST0_GBx);
            this.scaleSettingGBx.Location = new System.Drawing.Point(15, 25);
            this.scaleSettingGBx.Name = "scaleSettingGBx";
            this.scaleSettingGBx.Size = new System.Drawing.Size(442, 81);
            this.scaleSettingGBx.TabIndex = 0;
            this.scaleSettingGBx.TabStop = false;
            this.scaleSettingGBx.Text = "Scale Settings";
            this.scaleSettingGBx.RightToLeft = RightToLeft.No;
            // 
            // uScaleST0numUpDwn
            // 
            this.uScaleST0numUpDwn.DecimalPlaces = 4;
            this.uScaleST0numUpDwn.Increment = this.dIncrement;
            this.uScaleST0numUpDwn.Maximum = this.dMaximum;
            this.uScaleST0numUpDwn.Minimum = this.dMinimum;
            this.uScaleST0numUpDwn.Location = new System.Drawing.Point(25, 19);
            this.uScaleST0numUpDwn.Name = "uScaleST0numUpDwn";
            this.uScaleST0numUpDwn.Size = new System.Drawing.Size(70, 20);
            this.uScaleST0numUpDwn.TabIndex = 0;
            this.uScaleST0numUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.uScaleST0numUpDwn.Value = 1.0m;
            // 
            // scaleST0_GBx
            // 
            this.scaleST0_GBx.Controls.Add(this.vSacaleST0Label);
            this.scaleST0_GBx.Controls.Add(this.uScaleST0Label);
            this.scaleST0_GBx.Controls.Add(this.vScaleST0numUpDwn);
            this.scaleST0_GBx.Controls.Add(this.uScaleST0numUpDwn);
            this.scaleST0_GBx.Location = new System.Drawing.Point(7, 23);
            this.scaleST0_GBx.Name = "scaleST0_GBx";
            this.scaleST0_GBx.Size = new System.Drawing.Size(206, 48);
            this.scaleST0_GBx.TabIndex = 0;
            this.scaleST0_GBx.TabStop = false;
            this.scaleST0_GBx.Text = "Scale ST0";
            // 
            // vScaleST0numUpDwn
            // 
            this.vScaleST0numUpDwn.DecimalPlaces = 4;
            this.vScaleST0numUpDwn.Increment = this.dIncrement;
            this.vScaleST0numUpDwn.Maximum = this.dMaximum;
            this.vScaleST0numUpDwn.Minimum = this.dMinimum;
            this.vScaleST0numUpDwn.Location = new System.Drawing.Point(128, 19);
            this.vScaleST0numUpDwn.Name = "vScaleST0numUpDwn";
            this.vScaleST0numUpDwn.Size = new System.Drawing.Size(70, 20);
            this.vScaleST0numUpDwn.TabIndex = 0;
            this.vScaleST0numUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.vScaleST0numUpDwn.Value = 1.0m;
            // 
            // uScaleST0Label
            // 
            this.uScaleST0Label.AutoSize = true;
            this.uScaleST0Label.Location = new System.Drawing.Point(4, 23);
            this.uScaleST0Label.Name = "uScaleST0Label";
            this.uScaleST0Label.Size = new System.Drawing.Size(18, 13);
            this.uScaleST0Label.TabIndex = 1;
            this.uScaleST0Label.Text = "U:";
            this.uScaleST0Label.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // vSacaleST0Label
            // 
            this.vSacaleST0Label.AutoSize = true;
            this.vSacaleST0Label.Location = new System.Drawing.Point(108, 23);
            this.vSacaleST0Label.Name = "vSacaleST0Label";
            this.vSacaleST0Label.Size = new System.Drawing.Size(17, 13);
            this.vSacaleST0Label.TabIndex = 1;
            this.vSacaleST0Label.Text = "V:";
            this.vSacaleST0Label.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // scaleST1_GBx
            // 
            this.scaleST1_GBx.Controls.Add(this.vScaleST1Label);
            this.scaleST1_GBx.Controls.Add(this.uScaleST1Label);
            this.scaleST1_GBx.Controls.Add(this.vScaleST1numUpDwn);
            this.scaleST1_GBx.Controls.Add(this.uScaleST1numUpDwn);
            this.scaleST1_GBx.Location = new System.Drawing.Point(230, 23);
            this.scaleST1_GBx.Name = "scaleST1_GBx";
            this.scaleST1_GBx.Size = new System.Drawing.Size(206, 48);
            this.scaleST1_GBx.TabIndex = 0;
            this.scaleST1_GBx.TabStop = false;
            this.scaleST1_GBx.Text = "Scale ST1";
            // 
            // vScaleST1Label
            // 
            this.vScaleST1Label.AutoSize = true;
            this.vScaleST1Label.Location = new System.Drawing.Point(108, 23);
            this.vScaleST1Label.Name = "vScaleST1Label";
            this.vScaleST1Label.Size = new System.Drawing.Size(17, 13);
            this.vScaleST1Label.TabIndex = 1;
            this.vScaleST1Label.Text = "V:";
            // 
            // uScaleST1Label
            // 
            this.uScaleST1Label.AutoSize = true;
            this.uScaleST1Label.Location = new System.Drawing.Point(4, 23);
            this.uScaleST1Label.Name = "uScaleST1Label";
            this.uScaleST1Label.Size = new System.Drawing.Size(18, 13);
            this.uScaleST1Label.TabIndex = 1;
            this.uScaleST1Label.Text = "U:";
            // 
            // vScaleST1numUpDwn
            // 
            this.vScaleST1numUpDwn.DecimalPlaces = 4;
            this.vScaleST1numUpDwn.Increment = this.dIncrement;
            this.vScaleST1numUpDwn.Maximum = this.dMaximum;
            this.vScaleST1numUpDwn.Minimum = this.dMinimum;
            this.vScaleST1numUpDwn.Location = new System.Drawing.Point(128, 19);
            this.vScaleST1numUpDwn.Name = "vScaleST1numUpDwn";
            this.vScaleST1numUpDwn.Size = new System.Drawing.Size(70, 20);
            this.vScaleST1numUpDwn.TabIndex = 0;
            this.vScaleST1numUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.vScaleST1numUpDwn.Value = 1.0m;
            // 
            // uScaleST1numUpDwn
            // 
            this.uScaleST1numUpDwn.DecimalPlaces = 4;
            this.uScaleST1numUpDwn.Increment = this.dIncrement;
            this.uScaleST1numUpDwn.Maximum = this.dMaximum;
            this.uScaleST1numUpDwn.Minimum = this.dMinimum;
            this.uScaleST1numUpDwn.Location = new System.Drawing.Point(25, 19);
            this.uScaleST1numUpDwn.Name = "uScaleST1numUpDwn";
            this.uScaleST1numUpDwn.Size = new System.Drawing.Size(70, 20);
            this.uScaleST1numUpDwn.TabIndex = 0;
            this.uScaleST1numUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.uScaleST1numUpDwn.Value = 1.0m;
            // 
            // lgihtingSettingGBx
            // 
            this.lightingSettingGBx.Controls.Add(this.ambientScaleGBx);
            this.lightingSettingGBx.Controls.Add(this.diffuseScale);
            this.lightingSettingGBx.Location = new System.Drawing.Point(495, 25);
            this.lightingSettingGBx.Name = "lgihtingSettingGBx";
            this.lightingSettingGBx.Size = new System.Drawing.Size(377, 81);
            this.lightingSettingGBx.TabIndex = 0;
            this.lightingSettingGBx.TabStop = false;
            this.lightingSettingGBx.Text = "Lighting Settings";
            this.lightingSettingGBx.RightToLeft = RightToLeft.No;
            // 
            // ambientScaleGBx
            // 
            this.ambientScaleGBx.Controls.Add(this.ambientScaleNumUpDwn);
            this.ambientScaleGBx.Location = new System.Drawing.Point(198, 23);
            this.ambientScaleGBx.Name = "ambientScaleGBx";
            this.ambientScaleGBx.Size = new System.Drawing.Size(170, 48);
            this.ambientScaleGBx.TabIndex = 0;
            this.ambientScaleGBx.TabStop = false;
            this.ambientScaleGBx.Text = "Ambient Scale";
            // 
            // ambientScaleNumUpDwn
            // 
            this.ambientScaleNumUpDwn.DecimalPlaces = 4;
            this.ambientScaleNumUpDwn.Increment = this.dIncrement;
            this.ambientScaleNumUpDwn.Maximum = this.dMaximum;
            this.ambientScaleNumUpDwn.Minimum = this.dMinimum;
            this.ambientScaleNumUpDwn.Location = new System.Drawing.Point(28, 19);
            this.ambientScaleNumUpDwn.Name = "ambientScaleNumUpDwn";
            this.ambientScaleNumUpDwn.Size = new System.Drawing.Size(114, 20);
            this.ambientScaleNumUpDwn.TabIndex = 0;
            this.ambientScaleNumUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.ambientScaleNumUpDwn.Value = 1.0m;
            // 
            // diffuseScale
            // 
            this.diffuseScale.Controls.Add(this.diffuseScaleNumUpDwn);
            this.diffuseScale.Location = new System.Drawing.Point(8, 23);
            this.diffuseScale.Name = "diffuseScale";
            this.diffuseScale.Size = new System.Drawing.Size(170, 48);
            this.diffuseScale.TabIndex = 0;
            this.diffuseScale.TabStop = false;
            this.diffuseScale.Text = "Diffuse Scale";
            // 
            // diffuseScaleNumUpDwn
            // 
            this.diffuseScaleNumUpDwn.DecimalPlaces = 4;
            this.diffuseScaleNumUpDwn.Increment = this.dIncrement;
            this.diffuseScaleNumUpDwn.Maximum = this.dMaximum;
            this.diffuseScaleNumUpDwn.Minimum = this.dMinimum;
            this.diffuseScaleNumUpDwn.Location = new System.Drawing.Point(28, 19);
            this.diffuseScaleNumUpDwn.Name = "diffuseScaleNumUpDwn";
            this.diffuseScaleNumUpDwn.Size = new System.Drawing.Size(114, 20);
            this.diffuseScaleNumUpDwn.TabIndex = 0;
            this.diffuseScaleNumUpDwn.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.diffuseScaleNumUpDwn.Value = 1.0m;
            // 
            // fallbackPanel
            //
            //this.fbGlobalOptionsPanel.ResumeLayout(false);
            this.scaleSettingGBx.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.uScaleST0numUpDwn)).EndInit();
            this.scaleST0_GBx.ResumeLayout(false);
            this.scaleST0_GBx.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.vScaleST0numUpDwn)).EndInit();
            this.scaleST1_GBx.ResumeLayout(false);
            this.scaleST1_GBx.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.vScaleST1numUpDwn)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.uScaleST1numUpDwn)).EndInit();
            this.lightingSettingGBx.ResumeLayout(false);
            this.ambientScaleGBx.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.ambientScaleNumUpDwn)).EndInit();
            this.diffuseScale.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.diffuseScaleNumUpDwn)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();   

        }

        public void setData(ArrayList data, Dictionary<string, string> tgaFilesDictionary)
        {
            resetFields();

            bool processFBoptions = false;

            foreach (object obj in data)
            {
                System.Windows.Forms.Application.DoEvents();

                string line = common.COH_IO.removeExtraSpaceBetweenWords(((string)obj).Replace("\t", " ")).Trim();

                line = line.Replace("//", "#");

                if (line.ToLower().StartsWith("Fallback".ToLower()))
                {
                    processFBoptions = true;
                }
                if (processFBoptions)
                {
                    if (line.ToLower().StartsWith(("ScaleST0 ").ToLower()))
                    {
                        setScaleSt(common.COH_IO.fixCamelCase(line, "ScaleST0").Replace("ScaleST0 ", "").Trim(), "0", scaleST0_GBx);
                    }
                    else if (line.ToLower().StartsWith(("ScaleST1 ").ToLower()))
                    {
                        setScaleSt(common.COH_IO.fixCamelCase(line, "ScaleST1").Replace("ScaleST1 ", "").Trim(), "1", scaleST1_GBx);
                    }
                    else if (line.ToLower().StartsWith(("DiffuseScale ").ToLower()))
                    {
                        setNum(common.COH_IO.fixCamelCase(line, "DiffuseScale").Replace("DiffuseScale ", "").Trim(), diffuseScaleNumUpDwn);
                    }
                    else if (line.ToLower().StartsWith(("AmbientScale ").ToLower()))
                    {
                        setNum(common.COH_IO.fixCamelCase(line, "AmbientScale").Replace("AmbientScale ", "").Trim(), ambientScaleNumUpDwn);
                    }
                }
            }
        }

        private void setScaleSt(string line, string sType, GroupBox clrNexpGbx)
        {
            decimal u = 0, v = 0;

            bool uIntParseSuccess = false;

            bool vIntParseSuccess = false;

            string[] lineSplit = line.Split('#');

            string lineWOcomment = lineSplit[0].Trim();

            string[] colorStr = lineWOcomment.Split(' ');

            if (colorStr.Length == 2)
            {
                uIntParseSuccess = Decimal.TryParse(colorStr[0].Trim(), out u);

                vIntParseSuccess = Decimal.TryParse(colorStr[1].Trim(), out v);
            }

            if (uIntParseSuccess &&
               vIntParseSuccess)
            {
                Control[] ctl = clrNexpGbx.Controls.Find(
                    "uScaleST" + sType + "numUpDwn", false);

                ((NumericUpDown)ctl[0]).Value = u;

                ctl = clrNexpGbx.Controls.Find("vScaleST" + sType + "numUpDwn", false);

                ((NumericUpDown)ctl[0]).Value = v;
            }
        }

        private void setNum(string line, NumericUpDown numUpDwn)
        {
            string[] lineSplit = line.Split('#');

            string lineWOcomment = lineSplit[0].Trim();

            decimal dN = 0;

            if (Decimal.TryParse(lineWOcomment, out dN))
            {
                numUpDwn.Value = dN;
            }
        }
        private void resetFields()
        {
            uScaleST0numUpDwn.Value = 1; 

            vScaleST0numUpDwn.Value = 1;

            uScaleST1numUpDwn.Value = 1; 

            vScaleST1numUpDwn.Value = 1;

            diffuseScaleNumUpDwn.Value = 1;

            ambientScaleNumUpDwn.Value = 1;
        }
        public void getData(ref ArrayList data)
        {
            data.Add(string.Format("\t\tScaleST0 {0} {1}", uScaleST0numUpDwn.Value, vScaleST0numUpDwn.Value)); 
            
            data.Add(string.Format("\t\tScaleST1 {0} {1}", uScaleST1numUpDwn.Value, vScaleST1numUpDwn.Value));
           
            data.Add(string.Format("\t\tDiffuseScale {0}", diffuseScaleNumUpDwn.Value));
            
            data.Add(string.Format("\t\tAmbientScale {0}", ambientScaleNumUpDwn.Value));
        }

    }
}
