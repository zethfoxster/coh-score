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
    public partial class BehaviorWin : 
        Panel 
        //Form
    {
        private int width;
        
        //private string rootPath;
        public string fileName;
       
        //private ArrayList data;
        public BhvrNPartCommonFunctionPanel bpcfPanel;
        
        public event EventHandler BhvrComboBoxChanged;
       
        public ArrayList bhvrPanels;

        public event EventHandler IsDirtyChanged;
       
        public bool isDirty;
       
        public bool isLocked;
        
        public Behavior BhvrTag;

        public Event EventTag;

        public common.FormatedToolTip fToolTip;

        public BehaviorWin(ImageList imgList, string file_name)
        {
            fileName = file_name;
            width = 580;

            isLocked = true;

            isDirty = false;

            bhvrPanels = new ArrayList();

            InitializeComponent();

            BhvrTag = null;

            bpcfPanel = new BhvrNPartCommonFunctionPanel(this, "Behavior", imgList);

            bpcfPanel.PartOrBhvrComboBoxChanged -= new EventHandler(bpcfPanel_partOrBhvrComboBoxChanged);
            bpcfPanel.PartOrBhvrComboBoxChanged += new EventHandler(bpcfPanel_partOrBhvrComboBoxChanged);
            
            buildKeyPanel();
                        
            this.Controls.Add(bpcfPanel);
            
            bpcfPanel.Dock = DockStyle.Top;

            ArrayList ctlList = buildToolTipsObjList();

            fToolTip = common.COH_htmlToolTips.intilizeToolTips(ctlList, @"assetEditor/objectTrick/BhvrToolTips.html");

        }

        private ArrayList buildToolTipsObjList()
        {
            ArrayList ctlList = new ArrayList();

            foreach (TabPage tp in bhvr_tabControl.Controls)
            {
                foreach (BehaviorKeyPanel bkeyPanel in tp.Controls)
                {
                    string bKey = bkeyPanel.Name.Replace("_BehaviorKeyPanel", "");
                    ctlList.Add(new common.COH_ToolTipObject(bkeyPanel, bKey));
                }
            }

            return ctlList;
        }

        private void bpcfPanel_partOrBhvrComboBoxChanged(object sender, EventArgs e)
        {
            if (BhvrComboBoxChanged != null)
                BhvrComboBoxChanged(this, e);
        }

        private void buildKeyPanel()
        {
            string gameBranch = common.COH_IO.getRootPath(this.fileName);

            gameBranch = gameBranch.Substring(@"C:\".Length).Replace(@"\data\","");
            
            BehaviorKeys bhvrKeys = new BehaviorKeys(gameBranch);

            int y = 6;

            Color ltColor = SystemColors.ControlLight;

            Color dkColor = SystemColors.Control;

            foreach(BehaviorKey bk in bhvrKeys.keysList)
            {
                bool isFlag = bk.numFields == 0;
                BehaviorKeyPanel keyPanel = new BehaviorKeyPanel(this,
                                                                 bk.name, 
                                                                 bk.fieldsLabel, 
                                                                 bk.numFields, 
                                                                 bk.min, 
                                                                 bk.max, 
                                                                 bk.defaultVal,
                                                                 bk.numDecimalPlaces,
                                                                 bk.isOverridable, 
                                                                 bk.isColor,
                                                                 isFlag, 
                                                                 bk.comboItems, 
                                                                 width,
                                                                 bk.isCkbx,
                                                                 bk.group,
                                                                 bk.subGroup);

                keyPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top
                                    | System.Windows.Forms.AnchorStyles.Left)
                                    | System.Windows.Forms.AnchorStyles.Right)));


                switch (bk.group)
                {

                    case "Basic":
                        this.bhvrBasic_tabPage.Controls.Add(keyPanel);
                        y = 2 + ((keyPanel.Height + 2) * (this.bhvrBasic_tabPage.Controls.Count-1));
                        keyPanel.mainBackColor = this.bhvrBasic_tabPage.Controls.Count % 2 == 0 ? ltColor : dkColor;
                        break;

                    case "Color/Alpha":
                        this.bhvrColorAlpha_tabPage.Controls.Add(keyPanel);
                        y = 2 + ((keyPanel.Height + 2) * (this.bhvrColorAlpha_tabPage.Controls.Count-1));
                        keyPanel.mainBackColor = this.bhvrColorAlpha_tabPage.Controls.Count % 2 == 0 ? ltColor : dkColor;
                        break;

                    case "Physics":
                        this.bhvrPhysics_tabPage.Controls.Add(keyPanel);
                        y = 2 + ((keyPanel.Height + 2) * (this.bhvrPhysics_tabPage.Controls.Count-1));
                        keyPanel.mainBackColor = this.bhvrPhysics_tabPage.Controls.Count % 2 == 0 ? ltColor : dkColor;
                        break;

                    case "Shake/Blur/Light":
                        this.bhvrShakeBlurLight_tabPage.Controls.Add(keyPanel);
                        y = 2 + ((keyPanel.Height + 2) * (this.bhvrShakeBlurLight_tabPage.Controls.Count-1));
                        keyPanel.mainBackColor = this.bhvrShakeBlurLight_tabPage.Controls.Count % 2 == 0 ? ltColor : dkColor;
                        break;

                    case "Animation/Splat":
                        this.bhvrAnimationSplat_tabPage.Controls.Add(keyPanel);
                        y = 2 + ((keyPanel.Height + 2) * (this.bhvrAnimationSplat_tabPage.Controls.Count-1));
                        keyPanel.mainBackColor = this.bhvrAnimationSplat_tabPage.Controls.Count % 2 == 0 ? ltColor : dkColor;
                        break;
                }
                
                this.bhvrPanels.Add(keyPanel);

                keyPanel.Location = new System.Drawing.Point(0,y);

                keyPanel.Size = new System.Drawing.Size(width, 26);

                keyPanel.CheckChanged -= new EventHandler(keyPanel_CheckChanged);
                keyPanel.CheckChanged += new EventHandler(keyPanel_CheckChanged);

                keyPanel.IsDirtyChanged -= new EventHandler(keyPanel_IsDirtyChanged);
                keyPanel.IsDirtyChanged += new EventHandler(keyPanel_IsDirtyChanged);

                keyPanel.enable(false);
            }

            foreach (Control ctl in bhvrPhysics_tabPage.Controls)
            {
                if (!ctl.Name.ToLower().Equals("physics_BehaviorKeyPanel".ToLower()))
                {
                    ctl.Enabled = false;
                }
            }
        }

        public void fixPanelLayout()
        {
            int y = 6;
            int bTabCount = 0;
            int cTabCount = 0;
            int pTabCount = 0;
            int sTabCount = 0;
            int aTabCount = 0;

            Color ltColor = SystemColors.ControlLight;
            Color dkColor = SystemColors.Control;

            foreach (TabPage tb in this.bhvr_tabControl.Controls)
            {
                tb.VerticalScroll.Value = 0;
            }


            foreach (BehaviorKeyPanel keyPanel in bhvrPanels)
            {
                if (keyPanel.Used)
                {
                    string group = keyPanel.Parent.Text;
                    keyPanel.Visible = true;
                    switch (group)
                    {

                        case "Basic":
                            y = 2 + ((keyPanel.Height + 2) * (bTabCount++));
                            keyPanel.mainBackColor = bTabCount % 2 == 0 ? ltColor : dkColor;
                            break;

                        case "Color/Alpha":
                            y = 2 + ((keyPanel.Height + 2) * (cTabCount++));
                            keyPanel.mainBackColor = cTabCount % 2 == 0 ? ltColor : dkColor;
                            break;

                        case "Physics":
                            y = 2 + ((keyPanel.Height + 2) * (pTabCount++));
                            keyPanel.mainBackColor = pTabCount % 2 == 0 ? ltColor : dkColor;
                            break;

                        case "Shake/Blur/Light":
                            y = 2 + ((keyPanel.Height + 2) * (sTabCount++));
                            keyPanel.mainBackColor = sTabCount % 2 == 0 ? ltColor : dkColor;
                            break;

                        case "Animation/Splat":
                            y = 2 + ((keyPanel.Height + 2) * (aTabCount++));
                            keyPanel.mainBackColor = aTabCount % 2 == 0 ? ltColor : dkColor;
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

            data.Add("Behavior");

            foreach (TabPage tp in bhvr_tabControl.Controls)
            {
                getTabData(ref data, tp);
            }

            data.Add("End");

            return data;
        }

        private void getTabData(ref ArrayList data, TabPage tp)
        {
            data.Add("");
           
            data.Add("#".PadRight(59, '#'));
            
            data.Add(string.Format("### {0} ", tp.Text).PadRight(59, '#'));
            
            data.Add("#".PadRight(59, '#'));
            
            data.Add("");

            foreach (BehaviorKeyPanel bkeyPanel in tp.Controls)
            {
                string val = bkeyPanel.getVal();

                //if (bkeyPanel.Used)//!val.StartsWith("#"))
                {
                    data.Add(string.Format("\t{0}", val));
                    data.Add("");
                }
            }
           
            data.Add("");
        }

        private void copyTabPageMenuItem_Click(object sender, EventArgs e)
        {
            ArrayList data = new ArrayList();
            
            data.Add("#####BhvrCopy#####");
            
            TabPage selTP = bhvr_tabControl.SelectedTab;

            data.Add("##" + selTP.Text);
            
            getTabData(ref data, selTP);
            
            common.COH_IO.SetClipBoardDataObject(data);
            
            pasteTabPageMenuItem.Enabled = true;
        }

        private void pasteTabPageMenuItem_Click(object sender, EventArgs e)
        {
            if (fileName.EndsWith("null.bhvr"))
            {
                MessageBox.Show("Your are not allowed to change null.bhvr file\r\nNo Action was performed");
            }
            else if (isLocked)
            {
                MessageBox.Show("File is ReadOnly\r\nNo Action was performed");
            }
            else
            {
                ArrayList data = common.COH_IO.GetClipBoardStringDataObject();

                if (data != null && data.Count > 2)
                {
                    string header = (string)data[0];

                    if (header.Equals("#####BhvrCopy#####"))
                    {
                        TabPage selTP = bhvr_tabControl.SelectedTab;

                        string tabTitle = ((string)data[1]).Replace("#", "");

                        if (tabTitle.Equals(selTP.Text))
                        {
                            data.Remove("#####BhvrCopy#####");

                            data.Remove("##" + selTP.Text);

                            this.SuspendLayout();

                            foreach (BehaviorKeyPanel bkeyPanel in selTP.Controls)
                            {
                                string bKey = bkeyPanel.Name.Replace("_BehaviorKeyPanel", "");

                                string bVal = getBhvrKeyVal(bKey, data);

                                string bOvrVal = bkeyPanel.getOverrideVal();

                                if (bOvrVal.Length > bKey.Length)
                                    bOvrVal = bOvrVal.Trim().Substring(bKey.Length).Trim();

                                bool isOverrideVal = bOvrVal != null && bOvrVal.Trim().Length > 0;

                                if (bVal != null)
                                {
                                    bkeyPanel.setVal(this, bVal, bOvrVal, isOverrideVal, isLocked);

                                    bkeyPanel.isDirty = true;

                                    keyPanel_IsDirtyChanged(bkeyPanel, new EventArgs());
                                }
                            }

                            fixPanelLayout();

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

        public void loadStandAloneBhvrFile(string file_name)
        {
            ArrayList bhvrData = new ArrayList();

            common.COH_IO.fillList(bhvrData, file_name);

            Behavior bhvr = BehaviorParser.parse(file_name, ref bhvrData);

            bpcfPanel.bhvrStandAloneFile.Add(bhvr);

            if (!bpcfPanel.eventsComboBx.Items.Contains("StandAloneFiles"))
                this.bpcfPanel.eventsComboBx.Items.Add("StandAloneFiles");

            bpcfPanel.eventsComboBx.SelectedIndex = bpcfPanel.eventsComboBx.Items.Count - 1;

            bpcfPanel.eventsBhvrFilesMComboBx.SelectedIndex = bpcfPanel.eventsBhvrFilesMComboBx.Items.Count - 1;
        }

        public void updateBhvrOverride()
        {
            Event ev = (Event)EventTag;

            if (ev != null)
            {
                ev.eBhvrOverrides.Clear();

                ev.eBhvrOverrideDic.Clear();

                //TabPage selTP = bhvr_tabControl.SelectedTab;

                foreach (TabPage tp in bhvr_tabControl.Controls)
                {
                    //bhvr_tabControl.SelectedTab = tp;

                    foreach (BehaviorKeyPanel bkeyPanel in tp.Controls)
                    {
                        if (bkeyPanel.OverrideBhvr)
                        {
                            string val = bkeyPanel.getOverrideVal();

                            ev.eBhvrOverrides.Add(val);
                        }
                    }
                }

                //bhvr_tabControl.SelectedTab = selTP;

                ev.buildBhvrOvrDic();
            }
        }

        private void updateBhvr()
        {
            ArrayList data = getData();

            Behavior bhvr = BehaviorParser.parse(fileName, ref data);

            foreach (KeyValuePair<string, object> kvp in bhvr.bParameters)
            {
                this.BhvrTag.bParameters[kvp.Key] = kvp.Value;
            }
        }

        private void keyPanel_IsDirtyChanged(object sender, EventArgs e)
        {
            if (!isDirty)
            {
                isDirty = ((BehaviorKeyPanel)sender).isDirty;

                if (IsDirtyChanged != null)
                    IsDirtyChanged(this, e);

                setFileStatus();
            }
        }

        public void setIsDirty(bool dirty)
        {
            this.SuspendLayout();
            //isDirty = dirty;
            foreach(BehaviorKeyPanel bk in bhvrPanels)
            {
                bk.isDirty = dirty;
            }
            
            this.ResumeLayout(false);
            
            keyPanel_IsDirtyChanged((BehaviorKeyPanel)bhvrPanels[0], new EventArgs());
        }

        private void setFileStatus()
        {
            int imgInd = this.BhvrTag.ImageIndex;

            common.MComboBoxItem mcbI = (common.MComboBoxItem)this.bpcfPanel.eventsBhvrFilesMComboBx.SelectedItem;

            if (mcbI != null)
            {
                this.bpcfPanel.SuspendLayout();

                mcbI.ImageIndex = imgInd;

                this.bpcfPanel.ResumeLayout(false);

                this.bpcfPanel.Invalidate();

                this.Refresh();
            }
        }

        public void setBhvrPanels(Behavior bhvr)
        {
            //if (isDirty)
            {
                if (this.EventTag != null)
                {
                    updateBhvrOverride();
                }
                if (this.BhvrTag != null)
                {
                    updateBhvr();
                }
            }

            this.BhvrTag = bhvr;

            this.EventTag = (Event)bpcfPanel.eventsBhvrFilesMComboBx.Tag;

            this.fileName = bhvr.fileName;

            isLocked = bhvr.isReadOnly;

            isDirty = bhvr.isDirty;

            Dictionary<string, object> bhvrOvrDic = null;

            if (bhvr.Tag != null &&
                ((Event)bhvr.Tag).eBhvrOverrideDic != null)
            {
                bhvrOvrDic = ((Event)bhvr.Tag).eBhvrOverrideDic.ToDictionary(k => k.Key, k => k.Value);
            }

            foreach (TabPage tp in bhvr_tabControl.Controls)
            {
                tp.SuspendLayout();
                foreach (BehaviorKeyPanel bkeyPanel in tp.Controls)
                {
                    bkeyPanel.SuspendLayout();
                }
            }
            
            this.bhvr_tabControl.SuspendLayout();

            this.SuspendLayout();

            foreach (TabPage tp in bhvr_tabControl.Controls)
            {
                foreach (BehaviorKeyPanel bkeyPanel in tp.Controls)
                {
                    string bKey = bkeyPanel.Name.Replace("_BehaviorKeyPanel","");
                    
                    string bVal = null;

                    string bOvrVal = null;
                    
                    bool isOverrideVal = false;

                    if (bhvr.bParameters.ContainsKey(bKey) && 
                        bhvr.bParameters[bKey] != null)
                    {
                        
                        bVal = (string) bhvr.bParameters[bKey];
                    }

                    if (bhvrOvrDic != null && 
                        bhvrOvrDic.ContainsKey(bKey))
                    {
                        bOvrVal = (string)bhvrOvrDic[bKey];
                        isOverrideVal = true;
                    }

                    bkeyPanel.Tag = bhvr;
                    
                    bkeyPanel.setVal(this, bVal, bOvrVal, isOverrideVal, isLocked);                    
                    
                }                
            }
            
            //fixPanelLayout();

            foreach (TabPage tp in bhvr_tabControl.Controls)
            {
                foreach (BehaviorKeyPanel bkeyPanel in tp.Controls)
                {
                    bkeyPanel.ResumeLayout(false);
                }
                tp.ResumeLayout(false);
            }

            bhvr_tabControl.ResumeLayout(false);
            
            this.ResumeLayout(false);

            this.PerformLayout();

            setFileStatus();

            bool lockCtl = bhvr.isReadOnly;

            if (System.IO.Path.GetFileName(bhvr.fileName).ToLower().Equals("null.bhvr"))
                lockCtl = false;

            enable(true);
        }
        
        private string getBhvrKeyVal(string keyName, ArrayList data)
        {
            string val = null;

            foreach (string dStr in data)
            {
                string str = dStr.Replace("\t","").Trim();

                bool isCommented = str.StartsWith("#");

                str = str.Replace("#", "");

                if (str.ToLower().StartsWith(keyName.ToLower()))
                {
                    string suStr = str.Substring(keyName.Length);

                    val = isCommented ? string.Format("#{0}",suStr):suStr;

                    val = val.Trim();

                    break;
                }
            }
            return val;
        }

        public void enable(bool notLocked)
        {
            foreach (TabPage tp in bhvr_tabControl.Controls)
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
            this.bhvrPhysics_tabPage.AutoScroll = false;
            
            this.bhvrPhysics_tabPage.VerticalScroll.Visible = true;
            
            CheckBox cbx = (CheckBox)sender;
            
            if (cbx.Text.ToLower().Equals("physics"))
            {
                this.SuspendLayout();
               
                this.bhvr_tabControl.SuspendLayout();
                
                this.bhvrPhysics_tabPage.SuspendLayout();

                foreach (Control ctl in bhvrPhysics_tabPage.Controls)
                {
                    ctl.Enabled = cbx.Checked;
                }

                cbx.Parent.Enabled = true;
                
                //not needed if autoscroll is off
                //bhvrPhysics_tabPage.ScrollControlIntoView(cbx);
                this.bhvrPhysics_tabPage.ResumeLayout(false);
                
                this.bhvr_tabControl.ResumeLayout(false);
                
                this.ResumeLayout(false);
            }
            this.bhvrPhysics_tabPage.AutoScroll = true;
        }
        
        private void bhvr_tabControl_MouseWheel(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            int scrollVal = ((TabPage)bhvr_tabControl.SelectedTab).VerticalScroll.Value;
            
            int scrollMax = ((TabPage)bhvr_tabControl.SelectedTab).VerticalScroll.Maximum;
            
            int scrollMin = ((TabPage)bhvr_tabControl.SelectedTab).VerticalScroll.Minimum;

            if (e.Delta>0)
            {
                scrollVal -= ((TabPage)bhvr_tabControl.SelectedTab).VerticalScroll.LargeChange;
            }
            else
            {
                scrollVal += ((TabPage)bhvr_tabControl.SelectedTab).VerticalScroll.LargeChange;
            }

            scrollVal = Math.Min(Math.Max(scrollVal, scrollMin),scrollMax);
            
            ((TabPage)bhvr_tabControl.SelectedTab).VerticalScroll.Value = scrollVal;
        }
    }
}
