using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    public partial class ParticalsWin: Panel
        //Form
    {
        private int width;
        
        private string rootPath;
        
        public string fileName;
        
        private ArrayList data;
        
        public BhvrNPartCommonFunctionPanel bpcfPanel;
        
        public event EventHandler PartComboBoxChanged;
        
        public ArrayList partPanels;

        public event EventHandler IsDirtyChanged;

        private bool standAloneWindow;
        
        public bool isDirty;
        
        public bool isLocked;
        
        public Partical Tag;

        public common.FormatedToolTip fToolTip;

        private Dictionary<string, string> tgaFilesDictionary;

        //loadStandAlonePartFile(string file_name)

        public ParticalsWin(string root_path, ref Dictionary<string, string> tgaDic, ImageList imgList)
        {
            width = 580;
            initPartWin(root_path, ref tgaDic, imgList);
            standAloneWindow = false;
        }

        public ParticalsWin(string root_path, string partFileName, ref Dictionary<string, string> tgaDic)
        {
            width = 580;
            standAloneWindow = true;
            ImageList imgList = new ImageList();
            imgList.ImageSize = new Size(16, 8);
            imgList.Images.Add(Properties.Resources.ImgGreenGreen);
            imgList.Images.Add(Properties.Resources.ImgRedGreen);
            imgList.Images.Add(Properties.Resources.ImgRedRed);
            imgList.Images.Add(Properties.Resources.ImgRedBlack);

            initPartWin(root_path, ref tgaDic, imgList);

            loadStandAlonePartFile(partFileName);

            fixPanelLayout();
        }

        private void initPartWin(string root_path, ref Dictionary<string, string> tgaDic, ImageList imgList)
        {
            rootPath = root_path;

            isLocked = true;

            isDirty = false;

            partPanels = new ArrayList();

            InitializeComponent();

            Tag = null;

            bpcfPanel = new BhvrNPartCommonFunctionPanel(this, "Part", imgList);

            bpcfPanel.PartOrBhvrComboBoxChanged -= new EventHandler(bpcfPanel_partOrBhvrComboBoxChanged);
            bpcfPanel.PartOrBhvrComboBoxChanged += new EventHandler(bpcfPanel_partOrBhvrComboBoxChanged);

            buildKeyPanel();

            this.Controls.Add(bpcfPanel);

            bpcfPanel.Dock = DockStyle.Top;

            if (tgaDic != null)
            {
                tgaFilesDictionary = tgaDic.ToDictionary(k => k.Key, k => k.Value);
            }
            else
            {
                MessageBox.Show("Building TGA texture Dictionary...\r\nThis may take few seconds!");

                tgaFilesDictionary = common.COH_IO.getFilesDictionary(rootPath.Replace(@"\data", "") + @"src\Texture_Library", "*.tga");

                tgaDic = tgaFilesDictionary.ToDictionary(k => k.Key, k => k.Value);
            }

            ArrayList ctlList = buildToolTipsObjList();

            fToolTip = common.COH_htmlToolTips.intilizeToolTips(ctlList, @"assetEditor/objectTrick/PartToolTips.html");
        }

        public void addToTextureDic(string tgaFileName)
        {
            if (!tgaFilesDictionary.ContainsKey(tgaFileName))
            {
                tgaFilesDictionary[tgaFileName] = System.IO.Path.GetFileName(tgaFileName);
                tgaFilesDictionary = common.COH_IO.sortDictionaryKeys(tgaFilesDictionary);
            }
        }

        public string getTexturePath(string tgaFileName)
        {
            string[] tgafileNames = tgaFilesDictionary.Values.ToArray();

            string[] tgafilePaths = tgaFilesDictionary.Keys.ToArray();

            return common.COH_IO.findTGAPathFast(tgaFileName, tgafilePaths, tgafileNames);
        }

        private ArrayList buildToolTipsObjList()
        {
            ArrayList ctlList = new ArrayList();
            
            foreach (TabPage tp in part_tabControl.Controls)
            {
                foreach (ParticalKeyPanel pkeyPanel in tp.Controls)
                {
                    string pKey = pkeyPanel.Name.Replace("_ParticalKeyPanel", "");
                    ctlList.Add(new common.COH_ToolTipObject(pkeyPanel, pKey));
                }
            }

            return ctlList;
        }

        private void bpcfPanel_partOrBhvrComboBoxChanged(object sender, EventArgs e)
        {
            if (PartComboBoxChanged != null)
                PartComboBoxChanged(this, e);
        }

        private void buildKeyPanel()
        {
            string gameBranch = rootPath.Substring(@"C:\".Length).Replace(@"\data\", "");
            
            ParticalKeys partKeys = new ParticalKeys(gameBranch);

            int y = 6;

            int prevCtrlHeight = 0;

            Color ltColor = SystemColors.ControlLight;

            Color dkColor = SystemColors.Control;

            foreach(ParticalKey bk in partKeys.keysList)
            {
                bool isFlag = bk.numFields == 0;

                ParticalKeyPanel keyPanel = new ParticalKeyPanel(this,
                                                                 bk.name, 
                                                                 bk.fieldsLabel, 
                                                                 bk.numFields, 
                                                                 bk.min, 
                                                                 bk.max, 
                                                                 bk.defaultVal,
                                                                 bk.numDecimalPlaces,
                                                                 bk.isColor,
                                                                 isFlag, 
                                                                 bk.comboItems, 
                                                                 width,
                                                                 bk.isCkbx,
                                                                 rootPath,
                                                                 bk.group,
                                                                 bk.subGroup);

                keyPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top
                                    | System.Windows.Forms.AnchorStyles.Left)
                                    | System.Windows.Forms.AnchorStyles.Right)));



                switch (bk.group)
                {
                    case "Emitter":
                        this.partEmitter_tabPage.Controls.Add(keyPanel);
                        y = 2 + ((keyPanel.Height + 2) * (this.partEmitter_tabPage.Controls.Count-1));
                        keyPanel.mainBackColor = this.partEmitter_tabPage.Controls.Count % 2 == 0 ? ltColor : dkColor;
                        break;

                    case "Motion":
                        this.partMotion_tabPage.Controls.Add(keyPanel);
                        y = 2 + ((keyPanel.Height + 2) * (this.partMotion_tabPage.Controls.Count-1));
                        keyPanel.mainBackColor = this.partMotion_tabPage.Controls.Count % 2 == 0 ? ltColor : dkColor;
                        break;

                    case "Particle":
                        this.partPartical_tabPage.Controls.Add(keyPanel);
                        y = 2 + ((keyPanel.Height + 2) * (this.partPartical_tabPage.Controls.Count-1));
                        keyPanel.mainBackColor = this.partPartical_tabPage.Controls.Count % 2 == 0 ? ltColor : dkColor;
                        break;

                    case "Texture/Color":
                        if (this.partTextureColor_tabPage.Controls.Count == 0)
                            y = 0;

                        if (bk.numFields == 0 &&
                            !bk.name.ToLower().StartsWith("texture") &&
                            bk.comboItems == null)
                            keyPanel.showCommentCheckBx();

                        this.partTextureColor_tabPage.Controls.Add(keyPanel);
                        y += 2 + ((prevCtrlHeight));
                        prevCtrlHeight = keyPanel.Height;
                        keyPanel.mainBackColor = this.partTextureColor_tabPage.Controls.Count % 2 == 0 ? ltColor : dkColor;                        
                        break;
                }

                this.partPanels.Add(keyPanel);

                keyPanel.Location = new System.Drawing.Point(0, y);

                keyPanel.CheckChanged -= new EventHandler(keyPanel_CheckChanged);
                keyPanel.CheckChanged += new EventHandler(keyPanel_CheckChanged);

                keyPanel.IsDirtyChanged -= new EventHandler(keyPanel_IsDirtyChanged);
                keyPanel.IsDirtyChanged += new EventHandler(keyPanel_IsDirtyChanged);

                keyPanel.enable(false);
            }
        }
       
        public void fixPanelLayout()
        {
            int y = 6;
            int prevCtrlHeight = 0;

            int eTabCount = 0;
            int mTabCount = 0;
            int pTabCount = 0;
            int tTabCount = 0;

            Color ltColor = SystemColors.ControlLight;
            Color dkColor = SystemColors.Control;

            foreach (TabPage tb in this.part_tabControl.Controls)
            {
                tb.VerticalScroll.Value = 0;
            }

            foreach (ParticalKeyPanel keyPanel in partPanels)
            {
                if(keyPanel.Name.Contains("IgnoreFxTint")||keyPanel.Name.Contains("AlwaysDraw"))
                    keyPanel.Used = true;                   

                if (keyPanel.Used ||  standAloneWindow)
                {
                    string group = keyPanel.Parent.Text;
                    keyPanel.Visible = true;
                    switch (group)
                    {

                        case "Emitter":
                            y = 2 + ((keyPanel.Height + 2) * (eTabCount++));
                            keyPanel.mainBackColor = eTabCount % 2 == 0 ? ltColor : dkColor;
                            break;

                        case "Motion":
                            y = 2 + ((keyPanel.Height + 2) * (mTabCount++));
                            keyPanel.mainBackColor = mTabCount % 2 == 0 ? ltColor : dkColor;
                            break;

                        case "Particle":
                            y = 2 + ((keyPanel.Height + 2) * (pTabCount++));
                            keyPanel.mainBackColor = pTabCount % 2 == 0 ? ltColor : dkColor;
                            break;

                        case "Shake/Blur/Light":
                            y = 2 + ((keyPanel.Height + 2) * (tTabCount++));
                            keyPanel.mainBackColor = tTabCount % 2 == 0 ? ltColor : dkColor;
                            break;

                        case "Texture/Color":
                            if (tTabCount == 0)
                                y = 0;

                            y += 2 + ((prevCtrlHeight));
                            prevCtrlHeight = keyPanel.Height;
                            keyPanel.mainBackColor = ++tTabCount % 2 == 0 ? ltColor : dkColor;
                            break;
                    }

                    keyPanel.Location = new System.Drawing.Point(0, y);

                    keyPanel.toggleCheckBX();
                    keyPanel.toggleCheckBX();
                }
                else
                {
                    keyPanel.Visible = false;
                }
            }
        }
        
        public void initializeComboBx(ArrayList evList, bool setText)
        {
            bpcfPanel.initializeComboBxs(evList);

            if (setText)
            {
                if (bpcfPanel.eventsComboBx.Items.Count > 0)
                    bpcfPanel.eventsComboBx.Text = (string)bpcfPanel.eventsComboBx.Items[0];

                if (bpcfPanel.eventsBhvrFilesMComboBx.Items.Count > 0)
                {
                    bpcfPanel.eventsBhvrFilesMComboBx.Text = (string)((common.MComboBoxItem)bpcfPanel.eventsBhvrFilesMComboBx.Items[0]).Text;
                }
            }
        }

        public ArrayList getData()
        {
            ArrayList data = new ArrayList();

            data.Add("System");

            data.Add("");

            foreach (TabPage tp in part_tabControl.Controls)
            {
                getTabData(ref data, tp, true);
            }

            data.Add("End");

            return data;
        }

        private string processFlags(string[] flagsData)
        {
            string flags = null;

            bool IgnoreFxTintCommented = false;
            bool AlwaysDrawCommented = false;

            bool IgnoreFxTint = false;

            bool AlwaysDraw = false;

            if (flagsData[0] != null)
            {
                if (flagsData[0].Contains("#"))
                    IgnoreFxTintCommented = true;
                else
                    IgnoreFxTint = true;

            }

            if (flagsData[1] != null)
            {
                if (flagsData[1].Contains("#"))
                    AlwaysDrawCommented = true;
                else
                    AlwaysDraw = true;
            }

            if (IgnoreFxTintCommented && AlwaysDrawCommented)
                flags = "\t#Flags IgnoreFxTint AlwaysDraw";

            else if (IgnoreFxTint && AlwaysDraw)
                flags = "\tFlags IgnoreFxTint AlwaysDraw";

            else if (IgnoreFxTint && AlwaysDrawCommented)
                flags = "\tFlags IgnoreFxTint #AlwaysDraw";

            else if (IgnoreFxTintCommented && AlwaysDraw)
                flags = "\tFlags AlwaysDraw #IgnoreFxTint";

            return flags;
        }

        private void getTabData(ref ArrayList data, TabPage tp, bool addTabHeader)
        {
            if (addTabHeader)
            {
                data.Add("");
                data.Add("#".PadRight(59,'#'));
                data.Add(string.Format("### {0} ", tp.Text).PadRight(59, '#'));
                data.Add("#".PadRight(59,'#'));
                data.Add("");
            }

            string[] flagsData = new string[2];

            foreach (ParticalKeyPanel bkeyPanel in tp.Controls)
            {
                string bKeyData = string.Format("\t{0}", bkeyPanel.getVal());

                string val = null;

                if (bKeyData.Contains("IgnoreFxTint"))
                    flagsData[0] = bKeyData;

                if (bKeyData.Contains("AlwaysDraw"))
                {
                    flagsData[1] = bKeyData;
                    val = processFlags(flagsData);
                }

                if (!bKeyData.Contains("AlwaysDraw") &&
                    !bKeyData.Contains("IgnoreFxTint"))
                    val = bKeyData;

                if (val != null )
                {
                    data.Add(val);

                    data.Add("");
                }
            }

            data.Add("");
        }

        private void copyTabPageMenuItem_Click(object sender, EventArgs e)
        {
            ArrayList data = new ArrayList();
            
            data.Add("#####PartCopy#####");
            
            TabPage selTP = part_tabControl.SelectedTab;
            
            data.Add("##" + selTP.Text);
            
            getTabData(ref data, selTP, false);
            
            common.COH_IO.SetClipBoardDataObject(data);
            
            pasteTabPageMenuItem.Enabled = true;
        }

        private void pasteTabPageMenuItem_Click(object sender, EventArgs e)
        {
            if (isLocked)
            {
                MessageBox.Show("File is ReadOnly\r\nNo Action was performed");
            }
            else
            {
                ArrayList data = common.COH_IO.GetClipBoardStringDataObject();

                if (data != null && data.Count > 2)
                {
                    string header = (string)data[0];

                    if (header.Equals("#####PartCopy#####"))
                    {
                        TabPage selTP = part_tabControl.SelectedTab;

                        string tabTitle = ((string)data[1]).Replace("#","");

                        if (tabTitle.Equals(selTP.Text))
                        {
                            data.Remove("#####PartCopy#####");

                            data.Remove("##" + selTP.Text);

                            this.SuspendLayout();

                            foreach (ParticalKeyPanel pkeyPanel in selTP.Controls)
                            {
                                string pKey = pkeyPanel.Name.Replace("_ParticalKeyPanel", "");

                                string pVal = getPartKeyVal(pKey, data);

                                if (pVal != null)
                                {
                                    if (pVal.ToLower().EndsWith(".tga"))
                                    {
                                        string tpVal = getTexturePath(pVal);

                                        if (tpVal != null)
                                            pVal = tpVal;
                                    }

                                    pkeyPanel.setVal(this, pVal, isLocked);

                                    pkeyPanel.isDirty = true;

                                    keyPanel_IsDirtyChanged(pkeyPanel, new EventArgs());
                                }
                            }

                            //fixPanelLayout();

                            this.ResumeLayout(true);

                            enable(!isLocked);

                            pasteTabPageMenuItem.Enabled = false;
                        }
                        else
                        {
                            string mStr = "The selected tab dose not match the copied tab\r\n";
                            mStr += string.Format("\"{0}\"", tabTitle);
                            MessageBox.Show(mStr);
                        }
                    }
                    else
                    {
                        MessageBox.Show("Clipboard Data is not a Partical file data");
                    }
                }
            }
        }

        public void loadStandAlonePartFile(string file_name)
        {
            ArrayList partData = new ArrayList();
            
            common.COH_IO.fillList(partData, file_name);
            
            Partical part = ParticalParser.parse(file_name, ref partData);

            bpcfPanel.partStandAloneFile.Add(part);

            if (!bpcfPanel.eventsComboBx.Items.Contains("StandAloneFiles"))
                this.bpcfPanel.eventsComboBx.Items.Add("StandAloneFiles");

            if (standAloneWindow)
                bpcfPanel.initializeComboBxs(null);

            bpcfPanel.eventsComboBx.SelectedIndex = bpcfPanel.eventsComboBx.Items.Count - 1;
           
            //bpcfPanel.eventsBhvrFilesMComboBx.SelectedIndex = bpcfPanel.eventsBhvrFilesMComboBx.Items.Count - 1;
        }

        private void updatePart()
        {
            ArrayList data = getData();

            Partical part = ParticalParser.parse(fileName, ref data);

            foreach (KeyValuePair<string, object> kvp in part.pParameters)
            {
                this.Tag.pParameters[kvp.Key] = kvp.Value;
            }
        }

        private void keyPanel_IsDirtyChanged(object sender, EventArgs e)
        {
            if (!isDirty)
            {
                isDirty = ((ParticalKeyPanel)sender).isDirty;

                if (IsDirtyChanged != null)
                    IsDirtyChanged(this, e);

                setFileStatus();
            }
        }

        public void setIsDirty(bool dirty)
        {
            this.SuspendLayout();
            //isDirty = dirty;
            foreach (ParticalKeyPanel pk in partPanels)
            {
                pk.isDirty = dirty;
            }

            this.ResumeLayout(false);

            keyPanel_IsDirtyChanged((ParticalKeyPanel)partPanels[0], new EventArgs());
        }

        private void setFileStatus()
        {
            int imgInd = this.Tag.ImageIndex;

            common.MComboBoxItem mcbI = (common.MComboBoxItem)this.bpcfPanel.eventsBhvrFilesMComboBx.SelectedItem;

            if (mcbI != null && mcbI.ImageIndex != imgInd)
            {
                this.bpcfPanel.SuspendLayout();

                mcbI.ImageIndex = imgInd;

                this.bpcfPanel.ResumeLayout(false);

                this.bpcfPanel.Invalidate();

                this.Refresh();
            }
        }

        public void setPartPanels(Partical part)
        {

            if (this.Tag != null)
            {
                updatePart();
            }

            this.Tag = part;

            fileName = part.fileName;

            isLocked = part.isReadOnly;

            isDirty = part.isDirty;

            foreach (TabPage tp in part_tabControl.Controls)
            {
                tp.SuspendLayout();
                foreach (ParticalKeyPanel pkeyPanel in tp.Controls)
                {
                    pkeyPanel.SuspendLayout();
                }
            }

            this.part_tabControl.SuspendLayout();

            this.SuspendLayout();

            foreach (TabPage tp in part_tabControl.Controls)
            {
                foreach (ParticalKeyPanel pkeyPanel in tp.Controls)
                {
                    string pKey = pkeyPanel.Name.Replace("_ParticalKeyPanel", "");

                    string pVal = null;

                    if (part.pParameters.ContainsKey(pKey) &&
                        part.pParameters[pKey] != null)
                    {

                        pVal = (string)part.pParameters[pKey];

                        if (pVal.ToLower().EndsWith(".tga"))
                        {
                            string tmpVal = getTexturePath(pVal);
                            if (tmpVal != null)
                                pVal = tmpVal;
                        }                         
                    }
                    pkeyPanel.Tag = part;

                    pkeyPanel.setVal(this, pVal, isLocked);
                }
            }

            //fixPanelLayout();

            setFileStatus();

            foreach (TabPage tp in part_tabControl.Controls)
            {
                foreach (ParticalKeyPanel pkeyPanel in tp.Controls)
                {
                    pkeyPanel.ResumeLayout(false);
                }

                tp.ResumeLayout(false);
            }

            part_tabControl.ResumeLayout(false);

            this.ResumeLayout(false);

            enable(true);            
        }

        private string getPartKeyVal(string keyName, ArrayList data)
        {
            string val = null;

            foreach (string dStr in data)
            {
                string str = dStr.Replace("\t", "").Trim();

                bool isCommented = str.StartsWith("#");

                str = str.Replace("#", "");

                if (str.ToLower().StartsWith(keyName.ToLower()))
                {
                    string suStr = str.Substring(keyName.Length);

                    val = isCommented ? string.Format("#{0}", suStr) : suStr;

                    val = val.Trim();

                    break;
                }
            }
            return val;
        }

        public void enable(bool notLocked)
        {
            foreach (TabPage tp in part_tabControl.Controls)
            {
                foreach (Control ctl in tp.Controls)
                {
                    ctl.Enabled = notLocked;
                }
            }
        }

        private void keyPanel_CheckChanged(object sender, EventArgs e)
        {
            //turnning scroll on/off to stop tabpage from scrolling during disable loop
            this.partPartical_tabPage.AutoScroll = false;
            
            this.partPartical_tabPage.VerticalScroll.Visible = true;
            
            CheckBox cbx = (CheckBox)sender;
            
            if (cbx.Text.ToLower().Equals("partical"))
            {
                this.SuspendLayout();
            
                this.part_tabControl.SuspendLayout();
                
                this.partPartical_tabPage.SuspendLayout();

                foreach (Control ctl in partPartical_tabPage.Controls)
                {
                    ctl.Enabled = cbx.Checked;
                }

                cbx.Parent.Enabled = true;
                
                //not needed if autoscroll is off
                //bhvrPhysics_tabPage.ScrollControlIntoView(cbx);
                this.partPartical_tabPage.ResumeLayout(false);
                
                this.part_tabControl.ResumeLayout(false);
                
                this.ResumeLayout(false);
            }
            this.partPartical_tabPage.AutoScroll = true;
        }

        private void part_tabControl_MouseWheel(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            int scrollVal = ((TabPage)part_tabControl.SelectedTab).VerticalScroll.Value;
            
            int scrollMax = ((TabPage)part_tabControl.SelectedTab).VerticalScroll.Maximum;
            
            int scrollMin = ((TabPage)part_tabControl.SelectedTab).VerticalScroll.Minimum;

            if (e.Delta > 0)
            {
                scrollVal -= ((TabPage)part_tabControl.SelectedTab).VerticalScroll.LargeChange;
            }
            else
            {
                scrollVal += ((TabPage)part_tabControl.SelectedTab).VerticalScroll.LargeChange;
            }

            scrollVal = Math.Min(Math.Max(scrollVal, scrollMin), scrollMax);
            
            ((TabPage)part_tabControl.SelectedTab).VerticalScroll.Value = scrollVal;
        }
    }
}
