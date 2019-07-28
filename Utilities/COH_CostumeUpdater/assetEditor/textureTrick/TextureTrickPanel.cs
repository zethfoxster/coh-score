using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using COH_CostumeUpdater.assetEditor.objectTrick;

namespace COH_CostumeUpdater.assetEditor.textureTrick
{
    public partial class TextureTrickPanel : Panel
       //Form
    {
        private ArrayList ckbxList;
        private System.Collections.Generic.Dictionary<string, string> tgaFilesDictionary;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private bool isFolderTTrick;

        public TextureTrickPanel(OpenFileDialog ofd, Dictionary<string, string> tFDictionary)
        {
            isFolderTTrick = false;

            InitializeComponent();

            buildCkBxList();

            this.tgaFilesDictionary = tFDictionary;

            this.openFileDialog1 = ofd;

            setToolTips();

            reset();
        }

        private void setToolTips()
        {
            ArrayList tmpCkbxList = (ArrayList)ckbxList.Clone();

            tmpCkbxList.Add(new CheckBox_SpinCombo(BumpMap_ckbx, null, ""));

            tmpCkbxList.Add(new CheckBox_SpinCombo(Blend_ckbx, null, ""));

            COH_CostumeUpdater.assetEditor.objectTrick.ObjectTricksToolTips.intilizeToolTips(tmpCkbxList, @"assetEditor/objectTrick/ObjectTricksToolTips.html");
        }

        public string getTextureTrickTgaPath()
        {
            return this.tgaFileNameTxBx.Text;
        }

        void TextureTrickPanel_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (e.KeyData == Keys.Enter )
            {
                TextBox tb = (TextBox)sender;
                string tbName = tb.Name;
                switch (tb.Name)
                {
                    case "Blend_txBx":
                        tbName = "Blend";
                        break;
                    case "BumpMap_txBx":
                        tbName = "BumpMap";
                        break;
                }
                updateTextureBox(tbName, tb.Text.Trim());
            }
        }

        private void updateTextureBox(string textureBxName, string valStr)
        {
            string[] tgafileNames = tgaFilesDictionary.Values.ToArray();
            string[] tgafilePaths = tgaFilesDictionary.Keys.ToArray();
            string textureName = valStr.ToLower().EndsWith(".tga") ? valStr : valStr + ".tga";
            string tgaFile = common.COH_IO.findTGAPathFast(textureName, tgafilePaths, tgafileNames);
            CheckBox ckbx = null;
            PictureBox pBx = null;
            TextBox tBx = null;
            

            if (textureBxName.ToLower().Equals("Blend".ToLower()))
            {
                ckbx = this.Blend_ckbx;
                pBx = this.BlendType_Icon_pxBx;
                tBx = this.Blend_txBx;
            }
            else if (textureBxName.ToLower().Equals("BumpMap".ToLower()))
            {
                ckbx = this.BumpMap_ckbx;
                pBx = this.BumpMap_Icon_pxBx;
                tBx = this.BumpMap_txBx;
            }
            else
            {
                pBx = this.tgaIconPicBx;
                tBx = this.tgaFileNameTxBx;
            }

            if (tgaFile != null)
            {

                if (ckbx != null)
                {
                    ckbx.Checked = true;

                    this.ckbx_Click(ckbx, new EventArgs());
                }

                tBx.Text = tgaFile;

                openFileDialog1.FileName = tgaFile;

                setIconImage(tgaFile, pBx);

                if (System.IO.File.Exists(tgaFile))
                    openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(tgaFile);
            }
            else if(!textureName.Contains("/"))
            {
                tBx.Text = textureName;
                pBx.Image = COH_CostumeUpdater.Properties.Resources.imgNotFound;
            }
        }
       
        public Control findControl(string text)
        {
            Control [] ctls = this.Controls.Find(text, true);
            return ctls[0];
        }

