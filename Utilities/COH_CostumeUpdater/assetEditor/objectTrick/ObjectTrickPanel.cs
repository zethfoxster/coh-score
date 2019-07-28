using System;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.assetEditor.objectTrick
{
    public partial class ObjectTrickPanel : Panel
       //Form
    {
        private ArrayList ckbxList;
        private ArrayList autolods;
        private string gameBranch;
        private aeCommon.COH_ColorPicker colorPicker;

        public ObjectTrickPanel(string game_branch)
        {
            InitializeComponent();
            autolods = new ArrayList();
            gameBranch = game_branch;
            this.colorPicker = new aeCommon.COH_ColorPicker();
            buildCkBxList();
            ObjectTricksToolTips.intilizeToolTips(ckbxList, @"assetEditor/objectTrick/ObjectTricksToolTips.html");
            reset();
        }
        
        public Control findControl(string text)
        {
            Control [] ctls = this.Controls.Find(text, true);
            return ctls[0];
        }

        public void getData(ref ArrayList data)
        {
            getObjFlags(ref data);
            getTrickFlags(ref data);
            getGroupFlags(ref data);
            getGeneralOptions(ref data);
            getLODs(ref data);
        }

        private void getLODs(ref ArrayList data)
        {
            if (autolods.Count > 0)
            {
                foreach (AutoLODs autoLOD in autolods)
                {
                    data.Add("\tAutolod");

                    if(autoLOD.AllowedError != -555555)
                        data.Add(string.Format("\tAllowedError  {0}", autoLOD.AllowedError));

                    if (autoLOD.LodFar != -555555)
                        data.Add(string.Format("\tLodFar  {0}", autoLOD.LodFar));

                    if (autoLOD.LodFarFade != -555555)
                        data.Add(string.Format("\tLodFarFade  {0}", autoLOD.LodFarFade));

                    if (autoLOD.LodNear != -555555)
                        data.Add(string.Format("\tLodNear  {0}", autoLOD.LodNear));

                    if (autoLOD.LodNearFade != -555555)
                        data.Add(string.Format("\tLodNearFade  {0}", autoLOD.LodNearFade));

                    data.Add("\tEnd");

                    data.Add("");
                }
            }
            else
            {
                if (this.Autolod_ckbx.Checked)
                {
                    data.Add("\tAutolod");

                    getCtrlData("AllowedError", this.AllowedError_ckbx, new NumericUpDown[] { this.AllowedError_spinBx }, ref data);
                }

                getCtrlData("LodFar", this.LODFar_ckbx, new NumericUpDown[] { this.LOD_Far_SpnBx }, ref data);
                getCtrlData("LodFarFade", this.LODFarFade_ckbx, new NumericUpDown[] { this.Fade_Far_SpnBx }, ref data);
                getCtrlData("LodNear", this.LODNear_ckbx, new NumericUpDown[] { this.LOD_Near_SpnBx }, ref data);
                getCtrlData("LodNearFade", this.LODNearFade_ckbx, new NumericUpDown[] { this.Fade_Near_SpnBx }, ref data);

                if (this.Autolod_ckbx.Checked)
                {
                    data.Add("\tEnd");
                }
            }
        }

        private void getObjFlags(ref ArrayList data)
        {
            string flags = getFlags(this.objFlags_frame);
            string legacyObjFlags = getFlags(this.legacyObjFlags_frame);

            if (flags.Length + legacyObjFlags.Length > 0)
            {
                string objFlagStr = "ObjFlags " + flags.Trim();
                data.Add(string.Format("\t{0} {1}", objFlagStr, legacyObjFlags.Trim()));
            }
 
        }

        private void getTrickFlags(ref ArrayList data)
        {
            string flags = getFlags(this.trickFlags_frame);
            string legacyTrickFlags = getFlags(legacyTrickFlags_frame);

            if (flags.Length + legacyTrickFlags.Length > 0)
            {
                string trickFlagStr = "TrickFlags " + flags.Trim();
                data.Add(string.Format("\t{0} {1}", trickFlagStr, legacyTrickFlags.Trim()));
            }
        }

        private void getGroupFlags(ref ArrayList data)
        {
            string flags = getFlags(this.groupFlags_frame);
            if (flags.Length > 0)
            {
                data.Add(string.Format("\tGroupFlags {0}", flags.Trim()));
            }
        }

        private string getFlags(Control frameControl)
        {
            string flags = "";

            ArrayList ctls = new ArrayList();

            ctls.Clear();

            getDescendants(frameControl, ref ctls);

            foreach (Control ctl in ctls)
            {
                if (ctl.GetType() == typeof(CheckBox))
                {
                    CheckBox ckbx = (CheckBox)ctl;
                    if (ckbx.Checked)
                        flags += ctl.Text + " ";
                }
            }

            return flags;
        }
        
        private void getDescendants(Control ctl, ref ArrayList ctls)
        {            
            foreach (Control c in ctl.Controls)
            {
                ctls.Add(c);
                if (c.Controls.Count > 0)
                    getDescendants(c, ref ctls);
            }          
        }

        private void getGeneralOptions(ref ArrayList data)
        {
            getSway(ref data);
            getRotate(ref data);
            getLight(ref data);
            getAlphaRef_ObjTexBias_SortBias(ref data);
            getTextureSTScroll(ref data);
            getTintColor(ref data, 0);
            getTintColor(ref data, 1);

        }

        private void getTintColor(ref ArrayList data, int tintColorBit)
        {
            
            NumericUpDown rSpnBx = this.TintColor0_Red_spnBx;
            NumericUpDown gSpnBx = this.TintColor0_Green_spnBx;
            NumericUpDown bSpnBx = this.TintColor0_Blue_spnBx;
            CheckBox clrCkbx = this.TintColor0_ckbx;
            string tintClrName = string.Format("TintColor{0} ", tintColorBit);

            if (tintColorBit == 1)
            {
                rSpnBx = this.TintColor1_Red_spnBx;
                gSpnBx = this.TintColor1_Green_spnBx;
                bSpnBx = this.TintColor1_Blue_spnBx;
                clrCkbx = this.TintColor0_ckbx;
            }

            decimal r = rSpnBx.Value;
            decimal g = gSpnBx.Value;
            decimal b = bSpnBx.Value;

            bool hasScrollST = clrCkbx.Checked;

            if (hasScrollST)
            {
                data.Add(string.Format("\t{0} {1} {2} {3}", tintClrName, r, g, b));
            }
        }

        private void getSway(ref ArrayList data)
        {
            getCtrlData("Sway", this.Sway_ckbx, new NumericUpDown[] { this.SwaySpeed_SpnBx, this.SwayAngle_SpnBx }, ref data);
            getCtrlData("SwayPitch", this.SwayPitch_ckbx, new NumericUpDown[] { this.SwayPitch_Speed_SpnBx, this.SwayPitch_Angle_SpnBx}, ref data);
            getCtrlData("SwayRoll", this.SwayRoll_ckbx, new NumericUpDown[] { this.SwayRoll_Speed_SpnBx, this.SwayRoll_Angle_SpnBx}, ref data);
            getCtrlData("SwayRandomize", this.SwayRandomize_ckbx, new NumericUpDown[] { this.SwayRandomize_Speed_SpnBx, this.SwayRandomize_Angle_SpnBx}, ref data);
        }

        private void getRotate(ref ArrayList data)
        {
            getCtrlData("Rotate", this.Rotate_ckbx, new NumericUpDown[] { this.RotateSpeed_SpnBx }, ref data);
            getCtrlData("RotatePitch", this.RotatePitch_ckbx, new NumericUpDown[] { this.RotatePitch_Speed_SpnBx, this.RotatePitch_Angle_SpnBx }, ref data);
            getCtrlData("RotateRoll", this.RotateRoll_ckbx, new NumericUpDown[] { this.RotateRoll_Speed_SpnBx, this.RotateRoll_Angle_SpnBx }, ref data);
            getCtrlData("RotateRandomize", this.RotateRandomize_ckbx, new NumericUpDown[] { this.RotateRandomize_Speed_SpnBx }, ref data);
        }

        private void getLight(ref ArrayList data)
        {
            if (this.CastShadow_ckbx.Checked)
            {
                data.Add("CastShadow 1");
            }

            getCtrlData("NightGlow", this.NightGlow_ckbx, new NumericUpDown[] { this.NightGlow_Start_SpnBx, this.NightGlow_End_SpnBx }, ref data);
        }

        private void getAlphaRef_ObjTexBias_SortBias(ref ArrayList data)
        {
            getCtrlData(this.AlphaRef_ckbx.Text, this.AlphaRef_ckbx, new NumericUpDown[] { this.AlphaRef_Cutoff_SpnBx }, ref data);
            getCtrlData(this.ObjTexBias_ckbx.Text, this.ObjTexBias_ckbx, new NumericUpDown[] { this.ObjTexBais_Amount_SpnBx }, ref data);
            getCtrlData(this.SortBias_ckbx.Text, this.SortBias_ckbx, new NumericUpDown[] { this.SortBias_Amount_spnBx }, ref data);
        }

        private void getCtrlData(string key, CheckBox ckbx, NumericUpDown[] spnBxs, ref ArrayList data)
        {
            string spnBxsVal = "";

            if (spnBxs != null)
            {
                foreach (NumericUpDown n in spnBxs)
                {
                    decimal spinBx = n.Value;
                    spnBxsVal += string.Format("{0:0.00000} ", spinBx);
                }
            }

            string str = string.Format("\t{0}  {1}", key, spnBxsVal);

            bool hasProp = ckbx.Checked;

            if (hasProp)
            {
                data.Add(str);
            }
        }

        private void getTextureSTScroll(ref ArrayList data)
        {
            getScrollState(ref data, 0);
            getScrollState(ref data, 1);
            getSTAnim(ref data);
        }

        private void getScrollState(ref ArrayList data, int texBit)
        {
            string stName = string.Format("ScrollST{0}", texBit);
            decimal s = this.ScrollST0_S_SpnBx.Value;
            decimal t = this.ScrollST0_T_SpnBx.Value;
            bool hasScrollST = this.ScrollST0_ckbx.Checked;

            if(texBit == 1)
                hasScrollST = this.ScrollST1_ckbx.Checked;

            if (hasScrollST)
            {
                data.Add(string.Format("\t{0} {1:0.00000} {2:0.00000}", stName, s, t));
            }
        }

        private void getSTAnim(ref ArrayList data)
        {
            string animName = this.StAnim_ComboBx.Text;

            decimal speed = this.StAnim_Speed_SpnBx.Value;

            decimal scale = this.StAnim_Scale_SpnBx.Value;

            string interpolationType = getCheckedRadioBtn();

            string stAnimStr = string.Format("\tStAnim {0:0.00000} {1:0.00000}", speed, scale);

            if (!animName.Equals("Select Animation Name"))
            {
                stAnimStr = string.Format("{0} {1}", stAnimStr, animName);
            }

            if (interpolationType.Length > 0)
            {
                stAnimStr = string.Format("{0} {1}", stAnimStr, interpolationType);
            }

            bool hasSTAnim = this.StAnim_ckbx.Checked;

            if (hasSTAnim)
            {
                data.Add(stAnimStr);
            }
        }

        private string getCheckedRadioBtn()
        {
            if (Framesnap_radioBtn.Checked)
            {
                return "FRAMESNAP";
            }
            else if (PingPong_radioBtn.Checked)
            {
                return "PINGPONG";
            }
            return "";
        }

        public void setData(ArrayList data)
        {
            setCkBx(data);
        }

        private void setCkBx(ArrayList data)
        {
            reset();

            int autoLodInd = -1;

            autolods.Clear();

            foreach (object obj in data)
            {
                string line = common.COH_IO.removeExtraSpaceBetweenWords(((string)obj).Replace("\t", " ")).Trim();

                line = line.Replace("//", "#");

                if (line.ToLower().StartsWith(("ObjFlags ").ToLower()))
                {
                    string[] fStr = getFields(line, "ObjFlags ");
                    updateControls(fStr);
                    
                }
                else if (line.ToLower().StartsWith(("TrickFlags ").ToLower()))
                {
                    string[] fStr = getFields(line, "TrickFlags ");
                    updateControls(fStr);
                }
                else if (line.ToLower().StartsWith(("GroupFlags ").ToLower()))
                {
                    string[] fStr = getFields(line, "GroupFlags ");
                    updateControls(fStr);
                }
                //Sway Section
                else if (line.ToLower().StartsWith(("Sway ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "Sway ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("Sway");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                else if (line.ToLower().StartsWith(("SwayRoll ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "SwayRoll ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("SwayRoll");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                else if (line.ToLower().StartsWith(("SwayPitch ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "SwayPitch ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("SwayPitch");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                else if (line.ToLower().StartsWith(("SwayRandomize ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "SwayRandomize ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("SwayRandomize");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //Rotate Section
                else if (line.ToLower().StartsWith(("Rotate ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "Rotate ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("Rotate");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                else if (line.ToLower().StartsWith(("RotateRoll ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "RotateRoll ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("RotateRoll");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                else if (line.ToLower().StartsWith(("RotatePitch ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "RotatePitch ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("RotatePitch");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                else if (line.ToLower().StartsWith(("RotateRandomize ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "RotateRandomize ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("RotateRandomize");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //AlphaRef Section
                else if (line.ToLower().StartsWith(("AlphaRef ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "AlphaRef ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("AlphaRef");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //ObjTexBais Section
                else if (line.ToLower().StartsWith(("ObjTexBias ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "ObjTexBias ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("ObjTexBias");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //CastShadow Section Need more info...
                else if (line.ToLower().StartsWith(("CastShadow ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "CastShadow ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("CastShadow");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //NightGlow Section
                else if (line.ToLower().StartsWith(("NightGlow ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "NightGlow ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("NightGlow");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //Autolod legacy support
                else if (line.ToLower().StartsWith(("Autolod").ToLower()))
                {
                    string[] spinBoxData = new string [] {"0"};

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("Autolod");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                    
                    AutoLODs autoLOD = new AutoLODs();

                    autoLOD.AllowedError = -555555;

                    autoLOD.LodFar = -555555;

                    autoLOD.LodFarFade = -555555;

                    autoLOD.LodNear = -555555;

                    autoLOD.LodNearFade = -555555;

                    autolods.Add(autoLOD);

                    autoLodInd = autolods.Count - 1;                    
                }
                //AllowedError Section
                else if (line.ToLower().StartsWith(("AllowedError ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "AllowedError ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("AllowedError");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);

                    if (autolods.Count > 0 )
                    {
                        AutoLODs autoLOD = (AutoLODs)autolods[autoLodInd];

                        autoLOD.AllowedError = this.AllowedError_spinBx.Value;
                    }
                }
                //LODFar Distance Section
                else if (line.ToLower().StartsWith(("LodFar ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "LodFar ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("LodFar");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);

                    if (autolods.Count > 0)
                    {
                        AutoLODs autoLOD = (AutoLODs)autolods[autoLodInd];

                        autoLOD.LodFar = this.LOD_Far_SpnBx.Value;
                    }
                }
                //LODNear Distance Section
                else if (line.ToLower().StartsWith(("LodNear ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "LodNear ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("LodNear");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);

                    if (autolods.Count > 0)
                    {
                        AutoLODs autoLOD = (AutoLODs)autolods[autoLodInd];

                        autoLOD.LodNear = this.LOD_Near_SpnBx.Value;
                    }
                }
                //LODFarFade Section
                else if (line.ToLower().StartsWith(("LodFarFade ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "LodFarFade ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("LodFarFade");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);

                    if (autolods.Count > 0)
                    {
                        AutoLODs autoLOD = (AutoLODs)autolods[autoLodInd];

                        autoLOD.LodFarFade = this.Fade_Far_SpnBx.Value;
                    }
                }
                //LODNearFade Section
                else if (line.ToLower().StartsWith(("LodNearFade ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "LodNearFade ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("LodNearFade");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);

                    if (autolods.Count > 0)
                    {
                        AutoLODs autoLOD = (AutoLODs)autolods[autoLodInd];

                        autoLOD.LodNearFade = this.Fade_Near_SpnBx.Value;
                    }
                }
                //ScrollST0 Section
                else if (line.ToLower().StartsWith(("ScrollST0 ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "ScrollST0 ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("ScrollST0");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //ScrollST1 Section
                else if (line.ToLower().StartsWith(("ScrollST1 ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "ScrollST1 ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("ScrollST1");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //StAnim Section
                else if (line.ToLower().StartsWith(("StAnim ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "StAnim ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("StAnim");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //TintColor0 Section
                else if (line.ToLower().StartsWith(("TintColor0 ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "TintColor0 ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("TintColor0");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //TintColor1 Section
                else if (line.ToLower().StartsWith(("TintColor1 ").ToLower()))
                {
                    string[] spinBoxData = getFields(line, "TintColor1 ");

                    CheckBox_SpinCombo ckbxSpnCombo = getControl("TintColor1");

                    ckbxSpnCombo.updateSpinBoxes(spinBoxData);
                }
                //SortBias Section
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
        }

        private string[] getFields(string line, string key)
        {
            string subStr = line.Substring(0, key.Length - 1);

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

        private void TintColor_spnBx_ValueChanged(object sender, System.EventArgs e)
        {
            string clrSpnBxName = ((NumericUpDown)sender).Name;

            NumericUpDown rSpnBx = null, gSpnBx = null, bSpnBx = null;

            Button clrBtn = null;

            if (clrSpnBxName.StartsWith("TintColor1_"))
            {
                rSpnBx = this.TintColor1_Red_spnBx;

                gSpnBx = this.TintColor1_Green_spnBx;

                bSpnBx = this.TintColor1_Blue_spnBx;

                clrBtn = this.TintColor1_clrBtn;
            }
            else
            {
                rSpnBx = this.TintColor0_Red_spnBx;

                gSpnBx = this.TintColor0_Green_spnBx;

                bSpnBx = this.TintColor0_Blue_spnBx;

                clrBtn = this.TintColor0_clrBtn;
            }
            
            int r = (int)Math.Min(255, Math.Max(rSpnBx.Value, 0));

            int g = (int)Math.Min(255, Math.Max(gSpnBx.Value, 0));

            int b = (int)Math.Min(255, Math.Max(bSpnBx.Value, 0));

            Color c = Color.FromArgb(255, r, g, b);

            if (!c.Equals(clrBtn.BackColor))
            {
                clrBtn.BackColor = c;
            }

        }

        private void TintColor1_clr_btn_Click(object sender, System.EventArgs e)
        {
            colorPicker.setColor(TintColor1_clrBtn.BackColor);

            Color c = colorPicker.getColor();

            if (!c.Equals(TintColor1_clrBtn.BackColor))
            {
                TintColor1_clrBtn.BackColor = c;

                this.TintColor1_Red_spnBx.Value = c.R;

                this.TintColor1_Green_spnBx.Value = c.G;

                this.TintColor1_Blue_spnBx.Value = c.B;
            }
        }

        private void TintColor0_clrBtn_Click(object sender, System.EventArgs e)
        {
            colorPicker.setColor(TintColor0_clrBtn.BackColor);

            Color c = colorPicker.getColor();

            if (!c.Equals(TintColor0_clrBtn.BackColor))
            {
                TintColor0_clrBtn.BackColor = c;

                this.TintColor0_Red_spnBx.Value = c.R;

                this.TintColor0_Green_spnBx.Value = c.G;

                this.TintColor0_Blue_spnBx.Value = c.B;
            }
        }

        private void buildCkBxList()
        {
            string[] animList = System.IO.Directory.GetFiles(@"C:\" + gameBranch + @"\data\player_library\animations\tscroll", "*.anim");

            foreach (string str in animList)
            {                
                StAnim_ComboBx.Items.Add(System.IO.Path.GetFileNameWithoutExtension(str));
            }
            
            ckbxList = new ArrayList();

            ckbxList.Add(new CheckBox_SpinCombo(this.Additive_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.AlphaCutout_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.AlphaRef_ckbx, new NumericUpDown[] { this.AlphaRef_Cutoff_SpnBx }, "AlphaRef"));
            ckbxList.Add(new CheckBox_SpinCombo(this.BaseEditVisible_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.CameraFace_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.CastShadow_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.DontCastShadowMap_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.DontReceiveShadowMap_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.DoorVolume_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.DoubleSided_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.EditorVisible_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.FancyWater_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.FancyWaterOffOnly_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.ForceAlphaSort_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.ForceOpaque_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.FrontFace_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.FullBright_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.KeyLight_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.LavaVolume_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.LightFace_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.LightmapsOffOnly_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.Rotate_ckbx, new NumericUpDown[] { this.RotateSpeed_SpnBx }, "Rotate"));
            ckbxList.Add(new CheckBox_SpinCombo(this.RotateRoll_ckbx, new NumericUpDown[] { this.RotateRoll_Speed_SpnBx, this.RotateRoll_Angle_SpnBx }, "RotateRoll"));
            ckbxList.Add(new CheckBox_SpinCombo(this.RotatePitch_ckbx, new NumericUpDown[] { this.RotatePitch_Speed_SpnBx, this.RotatePitch_Angle_SpnBx }, "RotatePitch"));
            ckbxList.Add(new CheckBox_SpinCombo(this.RotateRandomize_ckbx, new NumericUpDown[] { this.RotateRandomize_Speed_SpnBx }, "RotateRandomize"));
            ckbxList.Add(new CheckBox_SpinCombo(this.NightLight_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.NightGlow_ckbx, new NumericUpDown[] { this.NightGlow_Start_SpnBx, this.NightGlow_End_SpnBx }, "NightGlow"));
            ckbxList.Add(new CheckBox_SpinCombo(this.NoColl_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.NoDraw_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.NoFog_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.NoLightAngle_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.NotSelectable_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.NoZTest_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.NoZWrite_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.ParentFade_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.ObjTexBias_ckbx, new NumericUpDown[] { this.ObjTexBais_Amount_SpnBx }, "ObjTexBias"));
            ckbxList.Add(new CheckBox_SpinCombo(this.ReflectTex0_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.ReflectTex1_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.RegionMarker_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.Autolod_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.AllowedError_ckbx, new NumericUpDown[] { this.AllowedError_spinBx }, "AllowedError"));
            ckbxList.Add(new CheckBox_SpinCombo(this.LODFar_ckbx, new NumericUpDown[] { this.LOD_Far_SpnBx }, "LodFar"));
            ckbxList.Add(new CheckBox_SpinCombo(this.LODNear_ckbx, new NumericUpDown[] { this.LOD_Near_SpnBx }, "LodNear"));
            ckbxList.Add(new CheckBox_SpinCombo(this.LODFarFade_ckbx, new NumericUpDown[] { this.Fade_Far_SpnBx }, "LodFarFade"));
            ckbxList.Add(new CheckBox_SpinCombo(this.LODNearFade_ckbx, new NumericUpDown[] { this.Fade_Near_SpnBx }, "LodNearFade"));
            ckbxList.Add(new CheckBox_SpinCombo(this.SelectOnly_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.SimpleAlphaSort_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.SoundOccluder_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.SortBias_ckbx, new NumericUpDown[] { this.SortBias_Amount_spnBx }, "SortBias"));
            ckbxList.Add(new CheckBox_SpinCombo(this.StaticFx_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.Sway_ckbx, new NumericUpDown[] { this.SwaySpeed_SpnBx, this.SwayAngle_SpnBx }, "Sway"));
            ckbxList.Add(new CheckBox_SpinCombo(this.SwayRoll_ckbx, new NumericUpDown[] { this.SwayRoll_Speed_SpnBx, this.SwayRoll_Angle_SpnBx }, "SwayRoll"));
            ckbxList.Add(new CheckBox_SpinCombo(this.SwayPitch_ckbx, new NumericUpDown[] { this.SwayPitch_Speed_SpnBx, this.SwayPitch_Angle_SpnBx }, "SwayPitch"));
            ckbxList.Add(new CheckBox_SpinCombo(this.SwayRandomize_ckbx, new NumericUpDown[] { this.SwayRandomize_Speed_SpnBx, this.SwayRandomize_Angle_SpnBx }, "SwayRandomize"));
            ckbxList.Add(new CheckBox_SpinCombo(this.Subtractive_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.TreeDraw_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.VisBlocker_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.VisOutside_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.VisOutsideOnly_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.VisShell_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.VisTray_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.VisWindow_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.VolumeTrigger_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.WaterVolume_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.WorldFx_ckbx, null, ""));
            ckbxList.Add(new CheckBox_SpinCombo(this.ScrollST0_ckbx,
                new NumericUpDown[] { this.ScrollST0_S_SpnBx, this.ScrollST0_T_SpnBx },"ScrollST0"));
            ckbxList.Add(new CheckBox_SpinCombo(this.ScrollST1_ckbx,
                new NumericUpDown[] { this.ScrollST1_S_SpnBx, this.ScrollST1_T_SpnBx },"ScrollST1"));
            ckbxList.Add(new CheckBox_SpinCombo(this.StAnim_ckbx,
                new NumericUpDown[] { this.StAnim_Speed_SpnBx, this.StAnim_Scale_SpnBx }, this.StAnim_ComboBx, 
                new RadioButton[] {this.Loop_radioBtn, this.PingPong_radioBtn, this.Framesnap_radioBtn},"StAnim"));
            ckbxList.Add(new CheckBox_SpinCombo(this.TintColor1_ckbx, 
                new NumericUpDown[] { this.TintColor1_Red_spnBx, TintColor1_Green_spnBx, TintColor1_Blue_spnBx }, "TintColor1"));
            ckbxList.Add(new CheckBox_SpinCombo(this.TintColor0_ckbx, 
                new NumericUpDown[] { this.TintColor0_Red_spnBx, TintColor0_Green_spnBx, TintColor0_Blue_spnBx }, "TintColor0"));
                             
        }
             
    }

    public class AutoLODs
    {
        public decimal AllowedError;
	    public decimal LodFarFade;
	    public decimal LodNearFade;
	    public decimal LodNear;
        public decimal LodFar;

        public AutoLODs()
        {
        }
    }
}
