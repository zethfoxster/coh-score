using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.assetEditor.aeCommon
{
    public partial class MatSpecLitCubeMapTintFlags : /*Form//*/Panel
    {
        public MatSpecLitCubeMapTintFlags(OpenFileDialog oFileD)
        {
            isDirty = false;

            this.dIncrement = new decimal(new int[] { 5, 0, 0, 65536 });
            
            this.dMaximum = new decimal(new int[] { 10000000, 0, 0, 0 });
            
            this.dMinimum = new decimal(new int[] { 10000000, 0, 0, -2147483648 });

            InitializeComponent();
            
            ImageList imgList = new ImageList();
            
            imgList.Images.Add(COH_CostumeUpdater.Properties.Resources.legacyFlag);
            
            this.legacyFlagsList.SmallImageList = imgList;
            
            this.legacyFlagsList.LargeImageList = imgList;
            
            this.openFileDialog1 = oFileD;

            this.colorPicker = new COH_ColorPicker();
        }

        private void clr1_Num_ValueChanged(object sender, EventArgs e)
        {
            int r = (int) Math.Min(255,Math.Max(this.clr1Rnum.Value,0));
            
            int g = (int)Math.Min(255, Math.Max(this.clr1Gnum.Value, 0));
            
            int b = (int)Math.Min(255, Math.Max(this.clr1Bnum.Value, 0));
            
            Color c = Color.FromArgb(255, r,g,b);

            if (!c.Equals(clr1Button.BackColor))
            {
                isDirty = true;
             
                clr1Button.BackColor = c;
            }
        }

        private void clr2_Num_ValueChanged(object sender, EventArgs e)
        {
            int r = (int)Math.Min(255, Math.Max(this.clr2Rnum.Value, 0));
            
            int g = (int)Math.Min(255, Math.Max(this.clr2Gnum.Value, 0));
            
            int b = (int)Math.Min(255, Math.Max(this.clr2Bnum.Value, 0));
            
            Color c = Color.FromArgb(255, r, g, b);

            if (!c.Equals(clr2Button.BackColor))
            {
                isDirty = true;

                clr2Button.BackColor = c;
            }
        }

        private void clr3_Num_ValueChanged(object sender, EventArgs e)
        {
            int r = (int)Math.Min(255, Math.Max(this.clr3Rnum.Value, 0));
            
            int g = (int)Math.Min(255, Math.Max(this.clr3Gnum.Value, 0));
            
            int b = (int)Math.Min(255, Math.Max(this.clr3Bnum.Value, 0));
            
            Color c = Color.FromArgb(255, r, g, b);

            if (!c.Equals(clr3Button.BackColor))
            {
                isDirty = true;

                clr3Button.BackColor = c;
            }
        }

        private void clr4_Num_ValueChanged(object sender, EventArgs e)
        {
            int r = (int)Math.Min(255, Math.Max(this.clr4Rnum.Value, 0));
            
            int g = (int)Math.Min(255, Math.Max(this.clr4Gnum.Value, 0));
            
            int b = (int)Math.Min(255, Math.Max(this.clr4Bnum.Value, 0));
            
            Color c = Color.FromArgb(255, r, g, b);

            if (!c.Equals(clr4Button.BackColor))
            {
                isDirty = true;

                clr4Button.BackColor = c;
            }
        }

        private void color1Button_Click(object sender, EventArgs e)
        {
            colorPicker.setColor(clr1Button.BackColor);
            
            Color c = colorPicker.getColor();

            if (!c.Equals(clr1Button.BackColor))
            {
                isDirty = true;

                clr1Button.BackColor = c;

                this.clr1Rnum.Value = c.R;

                this.clr1Gnum.Value = c.G;

                this.clr1Bnum.Value = c.B;
            }
        }

        private void color2Button_Click(object sender, EventArgs e)
        {
            colorPicker.setColor(clr2Button.BackColor);
            
            Color c = colorPicker.getColor();

            if (!c.Equals(clr2Button.BackColor))
            {
                isDirty = true;

                clr2Button.BackColor = c;

                this.clr2Rnum.Value = c.R;

                this.clr2Gnum.Value = c.G;

                this.clr2Bnum.Value = c.B;
            }
        }
       
        private void color3Button_Click(object sender, EventArgs e)
        {
            colorPicker.setColor(clr3Button.BackColor);
            
            Color c = colorPicker.getColor();

            if (!c.Equals(clr3Button.BackColor))
            {
                isDirty = true;

                clr3Button.BackColor = c;

                this.clr3Rnum.Value = c.R;

                this.clr3Gnum.Value = c.G;

                this.clr3Bnum.Value = c.B;
            }
        }

        private void color4Button_Click(object sender, EventArgs e)
        {
            colorPicker.setColor(clr4Button.BackColor);
            
            Color c = colorPicker.getColor();

            if (!c.Equals(clr4Button.BackColor))
            {
                isDirty = true;

                clr4Button.BackColor = c;

                this.clr4Rnum.Value = c.R;

                this.clr4Gnum.Value = c.G;

                this.clr4Bnum.Value = c.B;
            }           
        }
        
        void TextureCKBx_CheckedChanged(object sender, System.EventArgs e)
        {

            enableCubemaps(TextureCKBx.Checked, TextureCKBx.Checked);

            isDirty = true;
        }
        
        private void textureButton_Click(object sender, EventArgs e)
        {
            string cubemapsPath = @"C:\game\src\Texture_Library\static_cubemaps";

            string OldPath = openFileDialog1.InitialDirectory;

            if (!OldPath.ToLower().Contains("static_cubemaps"))
                openFileDialog1.InitialDirectory = cubemapsPath;

            DialogResult dr =  openFileDialog1.ShowDialog();

            if (dr == DialogResult.OK)
            {
                this.textureTXBx.Text = openFileDialog1.FileName;
                openFileDialog1.InitialDirectory = this.textureTXBx.Text;
                isDirty = true;
            }
        }

        void enableCubemaps(bool enable, bool textureEnable)
        {            
            fileBrowseBtn.Enabled = textureEnable;
            textureTXBx.Enabled = textureEnable;
            ReflectionDesaturateCkBx.Enabled = enable;
            reflectionTintCkBx.Enabled = enable;
            //below could exists w/o cubemap
            powerGBx.Enabled = enable;
            ScaleGBx.Enabled = enable;
            baseGBx.Enabled = enable;
            powerNum.Enabled = enable;
            scaleNum.Enabled = enable;
            BaseNum.Enabled = enable;
        }
        void dynamicChecKBx_Click(object sender, System.EventArgs e)
        {
            if (dynamicCKBx.Checked)
            {
                TextureCKBx.Checked = false;
                enableCubemaps(true, false);
            }
            if (!dynamicCKBx.Checked && !TextureCKBx.Checked)
                enableCubemaps(false, false);

            isDirty = true;
        }

        void checKBx_Click(object sender, System.EventArgs e)
        {

            if (TextureCKBx.Checked)
            {
                dynamicCKBx.Checked = false;
            }
            if (!dynamicCKBx.Checked && !TextureCKBx.Checked)
                enableCubemaps(false,false);

            isDirty = true;
        }

        private void resetFields()
        {
            Color c = Color.FromArgb(255,255,255,255);
            Color c34 = Color.FromArgb(255, 0, 0, 0);

            clr1Button.BackColor = c;
            clr1Rnum.Value = c.R;
            clr1Bnum.Value = c.B;
            clr1Gnum.Value = c.G;
            clr1ExponentNum.Value = 1;

            clr2Button.BackColor = c;
            clr2Rnum.Value = c.R;
            clr2Bnum.Value = c.B;
            clr2Gnum.Value = c.G;
            clr2ExponentNum.Value = 1;

            clr3Button.BackColor = c34;
            clr3Rnum.Value = c34.R; ;
            clr3Bnum.Value = c34.B; ;
            clr3Gnum.Value = c34.G; ;

            clr4Button.BackColor = c;
            clr4Rnum.Value = c34.R; ;
            clr4Bnum.Value = c34.B; ;
            clr4Gnum.Value = c34.G; ;

            BaseNum.Value = 1;
            powerNum.Value = 1;
            scaleNum.Value = 1;

            textureTXBx.Text = "";
            TextureCKBx.Checked = false;
            dynamicCKBx.Checked = false;
            ReflectionDesaturateCkBx.Checked = false;
            reflectionTintCkBx.Checked = false;

            enableCubemaps(false,false);

            diffuseScaleNum.Value = 1;

            addGlowMaxNum.Value = 1;
            addGlowMinNum.Value = 0;
            ambientScaleNum.Value = 1;

            noRandomAddGlow.Checked = false;
            alwaysAddGlow.Checked = false;
            oldTint.Checked = false;
            fullBright.Checked = false;
            fancyWater.Checked = false;
            AlphaWaterCkBx.Checked = false;
            FallbackForceOpaqueCkBx.Checked = false;

            legacyFlagsList.Items.Clear();

        }

        private string fixCamelCase(string line, string secName)
        {
            string fixedLine = "";

            char[] lineChars = line.ToCharArray();

            char[] secChars = secName.ToCharArray();

            if (line.ToLower().StartsWith((secName).ToLower()))
            {
                for (int i = 0; i < secName.Length; i++)
                {
                    char c = lineChars[i];

                    char ch = secChars[i];

                    {
                        if (ch.ToString().ToLower().Equals(c.ToString().ToLower()))
                        {
                            lineChars[i] = ch;
                        }
                    }
                }
            }

            foreach (char m in lineChars)
            {
                fixedLine += m;
            }

            return fixedLine.Trim();
        }

        public void setData(ArrayList data, string[] tgafilePaths, string[] tgafileNames)
        {
            bool hasReflectivityBase = false;
            bool hasReflectivityScale = false;
            bool hasReflectivityPower = false;
            bool hasTexture = false;
            bool hasSpecularExponent1 = false;
            bool hasSpecularExponent2 = false;


            resetFields();
            foreach (object obj in data)
            {
                string line = common.COH_IO.removeExtraSpaceBetweenWords(((string)obj).Replace("\t", " ")).Trim();

                line = line.Replace("//", "#");

                if (line.ToLower().StartsWith("Fallback".ToLower()))
                {
                    return;
                }
                if (line.ToLower().StartsWith("CubeMap ".ToLower()))
                {
                    line = fixCamelCase(line, "CubeMap");

                    setTexture(line.Replace("CubeMap ", "").Trim(), tgafilePaths, tgafileNames);

                    hasTexture = true;
                }
                if (line.ToLower().StartsWith("ReflectivityBase ".ToLower()))
                {
                    line = fixCamelCase(line, "ReflectivityBase");

                    setNum(line.Replace("ReflectivityBase ", ""), BaseNum);

                    hasReflectivityBase = true;
                }
                if (line.ToLower().StartsWith("ReflectivityScale ".ToLower()))
                {
                    line = fixCamelCase(line, "ReflectivityScale");

                    setNum(line.Replace("ReflectivityScale ", ""), scaleNum);

                    hasReflectivityScale = true;
                }
                if (line.ToLower().StartsWith("ReflectivityPower ".ToLower()))
                {
                    line = fixCamelCase(line, "ReflectivityPower");

                    setNum(line.Replace("ReflectivityPower ", ""), powerNum);

                    hasReflectivityPower = true;
                }
                if (line.ToLower().StartsWith("SpecularColor1 ".ToLower()))
                {
                    line = fixCamelCase(line, "SpecularColor1");

                    setColor(line.Replace("SpecularColor1 ", ""), "clr1", colorNExponent1);
                }
                else if (line.ToLower().StartsWith("SpecularColor2 ".ToLower()))
                {
                    line = fixCamelCase(line, "SpecularColor2");

                    setColor(line.Replace("SpecularColor2 ", ""), "clr2", colorNExponent2);
                }
                else if (line.ToLower().StartsWith("SpecularExponent1 ".ToLower()))
                {
                    line = fixCamelCase(line, "SpecularExponent1");

                    setNum(line.Replace("SpecularExponent1 ", ""), clr1ExponentNum);

                    hasSpecularExponent1 = true;
                }
                else if (line.ToLower().StartsWith("SpecularExponent2 ".ToLower()))
                {
                    line = fixCamelCase(line, "SpecularExponent2");

                    setNum(line.Replace("SpecularExponent2 ", ""), clr2ExponentNum);

                    hasSpecularExponent2 = true;
                }
                else if (line.ToLower().StartsWith("DiffuseScale ".ToLower()))
                {
                    line = fixCamelCase(line, "DiffuseScale");

                    setNum(line.Replace("DiffuseScale ", ""), diffuseScaleNum);
                }
                else if (line.ToLower().StartsWith("AmbientScale ".ToLower()))
                {
                    line = fixCamelCase(line, "AmbientScale");

                    setNum(line.Replace("AmbientScale ", ""), ambientScaleNum);
                }
                else if (line.ToLower().StartsWith("Flags ".ToLower()))
                {
                    line = fixCamelCase(line, "Flags");

                    setFlags(line.Replace("Flags ", ""));
                }
                else if (line.ToLower().StartsWith("ObjFlags ".ToLower()))
                {
                    line = fixCamelCase(line, "ObjFlags");

                    setFlags(line.Replace("ObjFlags ", ""));
                }
                else if (line.ToLower().StartsWith("MinAddGlow ".ToLower()))
                {
                    line = fixCamelCase(line, "MinAddGlow");

                    setNum(line.Replace("MinAddGlow ", ""), addGlowMinNum);
                }
                else if (line.ToLower().StartsWith("MaxAddGlow ".ToLower()))
                {
                    line = fixCamelCase(line, "MaxAddGlow");

                    setNum(line.Replace("MaxAddGlow ", ""), addGlowMaxNum);
                }                                           
                else if (line.ToLower().StartsWith("Color3 ".ToLower()))
                {
                    line = fixCamelCase(line, "Color3");

                    setColor(line.Replace("Color3 ", ""), "clr3", colorTint1);
                }
                else if (line.ToLower().StartsWith("Color4 ".ToLower()))
                {
                    line = fixCamelCase(line, "Color4");

                    setColor(line.Replace("Color4 ", ""), "clr4", colorTint2);
                }
                else if (line.ToLower().StartsWith("ReflectionDesaturate ".ToLower()))
                {
                    line = fixCamelCase(line, "ReflectionDesaturate");

                    string[] lineSplit = line.ToLower().Replace("ReflectionDesaturate ".ToLower(), "").Split('#');

                    string lineWOcomment = lineSplit[0].Trim();

                    if (lineWOcomment.StartsWith("1"))
                        ReflectionDesaturateCkBx.Checked = true;
                    else
                        ReflectionDesaturateCkBx.Checked = false;
                }
                else if (line.ToLower().StartsWith("reflectionTint ".ToLower()))
                {
                    line = fixCamelCase(line, "reflectionTint");

                    string[] lineSplit = line.ToLower().Replace("reflectionTint ".ToLower(), "").Split('#');

                    string lineWOcomment = lineSplit[0].Trim();

                    if(lineWOcomment.StartsWith("1"))
                        reflectionTintCkBx.Checked = true;
                    else
                        reflectionTintCkBx.Checked = false;
                }
                else if (line.ToLower().StartsWith("AlphaWater ".ToLower()))
                {
                    line = fixCamelCase(line, "AlphaWater");

                    string[] lineSplit = line.ToLower().Replace("AlphaWater ".ToLower(), "").Split('#');

                    string lineWOcomment = lineSplit[0].Trim();

                    if (lineWOcomment.StartsWith("1"))
                        AlphaWaterCkBx.Checked = true;
                    else
                        AlphaWaterCkBx.Checked = false;
                }
                if (!hasTexture &&
                    hasReflectivityBase &&
                    hasReflectivityPower &&
                    hasReflectivityScale)
                {
                    setTexture("generic_cubemap_face0.tga", tgafilePaths, tgafileNames);

                    if (textureTXBx.Text.Trim().Length == 0)
                        textureTXBx.Text = "generic_cubemap_face0.tga";

                    hasTexture = true;
                }
                if(!hasSpecularExponent1 &&
                    !hasSpecularExponent2 &&
                    line.ToLower().StartsWith("SpecularExponent ".ToLower()))
                {
                    line = fixCamelCase(line, "SpecularExponent");

                    setNum(line.Replace("SpecularExponent ", ""), clr1ExponentNum);

                    hasSpecularExponent1 = true;
              
                    setNum(line.Replace("SpecularExponent ", ""), clr2ExponentNum);

                    hasSpecularExponent2 = true;
                }
            }
        }

        private void setTexture(string line, string[] tgafilePaths, string[] tgafileNames)
        {
            int len = line.Length;

            if (line.Contains("#"))
            {
                len = line.IndexOf("#");

                string commentedTxName = line.Substring(len + "#".Length);

                commentedTxName = commentedTxName.ToLower().EndsWith(".tga") ? commentedTxName : commentedTxName + ".tga";

                if (System.IO.File.Exists(commentedTxName))
                {
                    textureTXBx.Text = commentedTxName;
                }
                else
                {
                    commentedTxName = System.IO.Path.GetFileName(commentedTxName);

                }

                textureTXBx.Text = common.COH_IO.findTGAPathFast(commentedTxName, tgafilePaths, tgafileNames);

                TextureCKBx_CheckedChanged(TextureCKBx, new EventArgs());  
            }

            string textureName = line.Substring(0, len).Trim();

            textureName = textureName.ToLower().EndsWith(".tga") ? textureName : textureName + ".tga";

            if (line.ToLower().Contains("none"))
            {
                TextureCKBx.Checked = false;

                TextureCKBx_CheckedChanged(TextureCKBx, new EventArgs());
            }
            else if (line.ToLower().Contains("%DYNAMIC%".ToLower()))
            {
                TextureCKBx.Checked = false;

                dynamicCKBx.Checked = true;

                TextureCKBx_CheckedChanged(TextureCKBx, new EventArgs());
            }
            else
            {
                TextureCKBx.Checked = true;

                textureTXBx.Text = common.COH_IO.findTGAPathFast(textureName, tgafilePaths, tgafileNames);

                TextureCKBx_CheckedChanged(TextureCKBx, new EventArgs());

            }

            if (dynamicCKBx.Checked)
                enableCubemaps(true, false);

            if (TextureCKBx.Checked)
                enableCubemaps(true,true);

            try
            {
                openFileDialog1.FileName = textureTXBx.Text;
                openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(textureTXBx.Text);

            }
            catch (Exception e)
            {
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

        private void setFlags(string line)
        {
            string[] lineSplit = line.Split('#');

            string lineWOcomment = lineSplit[0].Trim();

            string[] flags = lineWOcomment.Split(' ');

            foreach (string flag in flags)
            {
                if (fixCamelCase(flag, "none").Trim().ToLower().Equals("none"))
                {
                    return;
                }
                else if (fixCamelCase(flag, "NoRandomAddGlow").Trim().ToLower().Equals("NoRandomAddGlow".ToLower()))
                {
                    noRandomAddGlow.Checked = true;
                }
                else if (fixCamelCase(flag, "FullBright").Trim().ToLower().Equals("FullBright".ToLower()))
                {
                    fullBright.Checked = true;
                }
                else if (fixCamelCase(flag, "AlwaysAddGlow").Trim().ToLower().Equals("AlwaysAddGlow".ToLower()))
                {
                    alwaysAddGlow.Checked = true;
                }
                else if (fixCamelCase(flag, "OldTint").Trim().ToLower().Equals("OldTint".ToLower()))
                {                   
                    oldTint.Checked = true;
                }
                else if (fixCamelCase(flag, "FancyWater").Trim().ToLower().Equals("FancyWater".ToLower()))
                {
                    fancyWater.Checked = true;
                }
                else if (fixCamelCase(flag, "FallbackForceOpaque").Trim().ToLower().Equals("FallbackForceOpaque".ToLower()))
                {
                    FallbackForceOpaqueCkBx.Checked = true;
                }
                else
                {
                    addLegacyFlagData(flag.Trim());
                }
            }
        }
       
        private void setColor(string line, string clrP, GroupBox clrNexpGbx)
        {
            int r = 0, g = 0, b = 0;


            bool rIntParseSuccess = false;

            bool gIntParseSuccess = false;

            bool bIntParseSuccess = false;

            string[] lineSplit = line.Split('#');

            string lineWOcomment = lineSplit[0].Trim();

            string[] colorStr = lineWOcomment.Split(' ');

            if (colorStr.Length == 3)
            {
               rIntParseSuccess = Int32.TryParse(colorStr[0].Trim(), out r);

               gIntParseSuccess = Int32.TryParse(colorStr[1].Trim(), out g);

               bIntParseSuccess = Int32.TryParse(colorStr[2].Trim(), out b);
            }

            if (rIntParseSuccess &&
               gIntParseSuccess &&
               bIntParseSuccess)
            {
                Color c = Color.FromArgb(255, r, g, b);

                Control[] ctl = clrNexpGbx.Controls.Find(clrP + "Rnum", false);

                ((NumericUpDown)ctl[0]).Value = c.R;

                ctl = clrNexpGbx.Controls.Find(clrP + "Gnum", false);

                ((NumericUpDown)ctl[0]).Value = c.G;

                ctl = clrNexpGbx.Controls.Find(clrP + "Bnum", false);

                ((NumericUpDown)ctl[0]).Value = c.B;

                ctl = clrNexpGbx.Controls.Find(clrP + "Button", false);

                ((Button)ctl[0]).BackColor = c;
            }
            else
            {                
                //default color 3 and 4
                Color c = Color.FromArgb(255, 0, 0, 0);

                if (clrP.ToLower().Equals("clr1") ||
                    clrP.ToLower().Equals("clr2"))
                {
                    c = Color.FromArgb(255, 155, 155, 155);
                }
 

                Control[] ctl = clrNexpGbx.Controls.Find(clrP + "Rnum", false);

                ((NumericUpDown)ctl[0]).Value = c.R;

                ctl = clrNexpGbx.Controls.Find(clrP + "Gnum", false);

                ((NumericUpDown)ctl[0]).Value = c.G;

                ctl = clrNexpGbx.Controls.Find(clrP + "Bnum", false);

                ((NumericUpDown)ctl[0]).Value = c.B;

                ctl = clrNexpGbx.Controls.Find(clrP + "Button", false);

                ((Button)ctl[0]).BackColor = c;
            }
        }

        private string getFlags()
        {
            string flags = "";
            
            if(this.noRandomAddGlow.Checked)
                flags += "NoRandomAddGlow";

            flags = flags.Length > 0 ? string.Format("{0} ", flags) : flags;
            if(this.fullBright.Checked)
                flags += "FullBright";

            flags = flags.Length > 0 ? string.Format("{0} ", flags) : flags;
            if(this.alwaysAddGlow.Checked)
                flags += "AlwaysAddGlow";

            flags = flags.Length > 0 ? string.Format("{0} ", flags) : flags;
            if(this.oldTint.Checked)
                flags += "OldTint";

            flags = flags.Length > 0 ? string.Format("{0} ", flags) : flags;
            if (this.FallbackForceOpaqueCkBx.Checked)
                flags += "FallbackForceOpaque";

            if (this.legacyFlagsList.Items.Count > 0)
                flags +=  getLegacyFlagsData();

            if (flags == "")
                flags = "None ";

            return flags;
        }

        private string getLegacyFlagsData()
        {
            string legacy_flags = "";

            foreach (ListViewItem item in legacyFlagsList.Items)
            {
                legacy_flags = legacy_flags.Length > 0 ? string.Format("{0} ", legacy_flags) : legacy_flags;
                legacy_flags += (string)item.Text;
            }

            return legacy_flags;
        }

        private void addLegacyFlagData(string flag)
        {
            ListViewItem item = new ListViewItem(flag);
            item.Text = flag;
            item.Name = string.Format("legFlag_{0}",legacyFlagsList.Items.Count);
            item.ImageIndex = 0;
            legacyFlagsList.Items.Add(item);
        }

        private void getCubeMapData(ref ArrayList data)
        {
            string img = System.IO.Path.GetFileNameWithoutExtension(textureTXBx.Text);

            string dStr = "";

            if (TextureCKBx.Checked || dynamicCKBx.Checked)
            {
                if (TextureCKBx.Checked)
                {
                    if(textureTXBx.Text.Trim().Length > 0)
                        dStr = string.Format("\tCubeMap {0} //{1}", img, textureTXBx.Text);
                    else
                        dStr = "\tCubeMap generic_cubemap_face0.tga";

                    data.Add(dStr);
                }
                else if (dynamicCKBx.Checked)
                {
                    dStr = string.Format("\tCubeMap %DYNAMIC% //{0}", textureTXBx.Text);
                    data.Add(dStr);
                }

                dStr = string.Format("\tReflectivityBase {0}", BaseNum.Value);
                data.Add(dStr);
                dStr = string.Format("\tReflectivityScale {0}", scaleNum.Value);
                data.Add(dStr);
                dStr = string.Format("\tReflectivityPower {0}", powerNum.Value);
                data.Add(dStr);
            }
            else
            {
                dStr = string.Format("\t// CubeMap {0} //{1}", img, textureTXBx.Text);
                data.Add(dStr);
                dStr = string.Format("\t// ReflectivityBase {0}", BaseNum.Value);
                data.Add(dStr);
                dStr = string.Format("\t// ReflectivityScale {0}", scaleNum.Value);
                data.Add(dStr);
                dStr = string.Format("\t// ReflectivityPower {0}", powerNum.Value);
                data.Add(dStr);
            }
            if (reflectionTintCkBx.Checked)
            {
                dStr = "\treflectionTint 1";
                data.Add(dStr);
            }
            else
            {
                dStr = "\treflectionTint 0";
                data.Add(dStr);
            }
            if (ReflectionDesaturateCkBx.Checked)
            {
                dStr = "\tReflectionDesaturate 1";
                data.Add(dStr);
            }
            else
            {
                dStr = "\tReflectionDesaturate 0";
                data.Add(dStr);
            }

            data.Add("");           
        }

        public void getData(ref ArrayList data)
        {
            getCubeMapData(ref data);

            string dStr = string.Format("\tSpecularColor1 {0} {1} {2}", this.clr1Rnum.Value, this.clr1Gnum.Value, this.clr1Bnum.Value);
	        data.Add(dStr);
            dStr = string.Format("\tSpecularColor2 {0} {1} {2} // BLACK means inherit color from SpecularColor1",
                                 this.clr2Rnum.Value, this.clr2Gnum.Value, this.clr2Bnum.Value);
            data.Add(dStr);
	        dStr = string.Format("\tSpecularExponent1 {0}", this.clr1ExponentNum.Value);
            data.Add(dStr);
	        dStr = string.Format("\tSpecularExponent2 {0}", this.clr2ExponentNum.Value);
            data.Add(dStr);
            data.Add("");   

	        dStr = string.Format("\tDiffuseScale {0}", this.diffuseScaleNum.Value);
            data.Add(dStr);
	        dStr = string.Format("\tAmbientScale {0}", this.ambientScaleNum.Value);
            data.Add(dStr);
            data.Add("");

            string flags = getFlags();
	        dStr = string.Format("\tFlags {0} // NoRandomAddGlow FullBright AlwaysAddGlow OldTint",flags );
            data.Add(dStr);

            if (this.fancyWater.Checked)
                data.Add("\tObjFlags FancyWater");

            if (this.AlphaWaterCkBx.Checked)
                data.Add("\tAlphaWater 1");
            else
                data.Add("\tAlphaWater 0");

	        dStr = string.Format("\tMinAddGlow {0}", this.addGlowMinNum.Value);
            data.Add(dStr);
	        dStr = string.Format("\tMaxAddGlow {0}", this.addGlowMaxNum.Value);
            data.Add(dStr);
            data.Add("");

	        dStr = string.Format("\tColor3 {0} {1} {2} // Color tint for DualColor2, black inherits from colors 1 and 2",
                                 this.clr3Rnum.Value, this.clr3Gnum.Value, this.clr3Bnum.Value);
            data.Add(dStr);

	        dStr = string.Format("\tColor4 {0} {1} {2}",this.clr4Rnum.Value, this.clr4Gnum.Value, this.clr4Bnum.Value);
            data.Add(dStr);

            data.Add("");
        }

    }
}