        private void tgaIconPicBx_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            try
            {
                string path = ((TextBox)((PictureBox)sender).Tag).Text;

                if(path!= null && path.Length > 1)
                    viewTga(path, true);
            }
            catch (Exception ex)
            {
            }
        }
        private void menuItem1_Click(object sender, System.EventArgs e)
        {
            string path = this.tgaFileNameTxBx.Text;
            if (System.IO.File.Exists(path))
            {
                System.Diagnostics.Process.Start(path);
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

        void ckbx_Click(object sender, System.EventArgs e)
        {
            CheckBox ckbx = (CheckBox)sender;
            foreach (Control ctl in ckbx.Parent.Controls)
            {
                ctl.Enabled = ckbx.Checked;
                if (ctl.GetType() == typeof(PictureBox))
                    ctl.Visible = ckbx.Checked; 
            }
            ckbx.Enabled = true;
        }

        private void fileOpenBtn_Click(object sender, EventArgs e)
        {
            string oldFilter = this.openFileDialog1.Filter;
            string oldInitDir = this.openFileDialog1.InitialDirectory;

            this.openFileDialog1.Filter = "TGA files (*.tga)|*.tga";
            
            this.openFileDialog1.InitialDirectory = "C:\\game\\src\\Texture_Library";

            string path = ((TextBox)((Button)sender).Tag).Text;

            if (path != null && path.Length > 1 && System.IO.File.Exists(path))
            {
                openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(path);
            }

            DialogResult dr = openFileDialog1.ShowDialog();

            if (dr == DialogResult.OK)
            {
                ((TextBox)((Button)sender).Tag).Text = openFileDialog1.FileName;
                PictureBox pBx = (PictureBox)((TextBox)((Button)sender).Tag).Tag;               
                setIconImage(openFileDialog1.FileName, pBx);
            }

            this.openFileDialog1.Filter = oldFilter;
            this.openFileDialog1.InitialDirectory = oldInitDir;

        }

        private void setIconImage(string path, PictureBox pBx)
        {
            try
            {
                PresentParameters pp = new PresentParameters();
                pp.Windowed = true;
                pp.SwapEffect = SwapEffect.Copy;
                Device device = new Device(0, DeviceType.Hardware, this.tgaIconPicBx, CreateFlags.HardwareVertexProcessing, pp);
                Microsoft.DirectX.Direct3D.Texture tx = TextureLoader.FromFile(device, path);
                Microsoft.DirectX.GraphicsStream gs = TextureLoader.SaveToStream(ImageFileFormat.Png, tx);

                Bitmap btmap = new Bitmap(gs);

                pBx.Image = btmap;

                gs.Dispose();
                tx.Dispose();
                device.Dispose();
            }
            catch (Exception ex)
            { }
        }

        public void getData(ref ArrayList data)
        {
            getFlags(ref data, textureTrickFlags_gpBx);
            getBumpMap(ref data);
            getBlend(ref data);
            getFade(ref data);
            getScaleSTState(ref data, 0);
            getScaleSTState(ref data, 1);
            getSurface(ref data);
            getSortBias(ref data);
        }

        private void getBumpMap(ref ArrayList data)
        {
            string bMap = this.BumpMap_txBx.Text;
            if (BumpMap_ckbx.Checked && bMap.Length > 1)
            {
                bMap = System.IO.Path.GetFileName(bMap);
                string dataStr = string.Format("\tBumpMap {0}", bMap.Trim());
                data.Add(dataStr);
            } 
        }
        
        private void getBlend(ref ArrayList data)
        {
            string blend = this.Blend_txBx.Text;
            if (Blend_ckbx.Checked && blend.Length > 1)
            {
                blend = System.IO.Path.GetFileName(blend);
                string dataStr = string.Format("\tBlend {0}", blend.Trim());
                data.Add(dataStr);
                getBlendType(ref data);
            }
        }
        
        private void getBlendType(ref ArrayList data)
        {
            string blend = this.BlendType_ComboBox.Text;
            string dataStr = string.Format("\tBlendType {0}", blend.Trim());
            data.Add(dataStr);
        }

        private void getFlags(ref ArrayList data, Control frameControl)
        {
            string flags = "";

            foreach (CheckBox_SpinCombo cbsc in ckbxList)
            {
                if (cbsc.comboBoxControl == null &&
                    cbsc.radioButtonControls == null &&
                    cbsc.spinBoxControls == null)
                {
                    CheckBox ckbx = (CheckBox)cbsc.checkBoxControl;
                    if (ckbx.Checked)
                        flags += ckbx.Text + " ";
                }
            }
            if (flags.Length > 1)
            {
                string dataStr = string.Format("\tFlags {0}", flags);
                data.Add(dataStr);
            }
        }

        private void getSortBias(ref ArrayList data)
        {
            if (this.SortBias_ckbx.Checked)
            {
                decimal spinBx = this.SortBias_spnBx.Value;
                string val = string.Format("{0:0.00000}", spinBx);
                string dataStr = string.Format("\tSortBias {0}", val);
                data.Add(dataStr);
            }            
        }

        private void getFade(ref ArrayList data)
        {
            if (this.Fade_ckbx.Checked)
            {
                decimal nearFade = this.near_spnBx.Value;
                decimal farFade = this.far_spnBx.Value;
                string val = string.Format("{0:0.00000} {1:0.00000} ", nearFade, farFade);
                string dataStr = string.Format("\tFade {0}", val);
                data.Add(dataStr);
            }
        }

        private void getSurface(ref ArrayList data)
        {
            string surface = this.Surface_comboBox.Text;
            if (Surface_ckbx.Checked && !surface.ToLower().Equals("-Type-"))
            {
                string dataStr = string.Format("\tSurface {0}", surface.Trim());
                data.Add(dataStr);
            }
        }

        private void getScaleSTState(ref ArrayList data, int texBit)
        {
            string stName = string.Format("ScaleST{0}", texBit);

            decimal u = this.uScaleST0_spnBx.Value;
            decimal v = this.vScaleST0_spnBx.Value;
            bool hasScaleST = this.ScaleST0_ckbx.Checked;

            if(texBit==1)
            {
                u = this.uScaleST1_spnBx.Value;
                v = this.vScaleST1_spnBx.Value;
                hasScaleST = this.ScaleST1_ckbx.Checked;
            }

            if (hasScaleST)
            {
                data.Add(string.Format("\t{0} {1:0.00000} {2:0.00000}", stName, u, v));
            }
        }

        public void setData(ArrayList data)
        {
            setCkBx(data);
        }

        private void setCkBx(ArrayList data)
        {
            reset();

            foreach (object obj in data)
            {
                string line = common.COH_IO.removeExtraSpaceBetweenWords(((string)obj).Replace("\t", " ")).Trim();

                line = line.Replace("//", "#");

                if (line.ToLower().StartsWith(("Texture ").ToLower()))
                {
                    string[] tgaStr = getFields(line, "Texture ");

                    updateTextureBox("", tgaStr[0]);

                    isFolderTTrick = tgaStr[0].Contains("/");

                    BumpMap_grpBx.Enabled = !isFolderTTrick;

                    Blend_grpBx.Enabled = !isFolderTTrick;

                    scaleSetting_grpBx.Enabled = !isFolderTTrick;
                }
                else if (line.ToLower().StartsWith(("Flags ").ToLower()))
                {
                  string[] fStr = getFields(line, "Flags ");
                  updateControls(fStr);
                    
                }
                else if (line.ToLower().StartsWith(("Surface ").ToLower()))
                {
                    string[] comboBxData = {"0","0",""};

                    string[] cBxData = getFields(line, "Surface ");

                    comboBxData[2] = cBxData[0];

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("Surface");

                    ckbxSpnCombo.updateSpinBoxes(comboBxData);
                }
                else if (line.ToLower().StartsWith(("BlendType ").ToLower()))
                {
                    string[] cBxData = getFields(line, "BlendType ");

                    this.BlendType_ComboBox.Text = cBxData[0];
                }
                else if (line.ToLower().StartsWith(("Blend ").ToLower()))
                {
                    string[] tgaStr = getFields(line, "Blend ");

                    updateTextureBox("Blend", tgaStr[0]);
                }
                else if (line.ToLower().StartsWith(("BumpMap ").ToLower()))
                {
                    string[] tgaStr = getFields(line, "BumpMap ");

                    updateTextureBox("BumpMap", tgaStr[0]);
                }
                else if (line.ToLower().StartsWith(("ScaleST0 ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "ScaleST0 ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("ScaleST0");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }

                else if (line.ToLower().StartsWith(("ScaleST1 ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "ScaleST1 ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("ScaleST1");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                else if (line.ToLower().StartsWith(("Fade ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "Fade ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("Fade");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                else if (line.ToLower().StartsWith(("SortBias ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "SortBias ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("SortBias");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
             }
        }
        
        private void reset()
        {
            foreach (CheckBox_SpinCombo ctl in ckbxList)
            {
                ctl.enableControl(false);
            }
            
            this.tgaFileNameTxBx.Text = "";
            this.tgaIconPicBx.Image = null;
            this.BumpMap_txBx.Text = "";
            this.BumpMap_Icon_pxBx.Image = null;
            this.BumpMap_ckbx.Checked = false;
            ckbx_Click(BumpMap_ckbx, new EventArgs());            
            this.Blend_txBx.Text = "";
            this.BlendType_Icon_pxBx.Image = null;
            this.Blend_ckbx.Checked = false;
            ckbx_Click(Blend_ckbx, new EventArgs());
        }

        
        private string[] getFields(string line, string key)
        {
            string subStr = line.Substring(0, key.Length -1);

            string[] fieldsStr = common.COH_IO.fixCamelCase(line, subStr.Trim()).Replace(subStr, "").Trim().Split(' ');

            return fieldsStr;
        }

        private void updateControls(string[] fieldsStr)
        {
            foreach(string fStr in fieldsStr)
            {
                CheckBox_SpinCombo ckbxCombo = (CheckBox_SpinCombo) getControl(fStr);

                if (ckbxCombo != null)
                {
                    ckbxCombo.enableControl(true);
                }
            }
        }

        private CheckBox_SpinCombo getControl(string fStr)
        {
            foreach (CheckBox_SpinCombo ctl in ckbxList)
            {
                if(ctl.controlName.ToLower().Equals(fStr.ToLower()))
                    return ctl;
            }
            return null;
        }

        private void buildCkBxList()
        {
            ckbxList = new ArrayList();

            ckbxList.Add(new CheckBox_SpinCombo(TrueColor_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(MirrorT_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(MirrorS_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(RepeatT_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(ClampT_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(ClampS_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(RepeatS_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(NoMip_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(FullBright_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(Jpeg_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(IsBumpMap_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(SurfaceIcy_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(SurfaceSlick_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(NoColl_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(SurfaceBouncy_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(PointSample_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(Replaceable_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(Border_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(NoDither_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(NoRandomAddGlow_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(FallbackForceOpaque_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(OldTint_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(AlwaysAddGlow_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(Surface_ckbx, null, Surface_comboBox, null, "Surface"));

            ckbxList.Add(new CheckBox_SpinCombo(SortBias_ckbx, new NumericUpDown[] { SortBias_spnBx }, "SortBias"));
            ckbxList.Add(new CheckBox_SpinCombo(Fade_ckbx, new NumericUpDown[] { near_spnBx, far_spnBx }, "Fade"));
            ckbxList.Add(new CheckBox_SpinCombo(ScaleST0_ckbx, new NumericUpDown[] { uScaleST0_spnBx, vScaleST0_spnBx }, "ScaleST0"));
            ckbxList.Add(new CheckBox_SpinCombo(ScaleST1_ckbx, new NumericUpDown[] { uScaleST1_spnBx, vScaleST1_spnBx }, "ScaleST1"));
           }
             
    }                    
}
