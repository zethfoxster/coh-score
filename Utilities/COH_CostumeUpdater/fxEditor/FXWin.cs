using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace COH_CostumeUpdater.fxEditor
{
    public partial class FXWin : Panel
        //Form
    {
        public Dictionary<string, string> tgaFilesDictionary;

        private ArrayList configData;

        private object localClipBoardObj;

        private string fileName;

        private ArrayList fxAList;

        private string rootPath;
        
        private Dictionary<string, CheckBox> flagsDic;

        private Dictionary<string, object> bhvrNpart;

        private Dictionary<string, bool> mEventConfig;
        
        private Dictionary<string, bool> bWinConfig;

        private Dictionary<string, bool> pWinConfig;

        private ArrayList bhvrEvList;

        private ArrayList partEvList;

        private BehaviorWin bWin;

        private ParticalsWin pWin;

        private bool settingComboBx;

        private Color commentBkColor;

        private bool isDirty;

        public FXWin(string file_name, ref Dictionary<string, string> tgaDic, ImageList image_list )
        {
            configData = new ArrayList();
            
            configData.Clear();

            mEventConfig = new Dictionary<string, bool>();

            bWinConfig = new Dictionary<string, bool>();

            pWinConfig = new Dictionary<string, bool>();

            loadConfig();

            localClipBoardObj = null;

            this.fileName = file_name;

            commentBkColor = Color.LightGreen;

            settingComboBx = false;

            this.rootPath = common.COH_IO.getRootPath(fileName);

            InitializeComponent();

            fxOption_panel.Width = this.fx_tabPage.Width;
            fxTags_panel.Width = this.fx_tabPage.Width;
            fxName_panel.Width = this.fx_tabPage.Width;
            fxInput_panel.Width = this.fx_tabPage.Width;
            fxClampMinMax_panel.Width = this.fx_tabPage.Width;
            fxFileAgePlus_panel.Width = this.fx_tabPage.Width;
            fxLifSpanPlus_panel.Width = this.fx_tabPage.Width;

            fxLifeSpan_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxFileAge_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxPerformanceRadius_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxOnForceRadius_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxAnimScale_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxClampMaxScaleX_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxClampMaxScaleZ_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxClampMaxScaleY_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxClampMinScaleZ_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxClampMinScaleY_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxClampMinScaleX_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);
            fxLighting_numBx.ValueChanged += new System.EventHandler(numBx_ValueChanged);

            NotCompatibleWithAnimalHead_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            SoundOnly_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            DontInheritTexFromCostume_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            InheritGeoScale_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            DontSuppress_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            IsWeapon_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            DontInheritBits_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            IsShield_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            InheritAnimScale_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            UseSkinTint_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            IsArmor_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);
            InheritAlpha_ckbx.CheckedChanged += new System.EventHandler(numBx_ValueChanged);

            fx_tv.CheckBoxes = true;
            
            this.buildFlagDic();

            tgaFilesDictionary = tgaDic.ToDictionary(k => k.Key, k => k.Value);

            //Don't create duplicat bhvr and part of the same file just reference them 
            //so when a file changed all its reference will change too 
            bhvrNpart = new Dictionary<string, object>();

            bhvrEvList = new ArrayList();

            partEvList = new ArrayList();

            this.fx_tv.ImageList = image_list;

            //ArrayList ctlList = buildToolTipsObjList();

            //common.COH_htmlToolTips.intilizeToolTips(ctlList, @"assetEditor/objectTrick/FxToolTips.html");
        }

        private void loadConfig()
        {
            if (File.Exists(@"C:\Cryptic\scratch\fxConfig"))
            {
                common.COH_IO.fillList(configData, @"C:\Cryptic\scratch\fxConfig");
                
                mEventConfig.Clear();

                bWinConfig.Clear();
                
                pWinConfig.Clear();

                foreach (string str in configData)
                {
                    string[] split = str.Split(' ');

                    switch(split[0])
                    {
                        case "eventWin":
                            mEventConfig[split[1]] = true;
                            break;

                        case "bhvrWin":
                            bWinConfig[split[1]] = true;
                            break;

                        case "partWin":
                            pWinConfig[split[1]] = true;
                            break;
                    }
                }
            }
        }

        public string FileName
        {
            get
            {
                return fileName;
            }
        }

        public string RootPath
        {
            get
            {
                return rootPath;
            }
        }

        //called by FX_FileWin save()
        public ArrayList getData()
        {
            //reads FX and other windows to update the fxNode data before saving
            updateFxData();

            ArrayList data = new ArrayList();

            foreach (TreeNode fxNode in fx_tv.Nodes)
            {
                data.Add("#".PadRight(59));
                data.Add("FxInfo");
                data.Add("");

                //get fx flags Dictionary<string, bool>
                getFxFlags(ref data, fxNode);
                
                // get inputs 
                getFxInputs(ref data, fxNode);

                // get fParameters Dictionary<string, string>
                getFxParameters(ref data, fxNode);

                data.Add("");
                data.Add("#".PadRight(59, '#'));

                foreach (TreeNode conditionNode in fxNode.Nodes)
                {
                    string commentedSymbol = conditionNode.Checked ? "" : "#";
                    
                    data.Add("");
                    
                    data.Add("\t"+ (commentedSymbol + "Condition").Trim());

                    //get cParameters Dictionary<string, object>
                    getCondParameters(ref data, conditionNode);

                    foreach (TreeNode eventNode in conditionNode.Nodes)
                    {
                        getEventData(ref data, eventNode);
                    }

                    data.Add("");
                    data.Add("\t" + (commentedSymbol + "End").Trim());                    
                }

                data.Add("");
                data.Add("End");
            }

            clearDirtyBit();

            return data;            
        }

        public void fillFxTree(ArrayList fxList, BehaviorWin bhvrWin, ParticalsWin partWin)
        {
            if (fxList != null && fxList.Count > 0)
            {
                localClipBoardObj = null;

                fxAList = (ArrayList)fxList.Clone();

                bhvrNpart.Clear();                

                this.bWin = bhvrWin;

                this.pWin = partWin;

                this.bWin.bpcfPanel.fxFile = this.fileName;

                this.pWin.bpcfPanel.fxFile = this.fileName;

                //remove event handlers if exists
                this.bWin.BhvrComboBoxChanged -= new EventHandler(bWin_bhvrComboBoxChanged);

                this.bWin.IsDirtyChanged -= new EventHandler(bWin_IsDirtyChanged);

                this.pWin.PartComboBoxChanged -= new EventHandler(pWin_bhvrComboBoxChanged);

                this.pWin.IsDirtyChanged -= new EventHandler(pWin_IsDirtyChanged);

                this.mEventWin.IsDirtyChanged -= new EventHandler(mCondOrEventWin_IsDirtyChanged);

                this.mEventWin.ENameChanged -= new EventHandler(mEventWin_ENameChanged);

                this.mConditionWin.IsDirtyChanged -= new EventHandler(mCondOrEventWin_IsDirtyChanged);

                //add event handlers
                this.bWin.BhvrComboBoxChanged += new EventHandler(bWin_bhvrComboBoxChanged);

                this.bWin.IsDirtyChanged += new EventHandler(bWin_IsDirtyChanged);

                this.pWin.PartComboBoxChanged += new EventHandler(pWin_bhvrComboBoxChanged);

                this.pWin.IsDirtyChanged += new EventHandler(pWin_IsDirtyChanged);

                this.mEventWin.IsDirtyChanged += new EventHandler(mCondOrEventWin_IsDirtyChanged);

                this.mEventWin.ENameChanged += new EventHandler(mEventWin_ENameChanged);

                this.mConditionWin.IsDirtyChanged += new EventHandler(mCondOrEventWin_IsDirtyChanged);

                bhvrEvList.Clear();

                partEvList.Clear();

                this.fx_tv.BeginUpdate();

                this.fx_tv.Nodes.Clear();
                
                //remove handlers
                this.fx_tv.NodeMouseClick -= new TreeNodeMouseClickEventHandler(fx_tv_MouseClick);

                this.fx_tv.NodeMouseDoubleClick -= new TreeNodeMouseClickEventHandler(fx_tv_MouseDoubleClick);

                this.fx_tv.AfterSelect -= new TreeViewEventHandler(fx_tv_AfterSelect);

                this.fx_tv.BeforeSelect -= new TreeViewCancelEventHandler(fx_tv_BeforeSelect);

                this.fx_tv.AfterCheck -= new TreeViewEventHandler(fx_tv_AfterCheck);

                //add handlers
                this.fx_tv.NodeMouseClick += new TreeNodeMouseClickEventHandler(fx_tv_MouseClick);

                this.fx_tv.NodeMouseDoubleClick += new TreeNodeMouseClickEventHandler(fx_tv_MouseDoubleClick);

                this.fx_tv.AfterSelect += new TreeViewEventHandler(fx_tv_AfterSelect);

                this.fx_tv.BeforeSelect += new TreeViewCancelEventHandler(fx_tv_BeforeSelect);

                this.fx_tv.AfterCheck += new TreeViewEventHandler(fx_tv_AfterCheck);

                //this.fx_tv.BeforeCheck

                foreach (FX fx in fxList)
                {
                    System.Windows.Forms.TreeNode tn = new System.Windows.Forms.TreeNode();

                    tn.Tag = fx;

                    tn.Name = "tN_" + tn.GetHashCode();

                    tn.Text = "fx";

                    tn.ImageIndex = fx.ImageIndex;

                    tn.Checked = true;

                    this.fx_tv.Nodes.Add(tn);

                    int cInd = 1;

                    foreach (Condition cond in fx.conditions)
                    {
                        fillConditionTree(cInd, cond, tn, true);

                        cInd++;
                    }
                }

                this.fx_tv.EndUpdate();

                this.fx_tv.ExpandAll();

                this.fx_tv.SelectedNode = fx_tv.Nodes[0];

                this.Tag = (FX)fxList[0];

                setFxWinData();

                this.bWin.initializeComboBx(bhvrEvList, true);

                this.pWin.initializeComboBx(partEvList, true);
                
                this.mEventWin.filter(mEventConfig);
                
                filterBhvrWin(bWinConfig);
                
                filterPartWin(pWinConfig);
            }
            else
            {
                MessageBox.Show("Faild to load FX file: \r\n" + fileName);
            }

        }
        
        private ArrayList buildToolTipsObjList()
        {
            ArrayList ctlList = new ArrayList();

            DataGridView dgv = this.mEventWin.getDGV();
            
            string bKey;

            foreach (DataGridViewColumn dgvc in dgv.Columns)
            {
                bKey = dgvc.Name;
                ctlList.Add(new common.COH_ToolTipObject(dgvc, bKey));
            }

            /*
            foreach (TabPage tp in bhvr_tabControl.Controls)
            {
                foreach (BehaviorKeyPanel bkeyPanel in tp.Controls)
                {
                    string bKey = bkeyPanel.Name.Replace("_BehaviorKeyPanel", "");
                    ctlList.Add(new common.COH_ToolTipObject(bkeyPanel, bKey));
                }
            }
            */
            return ctlList;
        }
        
        private void clearDirtyBit()
        {
            FX fx = (FX) this.Tag;

            fx.isDirty = false;
            
            foreach (Condition cond in fx.conditions)
            {
                cond.isDirty = false;
                foreach (Event ev in cond.events)
                {
                    ev.isDirty = false;
                }
            }

            mEventWin.isDirty = false;

            mConditionWin.isDirty = false;

            this.isDirty = false;

            fx_tv.Nodes[0].ImageIndex = getImageInd(fx_tv.Nodes[0]);

            fx_tv.Nodes[0].SelectedImageIndex = fx_tv.Nodes[0].ImageIndex;

            setNodeStatus(" ", fx_tv.Nodes[0], true);

            this.fx_tv.Invalidate();
        }

        private void fillConditionTree(int cInd, Condition cond, TreeNode tn, bool fill_eventTree)
        {

            System.Windows.Forms.TreeNode condTn = new System.Windows.Forms.TreeNode();

            condTn.Tag = cond;

            condTn.Name = "condTn_" + tn.GetHashCode();

            condTn.Text = "Condition_" + cInd;

            condTn.ImageIndex = cond.ImageIndex;

            condTn.Checked = !cond.isCommented;

            if (cond.isCommented)
                condTn.BackColor = commentBkColor;

            tn.Nodes.Add(condTn);

            cond.treeViewNode = condTn;

            if (fill_eventTree)
            {
                int eInd = 1;

                foreach (Event ev in cond.events)
                {
                    fillEventTree(eInd, ev, condTn, true, true);
                    eInd++;
                }
            }
        }

        private void fillEventTree(int eInd, Event ev, TreeNode condTn, bool fillBhvr_tree, bool fillPart_tree)
        {
            
            System.Windows.Forms.TreeNode eventTn = new System.Windows.Forms.TreeNode();

            eventTn.Tag = ev;
            
            eventTn.Name = "eventTn" + condTn.GetHashCode();

            string eNameCmmChr = ev.eName != null && ev.eName.StartsWith("#") ? "#" : " ";

            string eParamEname = ev.eParameters["EName"] != null ? (string)ev.eParameters["EName"]:"";

            ev.eName = (string.Format("{0}{1}/{2}@Event_{3}",eNameCmmChr, condTn.Text, eParamEname.Trim(), eInd)).Trim();

            string evTxt = "@Event_" + eInd;

            eventTn.Text = ev.eParameters["EName"] != null ? string.Format("{0} {1}", (string)ev.eParameters["EName"], evTxt) : evTxt;

            eventTn.ImageIndex = ev.ImageIndex;

            eventTn.Checked = !ev.isCommented;

            if (ev.isCommented)
                eventTn.BackColor = commentBkColor;

            condTn.Nodes.Add(eventTn);

            ev.treeViewNode = eventTn;

            ev.buildBhvrOvrDic();

            if (ev.eBhvrOverrides.Count > 0 || ev.eBhvrs.Count > 0)
            {
                bhvrEvList.Add(ev);
            }

            if (ev.eparts.Count > 0)
            {
                partEvList.Add(ev);
            }

            if(fillBhvr_tree)
                fillBhvrTree(ev, eventTn, ref bhvrNpart);

            if(fillPart_tree)
                fillPartTree(ev, eventTn, ref bhvrNpart);
            
        }

        private void fillBhvrTree(Event ev, TreeNode eventTn, ref Dictionary<string, object> bhvrNpart)
        {
            ev.eBhvrsObject.Clear();

            if (ev.eBhvrOverrides.Count > 0 || ev.eBhvrs.Count > 0)
            {
                string bhvr = @"behaviors\null.bhvr";

                if (ev.eBhvrs.Count > 0)
                    bhvr = (string)ev.eBhvrs[0];

                if (ev.isCommented)
                    bhvr = "#" + bhvr;

                System.Windows.Forms.TreeNode bhvrTn = new System.Windows.Forms.TreeNode();

                bool isComment = bhvr.StartsWith("#");

                //haldle comment by stripping the first # char
                if (isComment)
                    bhvr = bhvr.Substring(1).Trim();

                Behavior behavior = null;

                if (bhvrNpart.ContainsKey(bhvr))
                {
                    behavior = (Behavior)bhvrNpart[bhvr];
                    bhvrTn.ToolTipText = behavior.fileName;
                }
                else
                {
                    behavior = buildBhvr(bhvr, ev);

                    if (behavior != null)
                    {
                        bhvrNpart[bhvr] = behavior;
                        bhvrTn.ToolTipText = behavior.fileName;
                    }
                    else
                    {
                        bhvrTn.ForeColor = Color.Gray;

                        MessageBox.Show(bhvr + " File Not Found!");
                        
                        bhvrTn.ToolTipText = bhvr + " file not Found";
                    }
                }

                if (behavior != null)
                {
                    bhvrTn.Tag = behavior;

                    ev.eBhvrsObject.Add(bhvrTn.Tag);

                    bhvrTn.ImageIndex = behavior.ImageIndex;
                }

                bhvrTn.Name = "bhvrTn" + eventTn.GetHashCode();

                bhvrTn.Text = "Bhvr " + Path.GetFileName(bhvr);

                bhvrTn.Checked = !isComment;

                if (isComment)
                    bhvrTn.BackColor = commentBkColor;

                eventTn.Nodes.Insert(0,bhvrTn);
            }
        }

        private void fillPartTree(Event ev, TreeNode eventTn, ref Dictionary<string, object> bhvrNpart)
        {
            //clear partical obj and replace them by common partical obj when the same part file is used
            ev.epartsObject.Clear();

            foreach (string evPart in ev.eparts)
            {
                System.Windows.Forms.TreeNode partTn = new System.Windows.Forms.TreeNode();

                string part = evPart;

                Partical partical = null;

                bool isComment = part.StartsWith("#");

                //haldle comment by stripping the first # char
                if (isComment)
                    part = part.Substring(1).Trim();

                if (bhvrNpart.ContainsKey(part))
                {
                    partical = (Partical)bhvrNpart[part];
                    partTn.ToolTipText = partical.fileName;
                }
                else
                {
                    partical = buildPart(part, ev);

                    if (partical != null)
                    {
                        bhvrNpart[part] = partical;
                        partTn.ToolTipText = partical.fileName;
                    }
                    else
                    {
                        partTn.ForeColor = Color.Gray;
                        MessageBox.Show(part + " File Not Found!");
                        partTn.ToolTipText = part + " file not Found";
                    }
                }

                if (partical != null)
                {
                    partical.isCommented = isComment;

                    partTn.Tag = partical;

                    ev.epartsObject.Add(partTn.Tag);

                    partTn.ImageIndex = partical.ImageIndex;
                }

                partTn.Name = "partTn" + eventTn.GetHashCode();

                partTn.Text = "Part " + Path.GetFileName(part);

                partTn.Checked = !isComment;

                if (isComment)
                    partTn.BackColor = commentBkColor;

                eventTn.Nodes.Add(partTn);
            }
        }

        private void getFxFlags(ref ArrayList data, TreeNode fxNode)
        {
            string flags = "";

            FX fx =(FX) fxNode.Tag;
            foreach (KeyValuePair<string, bool> kvp in fx.flags)
            {
                if (kvp.Value)
                    flags += kvp.Key + " ";
            }
            if (flags.Trim().Length > 0)
            {
                data.Add("\tFlags " + flags.Trim());
                data.Add("");
            }
        }

        private void getFxInputs(ref ArrayList data, TreeNode fxNode)
        {
            FX fx = (FX)fxNode.Tag;

            foreach (string input in fx.inputs)
            {
                if (input.Trim().Length > 0)
                {
                    data.Add("\tInput");
                    data.Add("\t\t" + input);
                    data.Add("\tEnd");
                    data.Add("");
                }
            }
        }

        private void getFxParameters(ref ArrayList data, TreeNode fxNode)
        {
            FX fx = (FX)fxNode.Tag;

            //Lifespan 12
            if (fx.fParameters["LifeSpan"] != null)
            {
                data.Add("\tLifeSpan " + fx.fParameters["LifeSpan"]);
                data.Add("");
            }
            //no sample found
            if (fx.fParameters["FileAge"] != null)
            {
                data.Add("\tFileAge " + fx.fParameters["FileAge"]);
            }
            //Lighting  1
            if (fx.fParameters["Lighting"] != null)
            {
                data.Add("\tLighting " + fx.fParameters["Lighting"]);
            }
            //no sample found
            if (fx.fParameters["PerformanceRadius"] != null)
            {
                data.Add("\tPerformanceRadius " + fx.fParameters["PerformanceRadius"]);
            }
            //OnForceRadius 0
            if (fx.fParameters["OnForceRadius"] != null)
            {
                data.Add("\tOnForceRadius " + fx.fParameters["OnForceRadius"]);
            }
            //no sample found in fxInfo
            if (fx.fParameters["AnimScale"] != null)
            {
                data.Add("\tAnimScale " + fx.fParameters["AnimScale"]);
            }
            //ClampMinScale 0.75 0.75 0.75
            if (fx.fParameters["ClampMinScale"] != null)
            {
                data.Add("\tClampMinScale " + fx.fParameters["ClampMinScale"]);
            }
            //ClampMaxScale 1.25 1.25 1.25
            if (fx.fParameters["ClampMaxScale"] != null)
            {
                data.Add("\tClampMaxScale " + fx.fParameters["ClampMaxScale"]);
            }
        }

        private string formatCondParameters(string paramName, Condition con)
        {
            return formatCondParameters(paramName, con, "");
        }

        private string formatCondParameters(string paramName, Condition con, string triggerBitsVal)
        {
            string results = "\t\t";

            string paramVal = "";

            if (paramName.Equals("TriggerBits"))
            {
                paramVal = triggerBitsVal;
            }
            else
            {
                paramVal = ((string)con.cParameters[paramName]).Trim();
            }

            bool isCommented = paramVal.StartsWith("#");

            if (isCommented)
            {
                results += "#";
                paramVal = paramVal.Substring(1);
            }
            
            results += paramName + " " + paramVal;

            return results;
        }

        private void getCondParameters(ref ArrayList data, TreeNode conditionNode)
        {
            Condition con = (Condition)conditionNode.Tag;

            if (con.cParameters["On"] != null)
            {
                data.Add(formatCondParameters("On", con));
            }
            if (con.cParameters["Time"] != null)
            {
                data.Add(formatCondParameters("Time", con));
            }
            if (con.cParameters["TimeOfDay"] != null)
            {
                data.Add(formatCondParameters("TimeOfDay", con));
            }
            if (con.cParameters["TriggerBits"] != null)
            {
                string[] triggerBits = ((string)con.cParameters["TriggerBits"]).Split(';');
                foreach (string triggerBit in triggerBits)
                {
                    data.Add(formatCondParameters("TriggerBits", con, triggerBit.Trim()));
                }
            }
            if (con.cParameters["DayStart"] != null)
            {
                data.Add(formatCondParameters("DayStart", con));
            }
            if (con.cParameters["DayEnd"] != null)
            {
                data.Add(formatCondParameters("DayEnd", con));
            }
            if (con.cParameters["Repeat"] != null)
            {
                data.Add(formatCondParameters("Repeat", con));
            }
            if (con.cParameters["RepeatJitter"] != null)
            {
                data.Add(formatCondParameters("RepeatJitter", con));
            }
            if (con.cParameters["Random"] != null)
            {
                data.Add(formatCondParameters("Random", con));
            }
            if (con.cParameters["Chance"] != null)
            {
                data.Add(formatCondParameters("Chance", con));
            }
            if (con.cParameters["DoMany"] != null)
            {
                data.Add(formatCondParameters("DoMany", con));
            }
            // ForceThreshold 150
            if (con.cParameters["ForceThreshold"] != null)
            {
                data.Add(formatCondParameters("ForceThreshold", con));
            }
        }
       
        //not used once commenting implemented
        private void getCondParametersWOcomment(ref ArrayList data, TreeNode conditionNode)
        {
            Condition con = (Condition)conditionNode.Tag;

            if( con.cParameters["On"] != null )
            {
                data.Add("\t\tOn " + con.cParameters["On"]);
            }
            if (con.cParameters["Time"] != null)
            {
                data.Add("\t\tTime " + con.cParameters["Time"]);
            }
            if (con.cParameters["TimeOfDay"] != null)
            {
                data.Add("\t\tTimeOfDay " + con.cParameters["TimeOfDay"]);
            }
            if (con.cParameters["TriggerBits"] != null)
            {
                string[] triggerBits = ((string)con.cParameters["TriggerBits"]).Split(';');
                foreach (string triggerBit in triggerBits)
                {
                    data.Add("\t\tTriggerBits " + triggerBit.Trim());
                }
            }
            if (con.cParameters["DayStart"] != null)
            {
                data.Add("\t\tDayStart " + con.cParameters["DayStart"]);
            }
            if (con.cParameters["DayEnd"] != null)
            {
                data.Add("\t\tDayEnd " + con.cParameters["DayEnd"]);
            }
            if( con.cParameters["Repeat"] != null )
            {
                data.Add("\t\tRepeat " + con.cParameters["Repeat"]);
            }
            if( con.cParameters["RepeatJitter"] != null )
            {
                data.Add("\t\tRepeatJitter " + con.cParameters["RepeatJitter"]);
            }
            if( con.cParameters["Random"] != null )
            {
                data.Add("\t\tRandom " + con.cParameters["Random"]);
            }
            if( con.cParameters["Chance"] != null )
            {
                data.Add("\t\tChance " + con.cParameters["Chance"]);
            }
            if( con.cParameters["DoMany"] != null )
            {
                data.Add("\t\tDoMany " + con.cParameters["DoMany"]);
            }
            // ForceThreshold 150
            if (con.cParameters["ForceThreshold"] != null)
            {
                data.Add("\t\tForceThreshold " + con.cParameters["ForceThreshold"]);
            }
        }

        private void getEventData(ref ArrayList data, TreeNode eventNode)
        {
            Event ev = (Event)eventNode.Tag;

            string commentSymbol = eventNode.Checked ? "" : "#" ;

            data.Add("");
            
            data.Add("\t\t" + (commentSymbol + "Event").Trim());

            //done implementing comment
            getEventParameters(ref data, ev);

            TreeNode bhvrTn = getBhvrTN(eventNode);

            //get bhvr //done implementing comment
            if (ev.eBhvrs.Count > 0 && bhvrTn != null)
            {
                string bh = ((string)ev.eBhvrs[ev.eBhvrs.Count - 1]).Replace("#","");
                if(bhvrTn.Checked)
                    data.Add("\t\t\tBhvr " + bh);
                else 
                    data.Add("\t\t\t#Bhvr " + bh);
            }

            //get bhvrOverRide //done implementing comment
            getBhvrOverride(ref data, ev, eventNode);

            //get part //done implementing comment
            getParticalsFiles(ref data, ev, eventNode);

            //get geom //done implementing comment
            getGeom(ref data, ev);

            //get splat //done implementing comment
            getSplatFiles(ref data, ev);

            //get sound //done implementing comment
            getSoundFiles(ref data, ev);

            //get lifeSpan //done implementing comment
            getEventLifeSpan(ref data, ev);

            data.Add("\t\t" + (commentSymbol + "End").Trim());
        }

        private TreeNode getBhvrTN(TreeNode eventNode)
        {
            foreach (TreeNode tn in eventNode.Nodes)
            {
                if (tn.Text.StartsWith("Bhvr "))
                    return tn;
            }
            return null;
        }

        private string formatEvParameters(string paramName, Event ev)
        {
            string results = "\t\t\t";

            string paramVal = ((string)ev.eParameters[paramName]).Trim();
            
            bool isCommented = paramVal.StartsWith("#");

            if (isCommented)
            {
                results += "#";
                paramVal = paramVal.Substring(1);
            }

            switch (paramName)
            {
                case "SetState":
                    //SetState RIGHTHAND or SetState "Smash Pose Low"
                    paramVal = paramVal.Replace("\"","").Replace("'","");
                    paramVal = paramVal.Contains(' ') ? string.Format("\"{0}\"", paramVal) : paramVal;
                    break;
                //flag like either is or not
                case "HardwareOnly":
                case "PhysicsOnly":
                case "SoftwareOnly":
                case "CameraSpace":
                    if (paramVal.Equals("0"))
                        paramName = "";
                    paramVal = "";
                    break;
            }

            results += paramName + " " + paramVal;

            return results;
        }

        private void getEventParameters(ref ArrayList data, Event ev)
        {
            foreach (KeyValuePair<string, object> kvp in ev.eParameters)
            {
                //Splat gets added later
                if (kvp.Value != null &&
                    kvp.Key != "Splat"&&
                    kvp.Key != "LifeSpan"&&
                    kvp.Key != "LifeSpanJitter")
                {
                    string dataStr = formatEvParameters(kvp.Key, ev);
                    if(dataStr.Trim().Length>0)
                        data.Add(dataStr);
                }
            }
        }

        private void getEventParametersWOcomment(ref ArrayList data, Event ev)
        {
            if (ev.eParameters["EName"] != null)
            {
                data.Add("\t\t\tEName " + ev.eParameters["EName"]);
            }
            //flag like either HardwareOnly or not
            if (ev.eParameters["HardwareOnly"] != null)
            {
                data.Add("\t\t\tHardwareOnly");
            }
            //flag like either PhysicOnly or not
            if (ev.eParameters["PhysicsOnly"] != null)
            {
                data.Add("\t\t\tPhysicsOnly");
            }
            //flag like either SoftwareOnly or not
            if (ev.eParameters["SoftwareOnly"] != null)
            {
                data.Add("\t\t\tSoftwareOnly");
            }

            if (ev.eParameters["Type"] != null)
            {
                data.Add("\t\t\tType " + ev.eParameters["Type"]);
            }

            //Inherit Position Scale
            if (ev.eParameters["Inherit"] != null)
            {
                data.Add("\t\t\tInherit " + ev.eParameters["Inherit"]);
            }
            //Update Position
            if (ev.eParameters["Update"] != null)
            {
                data.Add("\t\t\tUpdate " + ev.eParameters["Update"]);
            }

            if (ev.eParameters["At"] != null)
            {
                data.Add("\t\t\tAt " + ev.eParameters["At"]);
            }
            if( ev.eParameters["AltPiv"] != null)
            {
                data.Add("\t\t\tAltPiv " + ev.eParameters["AltPiv"]);
            }
            if( ev.eParameters["Anim"] != null)
            {
                data.Add("\t\t\tAnim " + ev.eParameters["Anim"]);
            }
            if( ev.eParameters["AnimPiv"] != null)
            {
                data.Add("\t\t\tAnimPiv " + ev.eParameters["AnimPiv"]);
            }
            //Lookat	Target
            if (ev.eParameters["LookAt"] != null)
            {
                data.Add("\t\t\tLookAt " + ev.eParameters["LookAt"]);
            }

            //flag like either CameraSpace or not
            if( ev.eParameters["CameraSpace"] != null)
            {
                data.Add("\t\t\tCameraSpace");
            }

            if( ev.eParameters["CEvent"] != null)
            {
                data.Add("\t\t\tCEvent " + ev.eParameters["CEvent"]);
            }
            if (ev.eParameters["CThresh"] != null)
            {
                data.Add("\t\t\tCThresh " + ev.eParameters["CThresh"]);
            }
            if (ev.eParameters["CDestroy"] != null)
            {
                data.Add("\t\t\tCDestroy " + ev.eParameters["CDestroy"]);
            }

            //JEvent	:CemGateJointBreak.fx
            if( ev.eParameters["JEvent"] != null)
            {
                data.Add("\t\t\tJEvent " + ev.eParameters["JEvent"]);
            }
            //JDestroy	1
            if( ev.eParameters["JDestroy"] != null)
            {
                data.Add("\t\t\tJDestroy " + ev.eParameters["JDestroy"]);
            }
            //Magnet	Target
            if( ev.eParameters["Magnet"] != null)
            {
                data.Add("\t\t\tMagnet " + ev.eParameters["Magnet"]);
            }
            //Pmagnet HandL
            if (ev.eParameters["PMagnet"] != null)
            {
                data.Add("\t\t\tPMagnet " + ev.eParameters["PMagnet"]);
            }
            //POther  LarmR
            if( ev.eParameters["POther"] != null)
            {
                data.Add("\t\t\tPOther " + ev.eParameters["POther"]);
            }
            //Power   4 10
            if( ev.eParameters["Power"] != null)
            {
                data.Add("\t\t\tPower " + ev.eParameters["Power"]);
            }

            //SetState RIGHTHAND
            //SetState "Smash Pose Low"
            if( ev.eParameters["SetState"] != null)
            {
                string setStateVal = ((string)ev.eParameters["SetState"]).Trim();

                string setStateData = setStateVal.Contains(' ') ? string.Format("\t\t\tSetState \"{0}\"", setStateVal) : ("\t\t\tSetState " + setStateVal);

                data.Add(setStateData);
            }
            //SoundNoRepeat 3
            if( ev.eParameters["SoundNoRepeat"] != null)
            {
                data.Add("\t\t\tSoundNoRepeat " + ev.eParameters["SoundNoRepeat"]);
            }
            //Until NONHUMAN
            if( ev.eParameters["Until"] != null)
            {
                data.Add("\t\t\tUntil " + ev.eParameters["Until"]);
            }
            //While   WEAPON
            if( ev.eParameters["While"] != null)
            {
                data.Add("\t\t\tWhile " + ev.eParameters["While"]);
            }
            if (ev.eParameters["WorldGroup"] != null)
            {
                //WorldGroup External
                data.Add("\t\t\tWorldGroup " + ev.eParameters["WorldGroup"]);
            }
            //Flags	OneAtATime AdoptParentEntity NoReticle PowerLoopingSound
            if( ev.eParameters["Flags"] != null)
            {
                data.Add("\t\t\tFlags " + ev.eParameters["Flags"]);
            }

            if (ev.eParameters["ChildFx"] != null)
            {
                data.Add("\t\t\tChildFx " + ev.eParameters["ChildFx"]);
            }

            if (ev.eParameters["Cape"] != null)
            {
                data.Add("\t\t\tCape " + ev.eParameters["Cape"]);
            }

            /*
            //no sample for AtRayFx            
             if( ev.eParameters["AtRayFx"] != null)
            {
                data.Add("\t\t\t " + ev.eParameters[""]);
            }             
            //no sample found for CRotation            
            if( ev.eParameters["CRotation"] != null)
            {
                data.Add("\t\t\t " + ev.eParameters[""]);
            } 
            ////no sample found for IncludeFx
            if( ev.eParameters["IncludeFx"] != null)
            {
                data.Add("\t\t\t " + ev.eParameters[""]);
            }
            //no sample found for ParentVelocityFraction
            if( ev.eParameters["ParentVelocityFraction"] != null)
            {
                data.Add("\t\t\t " + ev.eParameters[""]);
            }
            //no sample found
            if( ev.eParameters["RayLength"] != null)
            {
                data.Add("\t\t\t " + ev.eParameters[""]);
            }
             */
        }

        private void getEventLifeSpan(ref ArrayList data, Event ev)
        {
            if(ev.eParameters["LifeSpan"] != null)
                data.Add(formatEvParameters("LifeSpan", ev));

            if(ev.eParameters["LifeSpanJitter"] != null)
                data.Add(formatEvParameters("LifeSpanJitter", ev));
        }

        private void getSoundFiles(ref ArrayList data, Event ev)
        {
            foreach (string sound in ev.eSounds)
            {
                if (sound.Trim().StartsWith("#"))
                {
                    data.Add("\t\t\t#Sound " + sound.Trim().Substring(1));
                }
                else
                {
                    data.Add("\t\t\tSound " + sound.Trim());
                }
            }
        }

        private void getSplatFiles(ref ArrayList data, Event ev)
        {
            string splat = (string)ev.eParameters["Splat"];

            if (splat != null)
            {
                if (splat.Trim().StartsWith("#"))
                {
                    data.Add("\t\t\t#Splat " + splat.Trim().Substring(1));
                }
                else
                {
                    data.Add("\t\t\tSplat " + splat.Trim());
                }
            }
        }

        private void getParticalsFiles(ref ArrayList data, Event ev, TreeNode eventTN)
        {
            foreach (TreeNode tn in eventTN.Nodes)
            {
                if (tn.Text.StartsWith("Part "))
                {
                    string partText = tn.Text.Substring("Part ".Length);
                    string partName = findPart(ev, partText);

                    if(partName.ToLower().EndsWith(".part"))
                    {
                        partName = partName.Replace("#","");

                        if (tn.Checked)
                        {
                            //parts.Add(partName);
                            data.Add("\t\t\tPart " + partName);
                        }
                        else
                        {
                            data.Add("\t\t\t#Part " + partName);
                        }
                    }
                }
            } 
        }

        private string findPart(Event ev, string partText)
        {
            string partName = partText;

            foreach (string part in ev.eparts)
            {
                if (Path.GetFileName(part).ToLower().Equals(partText.ToLower()))
                {
                    partName = part;
                    return partName;
                }
            }
            return partName;
        }

        private void getGeom(ref ArrayList data, Event ev)
        {
            foreach (string geom in ev.eGeoms)
            {
                bool isCommented = geom.Trim().StartsWith("#");

                if (geom.Trim().Length > 0)
                {
                    if (isCommented)
                    {
                        string val = geom.Trim().Substring(1);

                        data.Add("\t\t\t#Geom " + val);
                    }
                    else
                    {
                        data.Add("\t\t\tGeom " + geom);
                    }
                }
            }
        }

        private void getBhvrOverride(ref ArrayList data, Event ev, TreeNode eventNode)
        {
            if (ev.eBhvrOverrides.Count > 0)
            {
                string commentSymbole = eventNode.Checked ? "" : "#";

                data.Add("\t\t\t" + commentSymbole + "BhvrOverride");

                foreach (KeyValuePair<string, object> kvp in ev.eBhvrOverrideDic)
                {
                    if (kvp.Value != null)
                    {
                        string value = (string)kvp.Value;

                        string tabStr = "\t\t\t\t";

                        if (value.StartsWith("#"))
                        {
                            if(eventNode.Checked)
                                tabStr += "#";

                            value = value.Substring(1);
                        }

                        data.Add(string.Format("{0}{1}{2} {3}", tabStr, commentSymbole, kvp.Key, value));
                    }
                }

                data.Add("\t\t\t" + commentSymbole + "End");
            }
        }

        private void mEventWin_ENameChanged(object sender, EventArgs e)
        {
            Event ev = (Event)sender;

            TreeNode eventTn = ev.treeViewNode;

            if (!eventTn.Text.Contains("@"))
                eventTn.Text = "@" + eventTn.Text;

            string evTxt = eventTn.Text.Substring(eventTn.Text.IndexOf("@"));

            eventTn.Text = string.Format("{0}: {1}", (string)ev.eParameters["EName"], evTxt);

            this.bWin.initializeComboBx(bhvrEvList, true);

            this.pWin.initializeComboBx(partEvList, true);
        }
        
        //loop fixed
        private void mCondOrEventWin_IsDirtyChanged(object sender, EventArgs e)
        {
            if (sender != null)
            {
                switch (sender.GetType().Name)
                {
                    case "EventWin":
                        isDirty = mEventWin.isDirty;
                        break;

                    case "ConditionWin":
                        isDirty = mConditionWin.isDirty;
                        break;

                    case "TextBox":
                        isDirty = true;
                        break;
                }


                TreeNode tn = fx_tv.SelectedNode;

                tn.ImageIndex = getImageInd(tn);

                tn.SelectedImageIndex = tn.ImageIndex;

                if (tn.Parent != null)
                    tn.Parent.ImageIndex = getImageInd(tn.Parent);

                if (tn.Text.ToLower().Contains("@event_"))
                    tn.Parent.Parent.ImageIndex = getImageInd(tn.Parent.Parent);

                fx_tv.Invalidate();
            }
                
        }
        //loop fixed
        private void pWin_IsDirtyChanged(object sender, EventArgs e)
        {
            if (pWin.isDirty != isDirty)
            {
                common.MComboBoxItem mcbi = (common.MComboBoxItem)pWin.bpcfPanel.eventsBhvrFilesMComboBx.SelectedItem;

                Partical part = (Partical)pWin.bpcfPanel.selectedBhvrOrPartObject;

                if (part != null)
                {
                    part.isDirty = pWin.isDirty;

                    string cText = "Part " + mcbi.Text;

                    fx_tv.BeginUpdate();

                    setNodeStatus(cText, fx_tv.Nodes[0]);

                    fx_tv.EndUpdate();

                    fx_tv.Invalidate();
                }
                this.Refresh();
            }
        }
        //loop fixed
        private void bWin_IsDirtyChanged(object sender, EventArgs e)
        {
            if (bWin.isDirty != isDirty)
            {
                common.MComboBoxItem mcbi = (common.MComboBoxItem)bWin.bpcfPanel.eventsBhvrFilesMComboBx.SelectedItem;

                Behavior bhvr = (Behavior)bWin.bpcfPanel.selectedBhvrOrPartObject;

                if (bhvr != null)
                {
                    bhvr.isDirty = bWin.isDirty;

                    string cText = "Bhvr " + mcbi.Text;

                    fx_tv.BeginUpdate();

                    setNodeStatus(cText, fx_tv.Nodes[0]);

                    fx_tv.EndUpdate();

                    fx_tv.Invalidate();
                }
                this.Refresh();
            }
        }
        //commented out
        private void pWin_bhvrComboBoxChanged(object sender, EventArgs e)
        {
            /*
            if (!settingComboBx)
            {
                string cText = "Part " + ((common.MComboBoxItem)pWin.bpcfPanel.eventsBhvrFilesMComboBx.SelectedItem).Text;

                Event cEvent = (Event)pWin.bpcfPanel.eventsBhvrFilesMComboBx.Tag;

                selectEventInTreeView(cEvent);

                foreach (TreeNode tn in fx_tv.SelectedNode.Nodes)
                {
                    if (tn.Text.Equals(cText))
                    {
                        setNodeStatus(cText, fx_tv.Nodes[0]);

                        tn.ImageIndex = getImageInd(tn);

                        tn.SelectedImageIndex = tn.ImageIndex;

                        fx_tv.SelectedNode = tn;

                        if (!tn.IsVisible)
                            fx_tv.TopNode = tn;

                        return;
                    }
                }
            }
            */
        }
        //commented out
        private void bWin_bhvrComboBoxChanged(object sender, EventArgs e)
        {
            /*
            if (!settingComboBx)
            {
                string cText = "Bhvr " + ((common.MComboBoxItem)bWin.bpcfPanel.eventsBhvrFilesMComboBx.SelectedItem).Text;

                Event cEvent = (Event)bWin.bpcfPanel.eventsBhvrFilesMComboBx.Tag;

                selectEventInTreeView(cEvent);
                
                foreach (TreeNode tn in fx_tv.SelectedNode.Nodes)
                {
                    if (tn.Text.Equals(cText))
                    {
                        setNodeStatus(cText, fx_tv.Nodes[0]);

                        tn.ImageIndex = getImageInd(tn);

                        tn.SelectedImageIndex = tn.ImageIndex;

                        fx_tv.SelectedNode = tn;

                        if (!tn.IsVisible)
                            fx_tv.TopNode = tn;

                        return;
                    }
                }
            }
            */
        }

        private void setNodeStatus(string nodeText, TreeNode startingNode)
        {
            setNodeStatus(nodeText, startingNode, false);
        }
        
        private void setNodeStatus(string nodeText, TreeNode startingNode, bool setAllNodes)
        {
            foreach (TreeNode tn in startingNode.Nodes)
            {
                if (setAllNodes || tn.Text == nodeText)
                {
                    tn.ImageIndex = getImageInd(tn);
                    tn.SelectedImageIndex = tn.ImageIndex;
                }

                if (tn.Nodes.Count > 0)
                    setNodeStatus(nodeText, tn, setAllNodes);
            }
        }

        private void showContextMenu(TreeNode sender)
        {
            this.fx_tv_ContextMenuStrip.Items.Clear();

            if (sender.Tag != null)
            {
                this.deleteMenuItem.Enabled = false;

                this.addMenuItem.DropDownItems.Clear();

                this.copyMenuItem.Tag = sender;
                this.renameMenuItem.Tag = sender;
                this.pasteMenuItem.Tag = sender;
                this.addConditionToolStripMenuItem.Tag = sender;
                this.addEventToolStripMenuItem.Tag = sender;
                this.addBhvrToolStripMenuItem.Tag = sender;
                this.addPartToolStripMenuItem.Tag = sender;
                this.addSoundToolStripMenuItem.Tag = sender;
                this.addSplatToolStripMenuItem.Tag = sender;

                this.copyBhvrOvrMenuItem.Tag = sender;
                this.pasteBhvrOvrMenuItem.Tag = sender;
                this.addBhvrOvrMenuItem.Tag = sender;

                this.moveUpMenuItem.Tag = sender;
                this.moveDwnMenuItem.Tag = sender;

                this.deleteMenuItem.Tag = sender;
                this.deleteSplatMenuItem.Tag = sender;
                this.openInExplorerMenuItem.Tag = sender;

                switch (sender.Tag.GetType().Name)
                {
                    case "FX":
                        this.fx_tv_ContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.sendToUltraEditToolStripMenuItem,
                                                                this.openInExplorerMenuItem,
                                                                this.pasteMenuItem,
                                                                this.addMenuItem});

                        this.addMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.addConditionToolStripMenuItem,
                                                                this.addInputToolStripMenuItem});
                        break;

                    case "Condition":
                        this.fx_tv_ContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.copyMenuItem,
                                                                this.pasteMenuItem,
                                                                this.moveUpMenuItem,
                                                                this.moveDwnMenuItem,
                                                                this.addMenuItem,
                                                                this.deleteMenuItem});

                        this.addMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.addEventToolStripMenuItem});

                        break;

                    case "Event":
                        this.fx_tv_ContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.copyMenuItem,
                                                                this.pasteMenuItem,
                                                                this.moveUpMenuItem,
                                                                this.moveDwnMenuItem,
                                                                this.copyBhvrOvrMenuItem,
                                                                this.pasteBhvrOvrMenuItem,
                                                                this.addMenuItem,
                                                                this.deleteMenuItem});

                        if (((Event)sender.Tag).eParameters["Splat"] != null)
                        {
                            this.fx_tv_ContextMenuStrip.Items.Add(this.deleteSplatMenuItem);
                        }

                        this.addMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.addBhvrToolStripMenuItem,
                                                                this.addBhvrOvrMenuItem,
                                                                this.addPartToolStripMenuItem,
                                                                this.addSoundToolStripMenuItem,
                                                                this.addSplatToolStripMenuItem});
                        break;

                    case "Behavior":
                        if (sender.Text.Equals("Bhvr null.bhvr"))
                        {
                            this.fx_tv_ContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.sendToUltraEditToolStripMenuItem});
                        }
                        else
                        {
                            this.fx_tv_ContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.sendToUltraEditToolStripMenuItem,
                                                                this.openInExplorerMenuItem,
                                                                this.copyMenuItem,
                                                                this.pasteMenuItem,
                                                                this.deleteMenuItem});
                        }
                        break;

                    case "Partical":
                        this.fx_tv_ContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.sendToUltraEditToolStripMenuItem,
                                                                this.openInExplorerMenuItem,
                                                                this.moveUpMenuItem,
                                                                this.moveDwnMenuItem,
                                                                this.copyMenuItem,
                                                                this.pasteMenuItem,
                                                                this.addMenuItem,
                                                                this.renameMenuItem,
                                                                this.deleteMenuItem});

                        this.addMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.addPartToolStripMenuItem});
                        break;
                }
                if (this.Tag != null)
                {
                    foreach (ToolStripMenuItem mi in this.fx_tv_ContextMenuStrip.Items)
                    {
                        mi.Enabled = !((FX)this.Tag).isReadOnly;
                    }
                }

                this.sendToUltraEditToolStripMenuItem.Enabled = true;

                this.openInExplorerMenuItem.Enabled = true;

                //getting iData here to allow copying across multiple window and to test for enabling pasteMenuItem
                IDataObject iData = Clipboard.GetDataObject();
                
                localClipBoardObj = null;

                localClipBoardObj = getDataObject(iData);

                this.pasteMenuItem.Enabled = localClipBoardObj != null;

                this.pasteBhvrOvrMenuItem.Enabled = localClipBoardObj != null && localClipBoardObj.GetType().Name == "Event";

                if (this.pasteMenuItem.Enabled &&
                    localClipBoardObj.GetType().Name == "Behavior" && 
                    sender.Tag.GetType().Name != "Event")
                {
                    this.pasteMenuItem.Enabled = false;
                }
            }
            else
            {
                this.deleteMenuItem.Enabled = !((FX)this.Tag).isReadOnly;
                this.deleteMenuItem.Tag = sender;
                this.deleteSplatMenuItem.Tag = sender;
                this.fx_tv_ContextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
                                                                this.deleteMenuItem});
            }
            fx_tv.ContextMenuStrip = this.fx_tv_ContextMenuStrip;
        }

        private void fixNodeCommentStatus(TreeNode tn)
        {
            if (tn.Tag != null)
            {
                if (!settingComboBx)
                {
                    settingComboBx = true;

                    updateFxData();

                    switch (tn.Tag.GetType().Name)
                    {
                        case "Condition":
                            commentCondition((Condition)tn.Tag, !tn.Checked);
                            break;

                        case "Event":
                            commentEvent((Event)tn.Tag, !tn.Checked);
                            break;

                        case "Behavior":
                            commetnBhvr((Event)tn.Parent.Tag, !tn.Checked);
                            break;

                        case "Partical":
                            string partName = tn.Text;
                            commentPartical((Event)tn.Parent.Tag, !tn.Checked, partName);
                            break;
                    }

                    setFxWinData();
                    bWin.initializeComboBx(this.bhvrEvList, true);
                    pWin.initializeComboBx(this.partEvList, true);
                    settingComboBx = false;
                }
            }

        }
        
        private void commentCondition(Condition cond, bool comment)
        {
            cond.isCommented = comment;

            #region cond.cParameters
            Dictionary<string, object> tmpDic = cond.cParameters.ToDictionary(k => k.Key, k => k.Value);

            foreach (KeyValuePair<string, object> kvp in tmpDic)
            {
                if (cond.cParameters[kvp.Key] != null)
                {
                    if (comment)
                    {
                        if (!((string)cond.cParameters[kvp.Key]).StartsWith("#"))
                            cond.cParameters[kvp.Key] = "#" + cond.cParameters[kvp.Key];
                    }
                    else
                    {
                        cond.cParameters[kvp.Key] = ((string)cond.cParameters[kvp.Key]).Replace("#", "");
                    }
                }
            }
            #endregion

            #region condition events
            foreach (Event ev in cond.events)
            {
                commentEvent(ev, comment);
            }
            #endregion
        }

        private void commentEvent(Event ev, bool comment)
        {
            ev.isCommented = comment;

            #region event eName
            if (ev.eName != null)
            {
                if (comment)
                {
                    if(!ev.eName.StartsWith("#"))
                        ev.eName = "#" + ev.eName;
                }
                else
                {
                    ev.eName = ev.eName.Replace("#", "");
                }
            }
            #endregion
            
            #region event eGeoms
            for (int j = 0; j < ev.eGeoms.Count; j++)
            {
                if (comment)
                {
                    if (!((string)ev.eGeoms[j]).StartsWith("#"))
                        ev.eGeoms[j] = "#" + ev.eGeoms[j];
                }
                else
                {
                    ev.eGeoms[j] = ((string)ev.eGeoms[j]).Replace("#", "");
                }
            }
            #endregion
            
            #region event eSounds
            for (int m = 0; m < ev.eSounds.Count; m++)
            {
                if (comment)
                {
                    if (!((string)ev.eSounds[m]).StartsWith("#"))
                        ev.eSounds[m] = "#" + ev.eSounds[m];
                }
                else
                {
                    ev.eSounds[m] = ((string)ev.eSounds[m]).Replace("#", "");
                }
            }
            #endregion

            #region event eParameters
            Dictionary<string, object> tmpDic = ev.eParameters.ToDictionary(k=>k.Key,k=>k.Value);

            foreach(KeyValuePair<string,object> kvp in tmpDic)
            {
                if (ev.eParameters[kvp.Key] != null)
                {
                    if (comment)
                    {
                        if(!((string)ev.eParameters[kvp.Key]).StartsWith("#"))
                            ev.eParameters[kvp.Key] = "#" + ev.eParameters[kvp.Key];
                    }
                    else
                    {
                        ev.eParameters[kvp.Key] = ((string)ev.eParameters[kvp.Key]).Replace("#", "");
                    }
                }
            }
            #endregion

            #region event bhvr
            commetnBhvr(ev, comment);
            #endregion

            #region event part
            for (int i = 0; i < ev.eparts.Count; i++)
            {
                if (comment)
                {
                    if (!((string)ev.eparts[i]).StartsWith("#"))
                        ev.eparts[i] = "#" + ev.eparts[i];
                }
                else
                {
                    ev.eparts[i] = ((string)ev.eparts[i]).Replace("#", "");
                }
            }
            #endregion

            #region event bhvroverride
            for (int ii = 0; ii < ev.eBhvrOverrides.Count; ii++)
            {
                if (comment)
                {
                    if (!((string)ev.eBhvrOverrides[ii]).StartsWith("#"))
                        ev.eBhvrOverrides[ii] = "#" + ev.eBhvrOverrides[ii];
                }
                else
                {
                    ev.eBhvrOverrides[ii] = ((string)ev.eBhvrOverrides[ii]).Replace("#", "");
                }
            }
            ev.eBhvrOverrideDic.Clear();
            ev.buildBhvrOvrDic();
            #endregion
        }
        
        private void commetnBhvr(Event ev, bool comment)
        {
            if (ev.eBhvrs.Count > 0)
            {
                if (comment)
                {
                    if (!((string)ev.eBhvrs[0]).StartsWith("#"))
                        ev.eBhvrs[0] = "#" + ev.eBhvrs[0];
                }
                else
                {
                    ev.eBhvrs[0] = ((string)ev.eBhvrs[0]).Replace("#", "");
                }
            }
        }
        
        private void commentPartical(Event ev, bool comment, string partText)
        {
            for (int i = 0; i < ev.eparts.Count; i++)
            {
                string pName = (string)ev.eparts[i];

                if (Path.GetFileName(pName).ToLower().Equals(partText.ToLower()))
                {
                    if (comment)
                    {
                        if (!((string)ev.eparts[i]).StartsWith("#"))
                            ev.eparts[i] = "#" + ev.eparts[i];
                    }
                    else
                    {
                        ev.eparts[i] = ((string)ev.eparts[i]).Replace("#", "");
                    }

                    return;
                }
            }
        }

        private void fx_tv_AfterCheck(object sender, TreeViewEventArgs e)
        {
            TreeNode tn = e.Node;

            if (!settingComboBx)
            {
                settingComboBx = true;

                if (tn.Checked)
                {
                    bool canBeChecked = canChildBeChecked(tn);

                    tn.Checked = canBeChecked;

                    if(!canBeChecked)
                    {
                        MessageBox.Show("This node can't be checked while its parent is un-checked!", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                        
                        settingComboBx = false;
                        
                        return;
                    }                    
                }
                mkNodeChkBxLikeParent(tn, tn.Checked);

                settingComboBx = false;

                fixNodeCommentStatus(tn);
            }
        }

        private bool canChildBeChecked(TreeNode tn)
        {
            if (tn.Parent != null && 
                tn.Parent != fx_tv.Nodes[0])
            {
                if (tn.Parent.Checked)
                    return canChildBeChecked(tn.Parent);
                else
                    return false;
            }
            else
            {
                return tn.Checked;
            }
        }

        private void mkNodeChkBxLikeParent(TreeNode tn, bool isChecked)
        {
            foreach (TreeNode cTn in tn.Nodes)
            {
                cTn.Checked = isChecked;

                if (isChecked)
                    cTn.BackColor = fx_tv.BackColor;
                else
                {
                    cTn.BackColor = commentBkColor;
                }

                if (cTn.Nodes.Count > 0)
                    mkNodeChkBxLikeParent(cTn, isChecked);
            }
        }
        
        private void fx_tv_MouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            settingComboBx = true;

            TreeNode tn = e.Node;

            fx_tv.SelectedNode = e.Node;  

            showContextMenu(tn);

            if (fx_tv.SelectedNodes.Count > 1)
            {
                this.moveUpMenuItem.Enabled = false;
                this.moveDwnMenuItem.Enabled = false;
                this.renameMenuItem.Enabled = false;
            }

            if (ModifierKeys == Keys.Control)
                tn.ExpandAll();

            if (tn.Tag != null)
            {
                if (tn.Tag.GetType() == typeof(FX))
                {
                    fx_tabControl.SelectedTab = fx_tabPage;
                }

                else if (tn.Tag.GetType() == typeof(Condition))
                {
                    mConditionWin.selectConditionRow((Condition)tn.Tag);
                    fx_tabControl.SelectedTab = conditions_tabPage;
                }

                else if (tn.Tag.GetType() == typeof(Event))
                {
                    Event fxEvent = (Event)tn.Tag;

                    switch (fx_tabControl.SelectedTab.Text)
                    {
                        case "Ev. Splat:":
                            selectSplat(fxEvent);
                            break;

                        case "Ev. Sounds:":
                            selectSounds(fxEvent);
                            break;
                    }

                    if(!(this.fx_tabControl.SelectedTab == sound_tabPage && fxEvent.eSounds.Count > 0) &&
                       !(this.fx_tabControl.SelectedTab == splat_tabPage && fxEvent.eParameters["Splat"] != null))
                    {
                        mEventWin.selectEventRow(fxEvent);
                        fx_tabControl.SelectedTab = events_tabPage;
                    }
                }
            }               

            settingComboBx = false;
        }

        private void fx_tv_MouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            settingComboBx = true;

            TreeNode tn = e.Node;

            if (tn.Text.StartsWith("Bhvr "))
            {
                bWin.bpcfPanel.settingCbx = true;

                selectBWinEventComboBox(((Event)tn.Parent.Tag));

                bWin.bpcfPanel.settingCbx = false;

                if (bWin.bpcfPanel.eventsBhvrFilesMComboBx.Text != tn.Text.Replace("Bhvr ", ""))
                {
                    bWin.bpcfPanel.eventsBhvrFilesMComboBx.Text = tn.Text.Replace("Bhvr ", "");
                    filterBhvrWin(bWinConfig);
                }
            }

            else if (tn.Text.StartsWith("Part "))
            {
                pWin.bpcfPanel.settingCbx = true;

                selectPWinEventComboBox(((Event)tn.Parent.Tag));

                pWin.bpcfPanel.settingCbx = false;

                if (pWin.bpcfPanel.eventsBhvrFilesMComboBx.Text != tn.Text.Replace("Part ", ""))
                {
                    pWin.bpcfPanel.eventsBhvrFilesMComboBx.Text = tn.Text.Replace("Part ", "");
                    filterPartWin(pWinConfig);
                }
            }

            settingComboBx = false;
        }

        private void fx_tv_BeforeSelect(object sender, TreeViewCancelEventArgs e)
        {
            if (fx_tv.SelectedNode != null)
            {
                fx_tv.SuspendLayout();

                Color bkColor = e.Node.Checked ? e.Node.BackColor:fx_tv.BackColor;

                if (fx_tv.SelectedNode.Checked)
                    fx_tv.SelectedNode.BackColor = bkColor;
                else
                    fx_tv.SelectedNode.BackColor = commentBkColor;

                fx_tv.ResumeLayout(false);
            }
         }

        private void fx_tv_AfterSelect(object sender, TreeViewEventArgs e)
        {
            fx_tv.SuspendLayout();

            e.Node.BackColor = e.Node.Checked ? Color.Yellow:Color.YellowGreen;
           
            fx_tv.SelectedNode.ImageIndex = getImageInd(fx_tv.SelectedNode);
            
            fx_tv.SelectedNode.SelectedImageIndex = fx_tv.SelectedNode.ImageIndex;

            fx_tv.ResumeLayout(false);
        }

        private int getImageInd(TreeNode tn)
        {
            if (tn != null && tn.Tag != null)
            {
                Object obj = tn.Tag;
                switch (obj.GetType().Name)
                {
                    case "FX":
                        return ((FX)obj).ImageIndex;

                    case "Condition":
                        return ((Condition)obj).ImageIndex;

                    case "Event":
                        return ((Event)obj).ImageIndex;

                    case "Behavior":
                        return ((Behavior)obj).ImageIndex;

                    case "Partical":
                        return ((Partical)obj).ImageIndex;
                }
            }

            return 0;
        }

        private void mConditionWin_CurrentRowChanged(object sender, System.EventArgs e)
        {
            Condition cond = mConditionWin.CurrentRowCondition;
            if (cond != null)
            {
                if (fx_tv.SelectedNode.Tag.GetType() == typeof(Condition) &&
                    ((Condition)fx_tv.SelectedNode.Tag).Equals(cond))
                {
                    return;
                }
                selectNodeInTreeView(cond);
            }
        }

        private void mEventWin_CurrentRowChanged(object sender, System.EventArgs e)
        {
            Event ev = mEventWin.CurrentRowEvent;

            if (ev != null)
            {
                if (fx_tv.SelectedNode.Tag.GetType() == typeof(Event) &&
                    ((Event)fx_tv.SelectedNode.Tag).Equals(ev))
                {
                    return;
                }
                selectNodeInTreeView(ev);
            }
        }

        private void selectNodeInTreeView(Object obj)
        {            
            TreeNode ctn = null;

            switch (obj.GetType().Name)
            {
                case "Event":
                    ctn = ((Event)obj).treeViewNode;
                    break;

                case "Condition":
                    ctn = ((Condition)obj).treeViewNode;
                    break;
            }
            if (ctn != null)
            {
                fx_tv.SelectedNode = ctn;

                if (!ctn.IsVisible)
                    fx_tv.TopNode = ctn;
            }
        }

        //fixed loop
        private void eSound_CurrentRowChanged(object sender, System.EventArgs e)
        {
            EventSound evS =(EventSound)sender;

            Event ev = evS.CurrentRowEvent;

            foreach (EventSound es in sound_tabPage.Controls)
            {
                if (es != evS)
                    es.clearSelection();
            }

            ((EventSound)sender).selectSoundPanel();

            if (fx_tv.SelectedNode.Tag.GetType() == typeof(Event) &&
                ((Event)fx_tv.SelectedNode.Tag).Equals(ev))
            {
                return;
            }

            selectNodeInTreeView(ev);
        }
        //fixed loop
        private void eSplat_CurrentRowChanged(object sender, System.EventArgs e)
        {
            SplatPanel sp =(SplatPanel)sender;

            Event ev = sp.CurrentRowEvent;

            foreach (SplatPanel es in splat_tabPage.Controls)
            {
                if(es != sp)
                    es.clearSelection();
            }

            ((SplatPanel)sender).selectSplatPanel();

            if (fx_tv.SelectedNode.Tag.GetType() == typeof(Event) &&
                ((Event)fx_tv.SelectedNode.Tag).Equals(ev))
            {
                return;
            }

            selectNodeInTreeView(ev);
        }

        private void selectBWinEventComboBox(Event fxEvent)
        {
            if (bWin.bpcfPanel.eventsComboBx.Tag != null)
            {
                int j = 0;
                foreach (Event ev in ((ArrayList)bWin.bpcfPanel.eventsComboBx.Tag))
                {
                    if (ev.Equals(fxEvent))
                    {
                        bWin.bpcfPanel.eventsComboBx.Text = (string)bWin.bpcfPanel.eventsComboBx.Items[j];
                        break;
                    }
                    j++;
                }
            }
          
        }

        private void selectPWinEventComboBox(Event fxEvent)
        {
            if (pWin.bpcfPanel.eventsComboBx.Tag != null)
            {
                int j = 0;
                foreach (Event ev in ((ArrayList)pWin.bpcfPanel.eventsComboBx.Tag))
                {
                    if (ev.Equals(fxEvent))
                    {
                        pWin.bpcfPanel.eventsComboBx.Text = (string)pWin.bpcfPanel.eventsComboBx.Items[j];
                        break;
                    }
                    j++;
                }
            }
        } 
      
        private Behavior buildBhvr(string bhvrPath, Event ev)
        {
            Behavior Bhvr = null;

            if (bhvrPath.StartsWith(":"))
            {
                bhvrPath = Path.GetDirectoryName(fileName) + @"\" + bhvrPath.Substring(1);
            }
            else
            {
                bhvrPath = rootPath + @"fx\" + bhvrPath.Replace("/", @"\");
            }

            if (File.Exists(bhvrPath))
            {
                ArrayList bhvrFileContent = new ArrayList();

                common.COH_IO.fillList(bhvrFileContent, bhvrPath);

                Bhvr = BehaviorParser.parse(bhvrPath, ref bhvrFileContent);

                if (Bhvr != null)
                    Bhvr.Tag = ev;
                else
                    MessageBox.Show(string.Format("{0} was not a correct Bhvr File!", Path.GetFileName(bhvrPath)));
            }
            return Bhvr;
        }

        private Partical buildPart(string partPath, Event ev)
        {
            Partical Part = null;
            if (partPath.StartsWith(":"))
            {
                partPath = Path.GetDirectoryName(fileName) + @"\" + partPath.Substring(1);
            }
            else
            {
                partPath = rootPath + @"fx\" + partPath.Replace("/", @"\");
            }

            if (File.Exists(partPath))
            {
                ArrayList partFileContent = new ArrayList();

                common.COH_IO.fillList(partFileContent, partPath);

                Part = ParticalParser.parse(partPath, ref partFileContent);

                if (Part != null)
                    Part.Tag = ev;
                else
                    MessageBox.Show(string.Format("{0} is not a correct Part File!", Path.GetFileName(partPath)));
            }

            return Part;
        }

        private void buildFlagDic()
        {        
            this.flagsDic = new Dictionary<string,CheckBox>();

            this.flagsDic["InheritAlpha"] = InheritAlpha_ckbx;
            
            this.flagsDic["IsArmor"] = IsArmor_ckbx;
            
            this.flagsDic["InheritAnimScale"] = InheritAnimScale_ckbx;
            
            this.flagsDic["DontInheritBits"] = DontInheritBits_ckbx; 
            
            this.flagsDic["DontSuppress"] = DontSuppress_ckbx;
            
            this.flagsDic["DontInheritTexFromCostume"] = DontInheritTexFromCostume_ckbx; 
            
            this.flagsDic["UseSkinTint"] = UseSkinTint_ckbx; 
            
            this.flagsDic["IsShield"] = IsShield_ckbx;
            
            this.flagsDic["IsWeapon"] = IsWeapon_ckbx; 
            
            this.flagsDic["NotCompatibleWithAnimalHead"] = NotCompatibleWithAnimalHead_ckbx;
            
            this.flagsDic["InheritGeoScale"] = InheritGeoScale_ckbx;
            
            this.flagsDic["SoundOnly"] = SoundOnly_ckbx;
        }

        private string getNumCkControl(CheckBox ckbx, NumericUpDown num)
        {
            if (ckbx.Checked)
            {
                decimal val;
                
                //num.Value to string has fixed decimal places
                string valStr = num.Value.ToString();
                
                if (decimal.TryParse(valStr.Trim(), out val))
                {
                    //val to string gets rid of the extra 0 on the right of the decimal
                   return val.ToString();
                }
            }

                return null;
        }

        private void getFxInputControls(ref FX fx)
        {
            fx.inputs.Clear();
            foreach (Control ctl in fxInput_panel.Controls)
            {
                if (ctl.GetType() == typeof(TextBox))
                {
                    string txtBxName = ctl.Name;

                    string ckBxName = txtBxName.Replace("inpName", "fxInput").Replace("_txbx", "_ckbx");

                    Control [] controls = fxInput_panel.Controls.Find(ckBxName, true);

                    if (controls.Length > 0 && 
                        ((CheckBox)controls[0]).Checked)
                    {
                        string inpVal = ctl.Text;
                        fx.inputs.Add("InpName " + inpVal.Trim());
                    }
                }
            }
        }

        private void updateFXwithControlsVal(ref FX fx)
        {
            fx.fParameters["LifeSpan"] = getNumCkControl(fxLifeSpan_ckbx, fxLifeSpan_numBx);

            fx.fParameters["FileAge"] = getNumCkControl(fxFileAge_ckbx, fxFileAge_numBx);

            fx.fParameters["PerformanceRadius"] = getNumCkControl(fxPerformanceRadius_ckbx, fxPerformanceRadius_numBx);

            fx.fParameters["OnForceRadius"] = getNumCkControl(fxOnForceRadius_ckbx, fxOnForceRadius_numBx);

            fx.fParameters["AnimScale"] = getNumCkControl(fxAnimScale_ckbx, fxAnimScale_numBx);

            string clampMaxVal = getNumCkControl(fxClampMaxScale_ckbx, fxClampMaxScaleX_numBx);

            if(clampMaxVal != null)
            {
                clampMaxVal += " " + getNumCkControl(fxClampMaxScale_ckbx, fxClampMaxScaleZ_numBx);

                clampMaxVal += " " + getNumCkControl(fxClampMaxScale_ckbx, fxClampMaxScaleY_numBx);

                clampMaxVal = clampMaxVal.Trim();
            }

            fx.fParameters["ClampMaxScale"] = clampMaxVal;

            string clampMinVal = getNumCkControl(fxClampMinScale_ckbx, fxClampMinScaleZ_numBx);
            
            if(clampMinVal != null)
            {
                clampMinVal += " " + getNumCkControl(fxClampMinScale_ckbx, fxClampMinScaleY_numBx);

                clampMinVal += " " + getNumCkControl(fxClampMinScale_ckbx, fxClampMinScaleX_numBx);

                clampMinVal = clampMinVal.Trim();
            }

            fx.fParameters["ClampMinScale"] = clampMinVal;

            fx.fParameters["Lighting"] = getNumCkControl(fxLighting_ckbx, fxLighting_numBx);

            getFxInputControls(ref fx);
        }

        private void updateFxData()
        {
            FX fx = (FX)fx_tv.Nodes[0].Tag;

            foreach (KeyValuePair<string, CheckBox> kvp in flagsDic)
            {
                if (fx.flags.ContainsKey(kvp.Key))
                {
                    CheckBox ckbx = flagsDic[kvp.Key];
                    fx.flags[kvp.Key] = ckbx.Checked;
                }
            }

            updateFXwithControlsVal(ref fx);

            mConditionWin.updateFXConditionsData();

            bWin.updateBhvrOverride();

            mEventWin.updateFXEventsData();

            updatetFxEventSound();

            updatetFxEventSplat();
        }

        private void updatetFxEventSplat()
        {
            bool hasSplat = this.splat_tabPage.Controls.Count > 0;

            if (hasSplat)
            {
                foreach (SplatPanel eSpalt in splat_tabPage.Controls)
                {
                    string commentSymbol = eSpalt.isCommented ? "#" : "";
                    
                    Event efEvent = (Event)eSpalt.Tag;
                    
                    efEvent.eParameters["Splat"] = (commentSymbol + eSpalt.getData()).Trim();
                }
            }
        }

        private void updatetFxEventSound()
        {
            bool hasSound = this.fx_tabControl.Controls.Contains(sound_tabPage) && sound_tabPage.Controls.Count > 0;
            
            if (hasSound)
            {
                Event efPrevEvent = null;

                foreach (EventSound eSound in sound_tabPage.Controls)
                {
                    string commentSymbol = eSound.isCommented ? "#" : "";

                    Event efEvent = (Event)eSound.Tag;

                    string soundData = (commentSymbol + eSound.getData()).Trim();

                    if(efPrevEvent != efEvent)
                    {
                        efEvent.eSounds.Clear();
                    }

                    efEvent.eSounds.Add(soundData);

                    efPrevEvent = efEvent;
                }
            }            
        }

        private void setNumCkControl(string valStr, CheckBox ckbx, NumericUpDown num)
        {
            settingComboBx = true;

            if (valStr != null)
            {
                decimal val;
                if (decimal.TryParse(valStr.Trim(), out val))
                {
                    ckbx.Checked = true;
                    num.Value = val;
                }
            }

            settingComboBx = false;
        }

        private void setControlsDic(FX fx)
        {
            fxName_txbx.Text = Path.GetFileName(fx.fParameters["Name"]);

            setNumCkControl(fx.fParameters["LifeSpan"],fxLifeSpan_ckbx,fxLifeSpan_numBx);

            setNumCkControl(fx.fParameters["FileAge"],fxFileAge_ckbx,fxFileAge_numBx);

            setNumCkControl(fx.fParameters["PerformanceRadius"],fxPerformanceRadius_ckbx,fxPerformanceRadius_numBx);

            setNumCkControl(fx.fParameters["OnForceRadius"],fxOnForceRadius_ckbx,fxOnForceRadius_numBx);

            setNumCkControl(fx.fParameters["AnimScale"],fxAnimScale_ckbx,fxAnimScale_numBx);

            if (fx.fParameters["ClampMaxScale"] != null)
            {
                string[] vals = fx.fParameters["ClampMaxScale"].Trim().Split(' ');

                setNumCkControl(vals[0], fxClampMaxScale_ckbx, fxClampMaxScaleX_numBx);

                setNumCkControl(vals[1], fxClampMaxScale_ckbx, fxClampMaxScaleZ_numBx);

                setNumCkControl(vals[2], fxClampMaxScale_ckbx, fxClampMaxScaleY_numBx);
            }

            if (fx.fParameters["ClampMinScale"] != null)
            {
                string[] valsMin = fx.fParameters["ClampMinScale"].Trim().Split(' ');

                setNumCkControl(valsMin[0], fxClampMinScale_ckbx, fxClampMinScaleZ_numBx);

                setNumCkControl(valsMin[1], fxClampMinScale_ckbx, fxClampMinScaleY_numBx);

                setNumCkControl(valsMin[2], fxClampMinScale_ckbx, fxClampMinScaleX_numBx);
            }

            setNumCkControl(fx.fParameters["Lighting"], fxLighting_ckbx, fxLighting_numBx);

            setFxInputControls(fx);

            enableCtl(this, new EventArgs());
        }

        private void setFxInputControls(FX fx)
        {
            Button lastButton = null;

            this.fxInput_panel.Tag = fx;
            
            this.fxInput_panel.SuspendLayout();

            clearAddInputControls();
            //this.fxInput_panel.Controls.Clear();

            if(fxInput_panel.Controls.Count>0)
                lastButton = (Button)fxInput_panel.Controls[fxInput_panel.Controls.Count - 1];

            fxInput_panel.Height = 28 * fx.inputs.Count;

            int controlsY = 4;

            int x = fxInput_panel.Width - 30;

            int textBoxWidth = x - 86 - 6 ;
            
            foreach (string input in fx.inputs)
            {
                if (lastButton != null)
                {
                    controlsY = lastButton.Bottom + 4;

                    textBoxWidth = lastButton.Left - 86 - 6;

                    x = lastButton.Left;

                    lastButton.Image = global::COH_CostumeUpdater.Properties.Resources.sub1;

                    lastButton.Image.Tag = "sub";
                }

                string inputText = input.Substring("InpName ".Length);

                addFxInputControls(this.fxInput_panel, textBoxWidth, x, controlsY, inputText, true);

                lastButton = (Button)fxInput_panel.Controls[fxInput_panel.Controls.Count - 1];
            }

            this.fxInput_panel.ResumeLayout(false);

            this.fxInput_panel.PerformLayout();
        }

        private void setFxWinData()
        {
            settingComboBx = true;

            FX fx = (FX)fx_tv.Nodes[0].Tag;

            foreach (KeyValuePair<string,bool> kvp in fx.flags)
            {
                if (flagsDic.ContainsKey(kvp.Key))
                {
                    CheckBox ckbx = flagsDic[kvp.Key];

                    ckbx.Checked = kvp.Value;
                }
            }

            settingComboBx = false;

            setControlsDic(fx);

            this.mConditionWin.setConditionsData(fx);

            this.mEventWin.setEventsData(fx);

            this.setFXsound(fx);

            this.setSplatPanel(fx);

            enableFx(!fx.isReadOnly);
        }

        private void enableFx(bool enable)
        {
            fxFlags_gpbx.Enabled = enable;
            mConditionWin.Enabled = enable;
            mEventWin.Enabled = enable;
            fxInput_panel.Enabled = enable;
            fxClampMinMax_panel.Enabled = enable;
            fxFileAgePlus_panel.Enabled = enable;
            fxLifSpanPlus_panel.Enabled = enable;
        }

        private void setSplatPanel(FX fx)
        {
            string [] tgafileNames = tgaFilesDictionary.Values.ToArray();

            string [] tgafilePaths = tgaFilesDictionary.Keys.ToArray();
            
            splat_tabPage.SuspendLayout();

            clearSplatPanle();

            foreach (Condition cond in fx.conditions)
            {
                foreach (Event ev in cond.events)
                {
                    if (ev.eParameters["Splat"] != null)
                    {
                        string tgaFileName = (string)ev.eParameters["Splat"];
                        
                        bool commented = tgaFileName.StartsWith("#");

                        tgaFileName = commented ? tgaFileName.Substring(1) : tgaFileName;

                        string tgaPath = common.COH_IO.findTGAPathFast(tgaFileName, tgafilePaths, tgafileNames);
                        
                        if (tgaPath != null)
                        {                           
                            setSplat(tgaPath, ev, commented);
                        }
                    }
                }
            }

            splat_tabPage.ResumeLayout(false);
        }

        private void setSplat(string tgaPath, Event efEvent, bool isCommented)
        {

            settingComboBx = true;

            SplatPanel eSplatPanel = null;

            int y = 4;

            if (splat_tabPage.Controls.Count > 0)
            {
                eSplatPanel = (SplatPanel)splat_tabPage.Controls[splat_tabPage.Controls.Count - 1];

                y = eSplatPanel.Bottom + 2;
            }

            addSplatPanel(y, splat_tabPage.Width - 2, tgaPath);

            eSplatPanel = (SplatPanel)splat_tabPage.Controls[splat_tabPage.Controls.Count - 1];

            eSplatPanel.Tag = efEvent;

            eSplatPanel.CurrentRowChanged += new EventHandler(eSplat_CurrentRowChanged);

            eSplatPanel.IsDirtyChanged += new EventHandler(mCondOrEventWin_IsDirtyChanged);

            eSplatPanel.setCommentColor(isCommented);

            settingComboBx = false;
        }     
       
        private void setFXsound(FX fx)
        {
            fx_tabControl.SuspendLayout();
            this.SuspendLayout();

            clearSoundPanle();

            foreach (Condition condition in fx.conditions)
            {
                foreach (Event ev in condition.events)
                {
                    if(ev.eSounds.Count>0)
                        setSound(ev);
                }
            }

            if (!fx_tabControl.TabPages.Contains(sound_tabPage) &&
                this.sound_tabPage.Controls.Count > 0)
            {
                fx_tabControl.TabPages.Add(sound_tabPage);
            }

            else if (this.sound_tabPage.Controls.Count == 0)
            {
                this.fx_tabControl.Controls.Remove(this.sound_tabPage);
            }

            fx_tabControl.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        private void clearAddInputControls()
        {

            foreach (Control c in this.fxInput_panel.Controls)
            {
                switch (c.GetType().Name)
                {
                    case "TextBox":
                        c.Validated -= new EventHandler(inpName_txbxNew_Validated);
                        break;

                    case "Button":
                        c.Click -= new System.EventHandler(this.addControl_btn_Click);
                        c.MouseHover -= new System.EventHandler(addInput_btn_MouseHover);
                        c.MouseEnter -= new System.EventHandler(addInput_btn_MouseHover);
                        c.KeyUp -= new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
                        c.KeyDown -= new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
                        break;
                }

                c.Dispose();
            }

            this.fxInput_panel.Controls.Clear();
        }

        private void clearSoundPanle()
        {
            foreach (EventSound eSound in sound_tabPage.Controls)
            {
                eSound.addSound_btn.Click -= new System.EventHandler(this.addControl_btn_Click);
                eSound.addSound_btn.MouseHover -= new System.EventHandler(addInput_btn_MouseHover);
                eSound.addSound_btn.MouseEnter -= new System.EventHandler(addInput_btn_MouseHover);
                eSound.addSound_btn.KeyUp -= new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
                eSound.addSound_btn.KeyDown -= new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
                eSound.Dispose();
            }

            sound_tabPage.Controls.Clear();
        }

        private void clearSplatPanle()
        {
            foreach (SplatPanel sp in this.splat_tabPage.Controls)
            {
                sp.CurrentRowChanged -= new EventHandler(eSplat_CurrentRowChanged);

                sp.IsDirtyChanged -= new EventHandler(mCondOrEventWin_IsDirtyChanged);

                sp.Dispose();
            }

            this.splat_tabPage.Controls.Clear();
        }

        private void setSound(Event efEvent)
        {
            settingComboBx = true;

            foreach (string soundStr in efEvent.eSounds)
            {
                string sound = soundStr.Trim().Replace("//","#");

                bool commented = sound.StartsWith("#");

                sound = commented ? sound.Substring(1) : sound;
                  
                if(sound.Trim().IndexOf('#')> -1)
                    sound = sound.Substring(0, sound.IndexOf('#'));

                string[] soundSpilt = sound.Split(' ');

                decimal radius, ramp, volume;

                decimal.TryParse(soundSpilt[1], out radius);

                decimal.TryParse(soundSpilt[2], out ramp);

                decimal.TryParse(soundSpilt[3], out volume);

                int y = 4;

                if(sound_tabPage.Controls.Count>0)
                {  
                    y = sound_tabPage.Controls[sound_tabPage.Controls.Count - 1].Bottom + 2;
                }

                EventSound eSound = addSoundPanel(null, y, sound_tabPage.Width - 2);              

                eSound.setData(soundSpilt[0], radius, ramp, volume);

                eSound.Tag = efEvent;

                eSound.setCommentColor(commented);
            }

            settingComboBx = false;
        }

        private void selectSounds(Event efEvent)
        {
            foreach (EventSound es in sound_tabPage.Controls)
            {
                if (((Event)es.Tag).Equals(efEvent))
                {
                    es.selectSoundPanel();
                }
                else
                {
                    es.clearSelection();
                }
            }
        }

        private void selectSplat(Event efEvent)
        {
            foreach (SplatPanel es in splat_tabPage.Controls)
            {
                if (((Event)es.Tag).Equals(efEvent))
                {
                    es.selectSplatPanel();
                }
                else
                {
                    es.clearSelection();
                }

            }
        }

        private void saveConfig()
        {
            Dictionary<string, bool> eventd;

            Dictionary<string, bool> bhvrd;

            Dictionary<string, bool> partd;

            fillDic(out eventd, out bhvrd, out partd);

            configData.Clear();

            configData.Add("rootPath " + rootPath);

            foreach (KeyValuePair<string, bool> kvp in eventd)
            {
                if (kvp.Value)
                    configData.Add("eventWin " + kvp.Key);
            }

            foreach (KeyValuePair<string, bool> kvp in bhvrd)
            {
                if (kvp.Value)
                    configData.Add("bhvrWin " + kvp.Key);
            }

            foreach (KeyValuePair<string, bool> kvp in partd)
            {
                if (kvp.Value)
                    configData.Add("partWin " + kvp.Key);
            }

            common.COH_IO.writeDistFile(configData, @"C:\Cryptic\scratch\fxConfig");
        }

        private void saveConfig_Click(object sender, System.EventArgs e)
        {
            DialogResult dr = MessageBox.Show("Are you Sure you want to overwrite your FX Configuration File", "Question", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
            if (dr == DialogResult.Yes)
            {
                saveConfig();

                MessageBox.Show("Configuration was sucessfuly saved");
                
                loadConfig();
            }
        }
        
        private void fillDic(out Dictionary<string, bool> eventd, out Dictionary<string, bool> bhvrd, out Dictionary<string, bool> partd)
        {
            eventd = new Dictionary<string, bool>();

            DataGridView dgv = this.mEventWin.getDGV();

            foreach (DataGridViewColumn dgvc in dgv.Columns)
            {

                if (!eventd.ContainsKey(dgvc.Name))
                {
                    eventd[dgvc.Name] = dgvc.Visible;
                }
            }

            fillBhvrDic(out bhvrd, "_BehaviorKeyPanel");

            fillPartDic(out partd, "_ParticalKeyPanel");
        }

        private void fillBhvrDic(out Dictionary<string, bool> bDic, string panelSuffix)
        {
            bDic = new Dictionary<string, bool>();

            foreach (BehaviorKeyPanel bKeyPanel in bWin.bhvrPanels)
            {
                string keyName = bKeyPanel.Name.Replace(panelSuffix, "");

                //keyName += "__" + bKeyPanel.group;
                
                bDic[keyName] = bKeyPanel.Used;
            }
        }

        private void fillPartDic(out Dictionary<string, bool> bDic, string panelSuffix)
        {
            bDic = new Dictionary<string, bool>();

            foreach (ParticalKeyPanel bKeyPanel in pWin.partPanels)
            {
                string keyName = bKeyPanel.Name.Replace(panelSuffix, "");

                //keyName += "__" + bKeyPanel.group;

                bDic[keyName] = bKeyPanel.Used;
            }
        }

        private void findGame_btn_Click(object sender, System.EventArgs e)
        {
            string msg ="About to look for COH GAME! The GAME most be running first\r\n IS THE GAME RUNNING?";
            
            DialogResult dr = MessageBox.Show(msg, "Find Game", MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            if (dr == DialogResult.Yes)
            {
                FxLauncher.FXLauncherForm.GameWinHD = FxLauncher.FXLauncherForm.getWHD();

                StringBuilder lpString = new StringBuilder();

                FxLauncher.FXLauncherForm.GetWindowText(FxLauncher.FXLauncherForm.GameWinHD, lpString, 20);

                if (lpString.ToString().Trim().Length == 0)
                {
                    MessageBox.Show("Did not find COH Game!\r\nPlease maximize COH Game then click find game again.");
                    return;
                }
                COH_CostumeUpdaterForm cuf = (COH_CostumeUpdaterForm)this.FindForm();

                if (cuf != null)
                    cuf.writeToLogBox("Editor is Connected to: " + lpString.ToString() + "\r\n", Color.Blue);
            }
        }

        private void filter_btn_Click(object sender, System.EventArgs e)
        {
            Dictionary<string, bool> eventd;

            Dictionary<string, bool> bhvrd;

            Dictionary<string, bool> partd;

            fillDic(out eventd, out bhvrd, out partd);            

            FilterActive filterActive = new FilterActive(eventd, bhvrd, partd);

            DialogResult dr = filterActive.ShowDialog();

            if (dr == DialogResult.OK)
            {
                filterActive.updateEnabledLists(out eventd, out bhvrd, out partd);

                this.mEventWin.filter(eventd);
                filterBhvrWin(bhvrd);
                filterPartWin(partd);
            }
        }
       
        private void filterBhvrWin(Dictionary<string, bool> bhvrd)
        {
            foreach (BehaviorKeyPanel bkey in bWin.bhvrPanels)
            {
                string keyName = bkey.Name.Replace("_BehaviorKeyPanel", "");

                //keyName += "__" + bkey.group;

                if (bhvrd.ContainsKey(keyName))
                {
                    bkey.Used = bhvrd[keyName];

                    bkey.isDirty = bkey.Used;
                }
            }
            
            //bWin.isDirty = true;
            
            bWin.fixPanelLayout();

            bWin.Update();

            //bWin_IsDirtyChanged(bWin, new EventArgs());
        }

        private void filterPartWin(Dictionary<string, bool> partd)
        {
            foreach (ParticalKeyPanel bkey in pWin.partPanels)
            {
                string keyName = bkey.Name.Replace("_ParticalKeyPanel", "");

                //keyName += "__" + bkey.group;
                if (partd.ContainsKey(keyName))
                {
                    bkey.Used = partd[keyName];

                    bkey.isDirty = bkey.Used;
                }
            }
            
            //pWin.isDirty = true;

            pWin.fixPanelLayout();

            pWin.Update();

            //pWin_IsDirtyChanged(pWin, new EventArgs());
        }

        private void refreshTreeViewStatusIcon()
        {
            foreach (TreeNode fxTn in fx_tv.Nodes)
            {
                FX fx = (FX)fxTn.Tag;

                fx.refreshP4ROAttributes();

                fxTn.ImageIndex = getImageInd(fxTn);
                fxTn.SelectedImageIndex = fxTn.ImageIndex;

                foreach (TreeNode condTn in fxTn.Nodes)
                {
                    condTn.ImageIndex = getImageInd(condTn);
                    condTn.SelectedImageIndex = condTn.ImageIndex;

                    foreach (TreeNode evTn in condTn.Nodes)
                    {
                        evTn.ImageIndex = getImageInd(evTn);
                        evTn.SelectedImageIndex = evTn.ImageIndex;
                    }
                }
            }
        }

        private void p4DepotDiff_btn_Click(object sender, System.EventArgs e)
        {
            if(ModifierKeys == Keys.Control)
                assetsMangement.CohAssetMang.p4vDiffDialog(fileName);
            else
                assetsMangement.CohAssetMang.p4vDepotDiff(fileName);
        }

        private void p4Add_btn_Click(object sender, System.EventArgs e)
        {
            string results = assetsMangement.CohAssetMang.addP4(fileName);

            MessageBox.Show(results);

            refreshTreeViewStatusIcon();

            enableFx(!((FX)this.fxInput_panel.Tag).isReadOnly);
        }

        private void p4CheckOut_btn_Click(object sender, System.EventArgs e)
        {
            string results = assetsMangement.CohAssetMang.checkout(fileName);

            MessageBox.Show(results);

            refreshTreeViewStatusIcon();

            enableFx(!((FX)this.fxInput_panel.Tag).isReadOnly);
        }

        private void p4CheckIn_btn_Click(object sender, System.EventArgs e)
        {
            string results = assetsMangement.CohAssetMang.p4CheckIn(fileName);

            if (results.ToLower().Contains("no files to submit"))
                results = fileName + " - unchanged, reverted\r\n" + results;

            MessageBox.Show(results);

            refreshTreeViewStatusIcon();

            enableFx(!((FX)this.fxInput_panel.Tag).isReadOnly);
        }

        private void addInput_btn_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            Button btn = (Button)sender;

            bool isSound = btn.Name.ToLower().Contains("sound");

            int pControlCnt = btn.Parent.Controls.Count;

            if (e.Modifiers == Keys.Control)
            {
                if ((string)btn.Image.Tag == "sub")
                {
                    btn.Image = global::COH_CostumeUpdater.Properties.Resources.Add1;

                    btn.Image.Tag = "add";
                }
                else
                {
                    btn.Image = global::COH_CostumeUpdater.Properties.Resources.sub1;

                    btn.Image.Tag = "sub";
                }
            }
        }

        private void addInput_btn_MouseHover(object sender, System.EventArgs e)
        {
            Button btn = (Button)sender;
            
            btn.Select();
        }

        private EventSound addSoundControl(Button btn, int controlsY)
        {
            int y = controlsY + btn.Parent.Height + 2;

            EventSound es = (EventSound)btn.Parent;

            EventSound nes = addSoundPanel(btn, y, btn.Parent.Width);

            int ind = sound_tabPage.Controls.IndexOf(es);

            sound_tabPage.Controls.SetChildIndex(nes, ind + 1);

            for (int i = ind + 2; i < sound_tabPage.Controls.Count; i++)
            {
                EventSound nextES = (EventSound)sound_tabPage.Controls[i];
                if (nextES != nes)
                {
                    y += es.Height + 2;
                    nextES.Location = new Point(0, y);                    
                }
            }
            return nes;
        }

        private void addSplatPanel(int y, int pWidth, string tgaPath)
        {
            this.splat_tabPage.SuspendLayout();

            SplatPanel sPanel = new SplatPanel(tgaPath);

            sPanel.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                   | System.Windows.Forms.AnchorStyles.Right)));

            sPanel.BackColor = System.Drawing.SystemColors.Control;

            sPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;

            sPanel.Location = new System.Drawing.Point(0, y);

            sPanel.Name = "splatPanel";

            sPanel.Size = new System.Drawing.Size(pWidth, 60);

            sPanel.TabIndex = 0;

            this.splat_tabPage.Controls.Add(sPanel);

            this.splat_tabPage.ResumeLayout(false);

            this.splat_tabPage.PerformLayout();
        }

        private EventSound addSoundPanel(Button btn, int y, int pWidth)
        {
            this.sound_tabPage.SuspendLayout();

            //since we could remove and add controls i need uniqDig to create uniq ctl name
            int uniqDig = Math.Abs(DateTime.Now.GetHashCode());

            EventSound eventSound = new EventSound(this.rootPath);
            // 
            // eventSound
            // 
            eventSound.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            eventSound.BackColor = System.Drawing.SystemColors.Control;
            eventSound.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            eventSound.Location = new System.Drawing.Point(0, y);
            eventSound.Name = "eventSound_"+uniqDig;
            eventSound.Size = new System.Drawing.Size(pWidth, 34);
            eventSound.TabIndex = 0;
            eventSound.Text = "EvenSound:";
            eventSound.addSound_btn.Click += new System.EventHandler(this.addControl_btn_Click);
            eventSound.addSound_btn.MouseHover += new System.EventHandler(addInput_btn_MouseHover);
            eventSound.addSound_btn.MouseEnter += new System.EventHandler(addInput_btn_MouseHover);
            eventSound.addSound_btn.KeyUp += new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
            eventSound.addSound_btn.KeyDown += new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
            eventSound.CurrentRowChanged += new EventHandler(eSound_CurrentRowChanged);
            eventSound.IsDirtyChanged += new EventHandler(mCondOrEventWin_IsDirtyChanged);

            sound_tabPage.Controls.Add(eventSound);

            this.sound_tabPage.ResumeLayout(false);

            return eventSound;
        }

        private void removeSoundControl(Button btn, int controlsY)
        {
            DialogResult dr = MessageBox.Show("Are You sure you wand to Remove this Event Sound Controls?", "Warrning:", MessageBoxButtons.YesNo);

            if (dr == DialogResult.Yes)
            {
                this.sound_tabPage.SuspendLayout();

                foreach (EventSound eSound in this.sound_tabPage.Controls)
                {
                    int y = eSound.Location.Y;

                    if (y == controlsY)
                    {
                        sound_tabPage.Controls.Remove(eSound);
                        eSound.addSound_btn.Click -= new System.EventHandler(this.addControl_btn_Click);
                        eSound.addSound_btn.MouseHover -= new System.EventHandler(addInput_btn_MouseHover);
                        eSound.addSound_btn.MouseEnter -= new System.EventHandler(addInput_btn_MouseHover);
                        eSound.addSound_btn.KeyUp -= new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
                        eSound.addSound_btn.KeyDown -= new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
                        eSound.removeHandles();
                        eSound.Dispose();
                    }
                }

                int top = 0;

                sound_tabPage.VerticalScroll.Value = 0;

                foreach (Control c in this.sound_tabPage.Controls)
                {
                    c.Location = new Point(c.Location.X,top);

                    top += c.Height + 2;  
                }

                this.sound_tabPage.ResumeLayout(false);
            }
        }

        private void removeCurrentAddControls(Button btn, Panel parentPanel, int controlsY)
        {
            DialogResult dr = MessageBox.Show("Are You sure you wand to Remove this Fx-Input Controls?", "Warrning:", MessageBoxButtons.YesNo);

            if (dr == DialogResult.Yes)
            {
                parentPanel.SuspendLayout();

                Control[] ctls = new Control[3];

                int i = 0;

                foreach (Control ctl in parentPanel.Controls)
                {
                    int y = ctl.Location.Y;

                    if (y == controlsY)
                    {
                        ctls[i++] = ctl;
                    }
                }

                foreach (Control c in ctls)
                {
                    parentPanel.Controls.Remove(c);
                    
                    switch (c.GetType().Name)
                    {
                        case "TextBox":
                            c.Validated -= new EventHandler(inpName_txbxNew_Validated);
                            break;

                        case "Button":
                            c.Click -= new System.EventHandler(this.addControl_btn_Click);
                            c.MouseHover -= new System.EventHandler(addInput_btn_MouseHover);
                            c.MouseEnter -= new System.EventHandler(addInput_btn_MouseHover);
                            c.KeyUp -= new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
                            c.KeyDown -= new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp); 
                            break;
                    }
                    
                    c.Dispose();
                }

                rearrangeInputControlChildren(parentPanel);

                parentPanel.ResumeLayout(false);
            }
        }
       
        private void rearrangeInputControlChildren(Panel p)
        {
            int y = 2;

            int i = 1;

            foreach (Control ctl in p.Controls)
            {
                int x = ctl.Location.X;

                ctl.Location = new Point (x,y);

                if (i % 3 == 0)
                {
                    y += ctl.Height + 4;
                }

                i++;
            }
        }

        private void addFxInputControls(Panel parentPanel, int textBoxWidth, int btnX, int controlsY)
        {
            addFxInputControls(parentPanel, textBoxWidth, btnX, controlsY, "", false);
        }
        
        private void addFxInputControls(Panel parentPanel, int textBoxWidth, int btnX, int controlsY, string textBxText, bool isChecked)
        {
            settingComboBx = true;

            int txbxWidth = textBoxWidth;

            int y = controlsY;

            Control[] ctls = parentPanel.Controls.Find("inpName_txbx", false);

            if (ctls.Length > 0)
            {
                TextBox inpNTxbx = (TextBox)ctls[0];

                txbxWidth = inpNTxbx.Width;
            }


            int cCount = parentPanel.Controls.Count / 3;

            //since we could remove and add controls i need uniqDig to create uniq ctl name
            int uniqDig = Math.Abs(DateTime.Now.GetHashCode());
            
            Button addInput_btnNew = new Button();

            CheckBox fxInput_ckbxNew = new CheckBox();

            TextBox inpName_txbxNew = new TextBox();

            // 
            // inpName_txbx
            // 
            inpName_txbxNew.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                   | System.Windows.Forms.AnchorStyles.Right)));
            inpName_txbxNew.Location = new System.Drawing.Point(86, y);
            inpName_txbxNew.Name = "inpName" + cCount + "_" + uniqDig + "_txbx";
            inpName_txbxNew.Size = new System.Drawing.Size(txbxWidth, 24);
            inpName_txbxNew.TabIndex = 1;
            inpName_txbxNew.Text = textBxText;
            inpName_txbxNew.Validated += new EventHandler(inpName_txbxNew_Validated);
            // 
            // addInput_btn
            // 
            addInput_btnNew.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            addInput_btnNew.Image = global::COH_CostumeUpdater.Properties.Resources.Add1;
            addInput_btnNew.Location = new System.Drawing.Point(btnX, y);
            addInput_btnNew.Name = "addInput" + cCount + "_" + uniqDig + "_btn";
            addInput_btnNew.Size = new System.Drawing.Size(24, 24);
            addInput_btnNew.TabIndex = 0;
            addInput_btnNew.UseVisualStyleBackColor = true;
            addInput_btnNew.Click += new System.EventHandler(this.addControl_btn_Click);
            addInput_btnNew.MouseHover += new System.EventHandler(addInput_btn_MouseHover);
            addInput_btnNew.MouseEnter += new System.EventHandler(addInput_btn_MouseHover);
            addInput_btnNew.KeyUp += new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
            addInput_btnNew.KeyDown += new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp); 
            // 
            // fxInput_ckbx
            // 
            fxInput_ckbxNew.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                   | System.Windows.Forms.AnchorStyles.Right)));
            fxInput_ckbxNew.AutoSize = true;
            fxInput_ckbxNew.Location = new System.Drawing.Point(9, y);
            fxInput_ckbxNew.Name = "fxInput" + cCount + "_" + uniqDig + "_ckbx";
            fxInput_ckbxNew.Size = new System.Drawing.Size(72, 24);
            fxInput_ckbxNew.TabIndex = 0;
            fxInput_ckbxNew.Text = "InpName:";
            fxInput_ckbxNew.UseVisualStyleBackColor = true;
            fxInput_ckbxNew.Checked = isChecked;


            parentPanel.Controls.Add(fxInput_ckbxNew);
            parentPanel.Controls.Add(inpName_txbxNew);
            parentPanel.Controls.Add(addInput_btnNew);

            parentPanel.Height += 28;

            settingComboBx = false;
        }

        private void inpName_txbxNew_Validated(object sender, EventArgs e)
        {
            if (!settingComboBx && !isDirty)
            {
                ((FX)this.fxInput_panel.Tag).isDirty = true;
                mCondOrEventWin_IsDirtyChanged(sender, e);
            }
        }
        //loop fixed
        private void numBx_ValueChanged(object sender, System.EventArgs e)
        {
            if (!settingComboBx && !isDirty)
            {
                ((FX)this.fxInput_panel.Tag).isDirty = true;
                mCondOrEventWin_IsDirtyChanged(sender, e);
                isDirty = true;
            }
        }

        private void addControl_btn_Click(object sender, EventArgs e)
        {
            Button btn = (Button)sender;

            bool isSound = btn.Name.ToLower().Contains("sound");

            int controlsY = btn.Bottom + 4;

            int textBoxWidth = btn.Left - 86 - 6;

            Panel parentPanel = (Panel)btn.Parent;

            if (((string)btn.Image.Tag) == "sub")
            {
                if (isSound)
                {
                    removeSoundControl(btn, btn.Parent.Location.Y);
                    if(sound_tabPage.Controls.Count > 0)
                        ((EventSound)sound_tabPage.Controls[sound_tabPage.Controls.Count - 1]).addSound_btn.Image = global::COH_CostumeUpdater.Properties.Resources.Add1;
                }
                else
                {
                    removeCurrentAddControls(btn, parentPanel, btn.Location.Y);
                    if(parentPanel.Controls.Count > 0)
                        ((Button)parentPanel.Controls[parentPanel.Controls.Count - 1]).Image = global::COH_CostumeUpdater.Properties.Resources.Add1;
                }

            }
            else
            {
                if (isSound)
                {
                    EventSound SenderESound = (EventSound)parentPanel;

                    sound_tabPage.SuspendLayout();

                    EventSound addedESound = addSoundControl(btn, btn.Parent.Location.Y);

                    addedESound.setData(SenderESound.SoundFile, SenderESound.Radius, SenderESound.Ramp, SenderESound.Volume);

                    addedESound.Tag = (Event)SenderESound.Tag;

                    ((Event)SenderESound.Tag).isDirty = true;

                    ((Event)SenderESound.Tag).parent.isDirty = true;

                    sound_tabPage.ResumeLayout();

                    sound_tabPage.ScrollControlIntoView(sound_tabPage.Controls[sound_tabPage.Controls.Count -1]);
                }
                else
                {
                    parentPanel.SuspendLayout();

                    addFxInputControls(parentPanel, textBoxWidth, btn.Left, controlsY);

                    parentPanel.ResumeLayout();
                }

                btn.Image = global::COH_CostumeUpdater.Properties.Resources.sub1;

                btn.Image.Tag = "sub";
            }
            TextBox tb = null;

            foreach(Control ctl in parentPanel.Controls)
            {
                if(ctl.GetType() == typeof(TextBox))
                {
                    tb = (TextBox) ctl;
                    break;
                }
            }

            inpName_txbxNew_Validated(tb, e);
        }

        private void sendToUltraEditToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode tn = this.fx_tv.SelectedNode;

            string path = null;

            if (tn != null)
            {
                switch(tn.Tag.GetType().Name)
                {
                    case "FX":
                        path = ((FX)tn.Tag).fxFileName;
                        break;

                    case "Behavior":
                        path = ((Behavior)tn.Tag).fileName;
                        break;

                    case "Partical":
                        path = ((Partical)tn.Tag).fileName;
                        break;
                }

                if(path != null && File.Exists(path))
                    System.Diagnostics.Process.Start(path);
            }
        }
        
        private void openInExplorerMenuItem_Click(object sender, EventArgs e)
        {
            ArrayList selection = new ArrayList();

            foreach (TreeNode tn in fx_tv.SelectedNodes)
            {
                string path = "";

                if (tn != null)
                {
                    switch (tn.Tag.GetType().Name)
                    {
                        case "FX":
                            path = ((FX)tn.Tag).fxFileName;
                            break;

                        case "Behavior":
                            path = ((Behavior)tn.Tag).fileName;
                            break;

                        case "Partical":
                            path = ((Partical)tn.Tag).fileName;
                            break;
                    }

                    if (path != null && path.Trim().Length > 0 && File.Exists(path))
                        selection.Add(path);
                }
            }

            string[] results = new string[selection.Count];
            int i = 0;
            foreach (string str in selection)
            {
                results[i++] = str;
            }

            common.ShowSelectedInExplorer.FilesOrFolders(results);    
        }
        
        private void renameMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode tn = (TreeNode)renameMenuItem.Tag;

            Partical part = (Partical)tn.Tag;

            string oldNamePath = part.fileName;

            string oldName = Path.GetFileName(oldNamePath);

            common.UserPrompt uInput = new COH_CostumeUpdater.common.UserPrompt("Partical rename", "Please enter New Partical Name");

            uInput.setTextBoxText(oldName);

            DialogResult uInputDr = uInput.ShowDialog();

            if (uInputDr == DialogResult.OK)
            {
                string partNewName = uInput.userInput.Trim();

                if (partNewName.Length > 1)
                {
                    partNewName = Path.GetFileName(partNewName);

                    bool renameSuccessful = part.renameFile(partNewName);

                    if (renameSuccessful)
                    {
                        FX fx = (FX)this.Tag;

                        foreach (Condition cond in fx.conditions)
                        {
                            foreach (Event ev in cond.events)
                            {
                                for (int i = 0; i < ev.eparts.Count; i++)
                                {
                                    string partName = (string)ev.eparts[i];

                                    if (partName.Contains(oldName))
                                    {
                                        ev.eparts[i] = ((string)ev.eparts[i]).Replace(oldName, partNewName);
                                    }
                                }
                            }
                        }

                        bhvrEvList.Clear();

                        partEvList.Clear();

                        this.fx_tv.BeginUpdate();

                        this.fx_tv.Nodes.Clear();

                        System.Windows.Forms.TreeNode tnNew = new System.Windows.Forms.TreeNode();

                        tnNew.Tag = fx;

                        tnNew.Name = "tN_" + tnNew.GetHashCode();

                        tnNew.Text = "fx";

                        tnNew.ImageIndex = fx.ImageIndex;

                        tnNew.Checked = true;

                        this.fx_tv.Nodes.Add(tnNew);

                        int cInd = 1;

                        foreach (Condition cond in fx.conditions)
                        {
                            fillConditionTree(cInd, cond, tnNew, true);

                            cInd++;
                        }                      

                        this.fx_tv.EndUpdate();

                        this.fx_tv.ExpandAll();

                        this.fx_tv.SelectedNode = fx_tv.Nodes[0];

                        setFxWinData();

                        this.pWin.initializeComboBx(partEvList, true);
                    }
                    else
                    {
                        MessageBox.Show("Rename Failed");
                    }
                }
            }
        }
        
        private void pasteBhvrOvrMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode evTN = null;

            Event evNode = null;

            if (localClipBoardObj.GetType().Name.Equals("Event"))
            {
                Event ev = (Event)((Event)localClipBoardObj).Clone();

                evTN = (TreeNode)pasteBhvrOvrMenuItem.Tag;

                evNode = (Event)evTN.Tag;

                bool fillBTree = evNode.eBhvrs.Count == 0 && evNode.eBhvrOverrides.Count == 0;

                evNode.eBhvrOverrideDic.Clear();

                evNode.eBhvrOverrides.Clear();

                foreach (object o in ev.eBhvrOverrides)
                {
                    evNode.eBhvrOverrides.Add(o);
                }

                evNode.buildBhvrOvrDic();

                if (fillBTree)
                    fillBhvrTree(evNode, evTN, ref bhvrNpart);

                if (!bhvrEvList.Contains(evNode))
                    bhvrEvList.Add(evNode);

                this.bWin.isDirty = false;

                this.bWin.initializeComboBx(bhvrEvList, false);

                //localClipBoardObj = null;

                //Clipboard.Clear();

                Point p = this.PointToScreen(Cursor.Position);

                fx_tv_MouseClick(this, new TreeNodeMouseClickEventArgs(evTN, MouseButtons.Left, 1, p.X, p.Y));

                this.bWin.isDirty = true;
            }
        }

        private void addBhvrOvrMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode evTN = (TreeNode)addBhvrOvrMenuItem.Tag;

            if (evTN.Tag.GetType().Name.Equals("Event"))
            {
                Event evNode = (Event)evTN.Tag;

                bool fillBTree = evNode.eBhvrs.Count == 0 && evNode.eBhvrOverrides.Count == 0;

                if (fillBTree)
                {
                    evNode.eBhvrOverrideDic.Clear();

                    evNode.eBhvrOverrides.Clear();

                    //add tmp override to build bhvr tree
                    evNode.eBhvrOverrides.Add(" ");

                    fillBhvrTree(evNode, evTN, ref bhvrNpart);

                    evNode.eBhvrOverrides.Clear();

                    if (!bhvrEvList.Contains(evNode))
                        bhvrEvList.Add(evNode);

                    this.bWin.initializeComboBx(bhvrEvList, false);

                    Point p = this.PointToScreen(Cursor.Position);

                    fx_tv_MouseClick(this, new TreeNodeMouseClickEventArgs(evTN, MouseButtons.Left, 1, p.X, p.Y));
                }
                else
                {
                    MessageBox.Show("The Event Node has a Bhvroverride.\r\nno action was performed");
                }
            }
        }

        private void moveUpMenuItem_Click(object sender, EventArgs e)
        {
            FX fx = (FX)fx_tv.Nodes[0].Tag;

            TreeNode evTN = (TreeNode)moveUpMenuItem.Tag;

            switch (evTN.Tag.GetType().Name)
            {
                case "Condition":
                    moveCodition(evTN, true);
                    mConditionWin.updateFXConditionsData();
                    mConditionWin.setConditionsData(fx);
                    mConditionWin.selectConditionRow((Condition)evTN.Tag);
                    break;

                case "Event":
                    moveEvent(evTN, true);
                    mEventWin.updateFXEventsData();
                    mEventWin.setEventsData(fx);
                    mEventWin.selectEventRow((Event)evTN.Tag);
                    break;

                case "Partical":
                    movePart(evTN, true);
                    break;
            }
        }

        private void moveDwnMenuItem_Click(object sender, EventArgs e)
        {
            FX fx = (FX)fx_tv.Nodes[0].Tag;

            TreeNode evTN = (TreeNode)moveDwnMenuItem.Tag;

            switch (evTN.Tag.GetType().Name)
            {
                case "Condition":
                    moveCodition(evTN, false);
                    mConditionWin.updateFXConditionsData();
                    mConditionWin.setConditionsData(fx);
                    mConditionWin.selectConditionRow((Condition)evTN.Tag);
                    break;

                case "Event":
                    moveEvent(evTN, false);
                    mEventWin.updateFXEventsData();
                    mEventWin.setEventsData(fx);
                    mEventWin.selectEventRow((Event)evTN.Tag);
                    break;

                case "Partical":
                    movePart(evTN, false);
                    break;
            }
        }

        private void moveCodition(TreeNode conditionTN, bool up)
        {
            int movDir = up ? -1 : 1;

            Condition cond = (Condition)conditionTN.Tag;

            FX fx = (FX)cond.parent;

            int eventIndex = fx.conditions.IndexOf(cond);

            int newPosInd = eventIndex + movDir;

            if (newPosInd > fx.conditions.Count - 1)
                newPosInd = 0;

            else if (newPosInd < 0)
                newPosInd = fx.conditions.Count - 1;

            TreeNode tn = null;

            TreeNode fxTN = conditionTN.Parent;

            if (fx != null)
            {
                fx.conditions.Remove(cond);

                fx.conditions.Insert(newPosInd, cond);

                this.fx_tv.BeginUpdate();

                fxTN.Nodes.Clear();

                int cInd = 1;

                bhvrEvList.Clear();
                partEvList.Clear();

                foreach (Condition con in fx.conditions)
                {

                    fillConditionTree(cInd, con, fxTN, true);

                    cInd++;
                }

                this.bWin.initializeComboBx(bhvrEvList, true);

                this.pWin.initializeComboBx(partEvList, true);

                this.fx_tv.EndUpdate();

                Point p = this.PointToScreen(Cursor.Position);

                tn = fxTN.Nodes[newPosInd];

                if (tn == null)
                    tn = fxTN;

                fx_tv_MouseClick(this, new TreeNodeMouseClickEventArgs(tn, MouseButtons.Left, 1, p.X, p.Y));
            }
        }

        private void moveEvent(TreeNode evTN, bool up)
        {
            int movDir = up ? -1 : 1;

            Event ev = (Event)evTN.Tag;

            Condition cond = (Condition)ev.parent;

            int eventIndex = cond.events.IndexOf(ev);

            int newPosInd = eventIndex + movDir;

            if (newPosInd > cond.events.Count - 1)
                newPosInd = 0;

            else if (newPosInd < 0)
                newPosInd = cond.events.Count - 1;

            TreeNode tn = null;

            TreeNode condTN = evTN.Parent;

            if (cond != null)
            {
                cond.events.Remove(ev);

                cond.events.Insert(newPosInd, ev);

                this.fx_tv.BeginUpdate();

                condTN.Nodes.Clear();

                int eInd = 1;

                foreach (Event evn in cond.events)
                {
                    bhvrEvList.Remove(evn);

                    partEvList.Remove(evn);

                    fillEventTree(eInd, evn, condTN, true, true);

                    eInd++;
                }

                this.bWin.initializeComboBx(bhvrEvList, true);

                this.pWin.initializeComboBx(partEvList, true);

                this.fx_tv.EndUpdate();

                Point p = this.PointToScreen(Cursor.Position);

                tn = condTN.Nodes[newPosInd];

                if (tn == null)
                    tn = condTN;

                fx_tv_MouseClick(tn, new TreeNodeMouseClickEventArgs(tn, MouseButtons.Left, 1, p.X, p.Y));
            }
        }

        private void movePart(TreeNode evTN, bool up)
        {
            Event ev = (Event)(evTN.Parent).Tag;
            
            int bhvrAdjustment = evTN.Parent.Nodes[0].Text.StartsWith("Bhvr ") ? 1 : 0;

            int partNodesMax = evTN.Parent.Nodes.Count - 1;

            int pInd = evTN.Parent.Nodes.IndexOf(evTN);

            int partIndex = pInd - bhvrAdjustment;

            int movDir = up ? -1 : 1;

            int pNodeNewInd = pInd + movDir;

            if (pNodeNewInd > partNodesMax)
            {
                pNodeNewInd = bhvrAdjustment;
            }
            else if (pNodeNewInd < bhvrAdjustment)
            {
                pNodeNewInd = partNodesMax;
            }
            int newPosInd = partIndex + movDir;

            if (newPosInd > ev.eparts.Count - 1)
                newPosInd = 0;

            else if (newPosInd < 0)
                newPosInd = ev.eparts.Count - 1;

            TreeNode tn = null;            
            
            evTN = evTN.Parent;

            if (ev != null && ev.eparts.Count > 1)
            {
                this.fx_tv.BeginUpdate();

                evTN.Nodes.Clear();

                ev.eBhvrsObject.Clear();

                ev.epartsObject.Clear();

                string path = (string)ev.eparts[partIndex];

                ev.eparts.RemoveAt(partIndex);

                ev.eparts.Insert(newPosInd, path);

                fillBhvrTree(ev, evTN, ref bhvrNpart);

                fillPartTree(ev, evTN, ref bhvrNpart);
                                
                this.pWin.initializeComboBx(partEvList, true);

                this.fx_tv.EndUpdate();

                Point p = this.PointToScreen(Cursor.Position);

                tn = evTN.Nodes[pNodeNewInd];

                if (tn == null)
                    tn = evTN;

                fx_tv_MouseClick(this, new TreeNodeMouseClickEventArgs(tn, MouseButtons.Left, 1, p.X, p.Y));
            }
        }
        
        private object getDataObject(IDataObject iData)
        {
            if (iData.GetDataPresent("FX"))
            {                
                return (FX)iData.GetData("FX", false);
            }
            else if (iData.GetDataPresent("Condition"))
            {
                return (Condition)iData.GetData("Condition", false);
            }
            else if (iData.GetDataPresent("Event"))
            {
                return (Event)iData.GetData("Event", false);
            }
            else if (iData.GetDataPresent("Behavior"))
            {
                return (Behavior)iData.GetData("Behavior", false);
            }
            else if (iData.GetDataPresent("Partical"))
            {
                return (Partical)iData.GetData("Partical", false);
            }
            else if (iData.GetDataPresent("ArrayList"))
            {
                return (ArrayList)iData.GetData("ArrayList", false);
            }
            return null;
        }

        private void copyMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode tn = (TreeNode)copyMenuItem.Tag;

            DataObject fxDataObject = new DataObject();

            if (fx_tv.SelectedNodes.Count > 1)
            {
                fxDataObject.SetData("ArrayList", false, fx_tv.SelectedNodes);
            }
            else
            {
                fxDataObject.SetData(tn.Tag.GetType().Name, false, tn.Tag);
            }

            Clipboard.Clear();

            common.COH_IO.SetClipBoardDataObject(fxDataObject);
        }

        private void pasteMenuItem_Click(object sender, EventArgs e)
        {
            //localClipBoardObj gets filled in showContextMenu function to allow for copying across
            //multiple windows and to test for showing pasteMenuItem
            if (localClipBoardObj != null)
            {

                if (localClipBoardObj.GetType().Name.Equals("ArrayList"))
                {
                    TreeNode pTn = (TreeNode)pasteMenuItem.Tag;

                    foreach (Object lClipBoardObj in (ArrayList)localClipBoardObj)
                    {
                        TreeNode tn = (TreeNode)lClipBoardObj;

                        pasteMenuItem.Tag = pTn;

                        if(tn.Tag!=null)
                            processPaset(tn.Tag);
                    }
                }
                else
                    processPaset(localClipBoardObj);


                this.bWin.initializeComboBx(bhvrEvList, false);

                this.pWin.initializeComboBx(partEvList, false);
            }
        }

        private void processPaset(Object lClipBoardObj)
        {
            FX fx = (FX)fx_tv.Nodes[0].Tag;

            string path = null;

            TreeNode evTN = null;

            Event evNode = null;

            switch (lClipBoardObj.GetType().Name)
            {
                case "Condition":
                    Condition condition = (Condition)((Condition)lClipBoardObj).Clone();

                    TreeNode fxTN = (TreeNode)pasteMenuItem.Tag;

                    if (fxTN.Tag.GetType().Name.Equals("FX") && condition != null)
                    {
                        condition.parent = fx;

                        int cInd = fx.conditions.Count + 1;

                        fx.conditions.Add(condition);

                        foreach (Event cEvent in condition.events)
                        {
                            cEvent.eName = null;
                        }

                        fillConditionTree(cInd, condition, fxTN, true);

                        mConditionWin.setCondition(condition);

                        mEventWin.updateFXEventsData();

                        mEventWin.setEventsData(fx);
                    }
                    break;

                case "Event":
                    Event ev = (Event)((Event)lClipBoardObj).Clone();

                    TreeNode condTN = (TreeNode)pasteMenuItem.Tag;

                    if (condTN.Tag.GetType().Name.Equals("Condition") && ev != null)
                    {
                        Condition cond = (Condition)condTN.Tag;

                        ev.parent = cond;

                        ev.eName = null;

                        int eInd = cond.events.Count + 1;

                        cond.events.Add(ev);

                        fillEventTree(eInd, ev, condTN, true, true);

                        mEventWin.updateFXEventsData();

                        mEventWin.setEventsData(fx);
                    }
                    break;

                case "Behavior":
                    string bhvrFn = ((Behavior)lClipBoardObj).fileName;

                    string bhvrRootPath = common.COH_IO.getRootPath(bhvrFn);

                    path = bhvrFn.Substring(bhvrRootPath.Length + @"fx\".Length);

                    evTN = (TreeNode)pasteMenuItem.Tag;

                    evNode = (Event)evTN.Tag;

                    if (evTN != null &&
                        evTN.Tag.GetType().Name.Equals("Event") &&
                        path != null &&
                        evNode != null)
                    {
                        addBhvrEvent(evNode, evTN, path);
                    }
                    break;

                case "Partical":
                    string partFn = ((Partical)lClipBoardObj).fileName;

                    string partRootPath = common.COH_IO.getRootPath(partFn);

                    path = partFn.Substring(partRootPath.Length + @"fx\".Length);

                    evTN = (TreeNode)pasteMenuItem.Tag;

                    if (evTN != null &&
                        path != null)
                    {
                        addPartEvent(evTN, path);
                    }
                    break;
            }
        }

        private void deleteMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult dr = MessageBox.Show("Are you sure you want to delete selected Item?", "Warning", MessageBoxButtons.YesNo);
            if (dr == DialogResult.Yes)
            {
                TreeNode [] treeNodes = new TreeNode[fx_tv.SelectedNodes.Count];
                int t = 0;

                foreach (TreeNode tNode in fx_tv.SelectedNodes)
                {
                    treeNodes[t++] = tNode;
                }

                TreeNode parentOfLastTNode = ((TreeNode)fx_tv.SelectedNodes[fx_tv.SelectedNodes.Count - 1]).Parent;

                foreach (TreeNode tn in treeNodes)
                {
                    //TreeNode tn = (TreeNode)deleteMenuItem.Tag;

                    TreeNode pTn = tn.Parent;

                    Event ev = null;

                    bool deleteTN = true;

                    string fxPath = System.IO.Path.GetDirectoryName(fileName);
                    if (tn.Tag != null)
                    {
                        switch (tn.Tag.GetType().Name)
                        {
                            case "Condition":
                                FX fx = (FX)this.Tag;
                                Condition fxCond = (Condition)tn.Tag;
                                foreach (Event cEV in fxCond.events)
                                {
                                    bhvrEvList.Remove(cEV);
                                    partEvList.Remove(cEV);
                                }
                                fx.conditions.Remove(fxCond);
                                mConditionWin.setConditionsData(fx);
                                mEventWin.updateFXEventsData();
                                mEventWin.setEventsData(fx);
                                clearSoundPanle();
                                setFXsound(fx);
                                clearSplatPanle();
                                setSplatPanel(fx);
                                break;

                            case "Event":
                                Condition cond = (Condition)pTn.Tag;
                                fx = cond.parent;
                                ev = (Event)tn.Tag;
                                bhvrEvList.Remove(ev);
                                partEvList.Remove(ev);
                                cond.events.Remove(ev);
                                mEventWin.updateFXEventsData();
                                mEventWin.setEventsData(fx);
                                clearSoundPanle();
                                setFXsound(fx);
                                clearSplatPanle();
                                setSplatPanel(fx);
                                break;

                            case "Behavior":
                                deleteBhvr(tn);
                                ev = (Event)pTn.Tag;
                                deleteTN = ev.eBhvrOverrides.Count == 0;
                                break;

                            case "Partical":
                                ev = (Event)pTn.Tag;
                                Partical part = (Partical)tn.Tag;
                                string partFn = part.fileName;
                                string partRootPath = common.COH_IO.getRootPath(partFn);
                                string partStr = partFn.Substring(partRootPath.Length + @"fx\".Length);
                                string partPath = System.IO.Path.GetDirectoryName(partFn);
                                int epartsCount = ev.eparts.Count;
                                ev.eparts.Remove(partStr);
                                if (ev.eparts.Count == epartsCount &&
                                    fxPath.ToLower().Equals(partPath.ToLower()))
                                {
                                    partStr = ":" + System.IO.Path.GetFileName(partFn);
                                    ev.eparts.Remove(partStr);
                                }
                                ev.epartsObject.Remove(part);
                                if (ev.eparts.Count == 0)
                                    partEvList.Remove(ev);
                                break;
                        }
                    }
                    else
                    {
                        string tnName = tn.Name;

                        /*if(tnName.StartsWith("bhvrTn"))
                        {
                            deleteBhvr(tn);
                            ev = (Event)pTn.Tag;
                            deleteTN = ev.eBhvrOverrides.Count == 0;
                        }
                        else */
                        if(tnName.StartsWith("partTn"))
                        {
                            ev = (Event)pTn.Tag;
                            string partName = tn.Text.Substring("Part ".Length);

                            int epartsCount = ev.eparts.Count;
                            string partStr = "";

                            foreach (string str in ev.eparts)
                            {
                                if(str.ToLower().Contains(partName.ToLower()))
                                    partStr = str;
                            }

                            if(partStr.Trim().Length > 0)
                                ev.eparts.Remove(partStr);

                            if (ev.eparts.Count == 0)
                                partEvList.Remove(ev);
                        }
                    }

                    if (deleteTN)
                        pTn.Nodes.Remove(tn);

                    this.bWin.initializeComboBx(bhvrEvList, true);

                    this.pWin.initializeComboBx(partEvList, true);
                }

                Point p = this.PointToScreen(Cursor.Position);

                fx_tv_MouseClick(this, new TreeNodeMouseClickEventArgs(parentOfLastTNode, MouseButtons.Left, 1, p.X, p.Y));
            }
        }

        private void deleteSplatMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode tn = (TreeNode)deleteMenuItem.Tag;

            FX fx = (FX)this.Tag;

            Event ev = (Event)tn.Tag;

            DialogResult dr = MessageBox.Show("Are you sure you want to delete the Splat of selected Event", "Warning", MessageBoxButtons.YesNo);

            if (dr == DialogResult.Yes)
            {
                ev.eParameters["Splat"] = null;

                clearSplatPanle();

                setSplatPanel(fx);
            }
        }

        private void deleteBhvr(TreeNode tn)
        {
            TreeNode pTn = tn.Parent;

            Event ev = null;

            ev = (Event)pTn.Tag;

            Behavior bhvrObj = (Behavior)tn.Tag;

            string bhvrFn = bhvrObj.fileName;

            string bhvrRootPath = common.COH_IO.getRootPath(bhvrFn);

            string bhvrStr = bhvrFn.Substring(bhvrRootPath.Length + @"fx\".Length);

            ev.eBhvrs.Remove(bhvrStr);

            ev.eBhvrsObject.Remove(bhvrObj);

            if (ev.eBhvrOverrides.Count == 0 && ev.eBhvrs.Count == 0)
                bhvrEvList.Remove(ev);

            if (ev.eBhvrOverrides.Count > 0)
            {

                string bhvr = @"behaviors\null.bhvr";

                Behavior behavior = null;

                if (bhvrNpart.ContainsKey(bhvr))
                {
                    behavior = (Behavior)bhvrNpart[bhvr];
                }
                else
                {
                    behavior = buildBhvr(bhvr, ev);

                    if (behavior != null)
                    {
                        bhvrNpart[bhvr] = behavior;
                    }
                    else
                    {
                        tn.ForeColor = Color.Gray;

                        MessageBox.Show(bhvr + " File Not Found!");

                        tn.ToolTipText = "BHVR file not Found";
                    }
                }

                if (behavior != null)
                {
                    tn.Tag = behavior;

                    ev.eBhvrsObject.Add(tn.Tag);

                    tn.ImageIndex = behavior.ImageIndex;
                }

                tn.Text = "Bhvr " + Path.GetFileName(bhvr);

                tn.Checked = true;
            }
        }
        
        private void addConditionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode fxTN = (TreeNode)addConditionToolStripMenuItem.Tag;

            FX fx = (FX)fxTN.Tag;

            int eInd = fx.conditions.Count + 1;

            Condition cond = new Condition(fxTN.Index, fx, false);

            fx.conditions.Add(cond);

            fillConditionTree(eInd, cond, fxTN, false);

            mConditionWin.setCondition(cond);
        }
        
        private void addEventToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FX fx = (FX)fx_tv.Nodes[0].Tag;

            TreeNode [] treeNodes = new TreeNode[fx_tv.SelectedNodes.Count];

            int t = 0;

            foreach (TreeNode tNode in fx_tv.SelectedNodes)
            {
                treeNodes[t++] = tNode;
            }

            foreach (TreeNode condTN in treeNodes)
            {

                Condition cond = (Condition)condTN.Tag;

                int eInd = cond.events.Count + 1;

                Event ev = new Event(condTN.Index, cond, false);

                cond.events.Add(ev);

                fillEventTree(eInd, ev, condTN, false, false);
            }
            mEventWin.updateFXEventsData();
            mEventWin.setEventsData(fx);
        }

        private void addBhvrEvent(Event ev, TreeNode evTN, string path)
        {
            
            this.fx_tv.BeginUpdate();

            evTN.Nodes.Clear();

            //fillBhvrTree rebuilds eBhvrsObjects
            ev.eBhvrsObject.Clear();

            //fillPartTree rebuilds epartsObjects
            ev.epartsObject.Clear();

            //Max One Bhvr/event
            ev.eBhvrs.Clear();

            ev.eBhvrs.Add(path);

            fillBhvrTree(ev, evTN, ref bhvrNpart);

            fillPartTree(ev, evTN, ref bhvrNpart);

            int ind = bhvrEvList.IndexOf(ev);

            if (ind < 0)
                bhvrEvList.Add(ev);

            this.bWin.initializeComboBx(bhvrEvList, true);

            this.fx_tv.EndUpdate();

            Point p = this.PointToScreen(Cursor.Position);

            fx_tv_MouseClick(this, new TreeNodeMouseClickEventArgs(evTN, MouseButtons.Left, 1, p.X, p.Y));
                       
        }

        private void addBhvrToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();

            ofd.FileOk -= new System.ComponentModel.CancelEventHandler(ofd_FileOk);
            ofd.FileOk += new System.ComponentModel.CancelEventHandler(ofd_FileOk);

            ofd.DefaultExt = ".bhvr";

            ofd.CheckFileExists = true;

            ofd.CheckPathExists = true;

            string path = Path.GetDirectoryName(((FX)this.Tag).fxFileName);
            
            ofd.InitialDirectory = path;
            
            ofd.Filter = "Bhvr file (*.bhvr)|*.bhvr";
            
            DialogResult dr = ofd.ShowDialog();
            
            if (dr == DialogResult.OK)
            {
                if (ofd.FileName.ToLower().EndsWith(".bhvr"))
                {
                    string fxFileRootPath = common.COH_IO.getRootPath(path);

                    string bhvrRootPath = common.COH_IO.getRootPath(ofd.FileName);

                    if (bhvrRootPath.ToLower().Equals(fxFileRootPath.ToLower()))
                    {
                        path = ofd.FileName.Substring(bhvrRootPath.Length + @"fx\".Length);

                        if (path.Trim().Length > 0)
                        {
                            TreeNode[] treeNodes = new TreeNode[fx_tv.SelectedNodes.Count];
                            int t = 0;
                            foreach (TreeNode evTN in fx_tv.SelectedNodes)
                            {
                                treeNodes[t++] = evTN;
                            }
                            foreach (TreeNode evTN in treeNodes)
                            {
                                //TreeNode evTN = (TreeNode)addBhvrToolStripMenuItem.Tag;

                                Event ev = (Event)evTN.Tag;

                                if (ev != null)
                                {
                                    addBhvrEvent(ev, evTN, path);
                                }
                            }
                        }
                    }
                }
                else
                {
                    MessageBox.Show(string.Format("{0} is not a correct bhvr File!", Path.GetFileName(ofd.FileName)));
                }
            }
            ofd.FileOk -= new System.ComponentModel.CancelEventHandler(ofd_FileOk);
        }

        private void addMultiEventPart(string[] fileNames)
        {
            COH_CostumeUpdaterForm c = (COH_CostumeUpdaterForm)this.FindForm();

            ArrayList nonePartFiles = new ArrayList();

            TreeNode[] treeNodes = new TreeNode[fx_tv.SelectedNodes.Count];

            int t = 0;

            foreach (TreeNode tNode in fx_tv.SelectedNodes)
            {
                treeNodes[t++] = tNode;
            }

            settingComboBx = true;

            this.fx_tv.BeginUpdate();

            foreach (TreeNode evTN in treeNodes)
            {
                Event ev = (Event)evTN.Tag;
                
                if (ev != null)
                {
                    evTN.Nodes.Clear();

                    ev.eBhvrsObject.Clear();

                    ev.epartsObject.Clear();

                    int ind = partEvList.IndexOf(ev);

                    if (ind < 0)
                        partEvList.Add(ev);

                    foreach (string pFname in fileNames)
                    {
                        if (pFname.EndsWith(".part"))
                        {
                            string path = Path.GetDirectoryName(((FX)this.Tag).fxFileName);

                            string fxFileRootPath = common.COH_IO.getRootPath(path);

                            string partRootPath = common.COH_IO.getRootPath(pFname);

                            if (partRootPath.ToLower().Equals(fxFileRootPath.ToLower()))
                            {
                                path = pFname.Substring(partRootPath.Length + @"fx\".Length);

                                if (path.Trim().Length > 0)
                                {
                                    if (!path.Replace("/", @"\").Contains(@"\"))
                                        path = ":" + path;

                                    ev.eparts.Add(path);
                                }
                            }
                        }
                        else
                        {
                            nonePartFiles.Add(pFname);
                        }
                    }

                    fillBhvrTree(ev, evTN, ref bhvrNpart);

                    fillPartTree(ev, evTN, ref bhvrNpart);


                }
            }

            this.fx_tv.EndUpdate();

            settingComboBx = false;

            Point p = this.PointToScreen(Cursor.Position);

            this.pWin.initializeComboBx(partEvList, true);

            fx_tv_MouseClick(this, new TreeNodeMouseClickEventArgs(treeNodes[treeNodes.Length - 1], MouseButtons.Left, 1, p.X, p.Y));

            if (nonePartFiles.Count > 0)
            {
                string msg = "";

                foreach (string fname in nonePartFiles)
                {
                    msg += string.Format("{0} is not a correct Part File!\r\n", Path.GetFileName(fname));
                }

                MessageBox.Show(msg);
            }

        }

        private void addPartEvent(TreeNode evTN, string path)
        {
            Event ev = null;

            int partIndex = 0;

            TreeNode tn = null;

            switch (evTN.Tag.GetType().Name)
            {
                case "Event":
                    ev = (Event)evTN.Tag;
                    partIndex = ev.eparts.Count;
                    break;

                case "Partical":
                    ev = (Event)(evTN.Parent).Tag;
                    partIndex = evTN.Index + 1;
                    evTN = evTN.Parent;
                    break;
            }

            if (ev != null )//&& !ev.eparts.Contains(path))//Keetsie requested adding same part multiple times
            {
                this.fx_tv.BeginUpdate();

                evTN.Nodes.Clear();

                ev.eBhvrsObject.Clear();

                ev.epartsObject.Clear();

                if (ev.eparts.Count != 0 && partIndex > ev.eparts.Count - 1 )
                    partIndex = ev.eparts.Count - 1;

                ev.eparts.Insert(partIndex, path);

                fillBhvrTree(ev, evTN, ref bhvrNpart);

                fillPartTree(ev, evTN, ref bhvrNpart);

                int ind = partEvList.IndexOf(ev);

                if (ind < 0)
                    partEvList.Add(ev);

                this.pWin.initializeComboBx(partEvList, true);

                this.fx_tv.EndUpdate();

                Point p = this.PointToScreen(Cursor.Position);

                tn = evTN.Nodes[partIndex];

                if (tn == null)
                    tn = evTN;

                fx_tv_MouseClick(this, new TreeNodeMouseClickEventArgs(tn, MouseButtons.Left, 1, p.X, p.Y));
            }
        }

        private void addPartToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string path = Path.GetDirectoryName(((FX)this.Tag).fxFileName);

            OpenFileDialog ofd = new OpenFileDialog();

            ofd.FileOk -= new System.ComponentModel.CancelEventHandler(ofd_FileOk);

            ofd.FileOk += new System.ComponentModel.CancelEventHandler(ofd_FileOk);

            ofd.DefaultExt = ".part";

            ofd.CheckFileExists = true;

            ofd.CheckPathExists = true;

            ofd.InitialDirectory = path;

            ofd.Filter = "part file (*.part)|*.part";

            ofd.Multiselect = true;
                        
            DialogResult dr = ofd.ShowDialog();

            if (dr == DialogResult.OK)
            {
                if (fx_tv.SelectedNodes.Count > 1)
                    addMultiEventPart(ofd.FileNames);
                else
                {
                    ArrayList nonePartFiles = new ArrayList();

                    TreeNode evTN = (TreeNode)addPartToolStripMenuItem.Tag;

                    foreach (string file_name in ofd.FileNames)
                    {
                        //if (ofd.FileName.ToLower().EndsWith(".part"))
                        if (file_name.EndsWith(".part"))
                        {
                            string fxFileRootPath = common.COH_IO.getRootPath(path);

                            string partRootPath = common.COH_IO.getRootPath(file_name);

                            if (partRootPath.ToLower().Equals(fxFileRootPath.ToLower()))
                            {
                                string lPath = file_name.Substring(partRootPath.Length + @"fx\".Length);

                                if (lPath.Trim().Length > 0)
                                {
                                    if (!lPath.Replace("/", @"\").Contains(@"\"))
                                        lPath = ":" + lPath;

                                    addPartEvent(evTN, lPath);
                                }
                            }
                        }
                        else
                        {
                            nonePartFiles.Add(file_name);
                        }
                    }
                    
                    if (nonePartFiles.Count > 0)
                    {
                        string msg = "";

                        foreach (string fname in nonePartFiles)
                        {
                            msg += string.Format("{0} is not a correct Part File!\r\n", Path.GetFileName(fname));
                        }

                        MessageBox.Show(msg);
                    }
                }
            }

            ofd.FileOk -= new System.ComponentModel.CancelEventHandler(ofd_FileOk);
        }

        private void ofd_FileOk(object sender, System.ComponentModel.CancelEventArgs e)
        {
            OpenFileDialog ofd = (OpenFileDialog)sender;

            if (!ofd.FileName.ToLower().EndsWith(ofd.DefaultExt.ToLower()))
            {
                MessageBox.Show(string.Format("{0} must ends with {1}!", Path.GetFileName(ofd.FileName), ofd.DefaultExt));
                e.Cancel = true;
            }
        }

        private void addSoundToolStripMenuItem_Click(object sender, EventArgs e)
        {
            common.UserPrompt soundForm = new common.UserPrompt("Event Sound");

            Event ev = (Event)((TreeNode)addSoundToolStripMenuItem.Tag).Tag;

            FX fx = ev.parent.parent;

            EventSound evSound = new EventSound(this.rootPath);

            soundForm.hideTxBxNLable(ref evSound.sound_TxBx, evSound);

            evSound.hideBtn();

            DialogResult dr = soundForm.ShowDialog();

            if (dr == DialogResult.OK)
            {
                string inpVal = soundForm.userInput;

                if (inpVal.Trim().Length > 0)
                {
                    updatetFxEventSound();

                    ev.eSounds.Add(evSound.getData());

                    sound_tabPage.SuspendLayout();

                    foreach (EventSound eSound in sound_tabPage.Controls)
                    {
                        eSound.addSound_btn.Click -= new System.EventHandler(this.addControl_btn_Click);
                        eSound.addSound_btn.MouseHover -= new System.EventHandler(addInput_btn_MouseHover);
                        eSound.addSound_btn.MouseEnter -= new System.EventHandler(addInput_btn_MouseHover);
                        eSound.addSound_btn.KeyUp -= new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
                        eSound.addSound_btn.KeyDown -= new System.Windows.Forms.KeyEventHandler(addInput_btn_KeyUp);
                        eSound.Dispose();
                    }

                    sound_tabPage.Controls.Clear();

                    sound_tabPage.ResumeLayout(false);

                    setFXsound(fx);
                }
            }
        }
        
        private void addSplatToolStripMenuItem_Click(object sender, EventArgs e)
        {
            common.UserPrompt splatForm = new common.UserPrompt("Event Splat");

            Event ev = (Event)((TreeNode)addSplatToolStripMenuItem.Tag).Tag;

            FX fx = ev.parent.parent;

            SplatPanel sp = new SplatPanel(fx.fxFileName);
            
            splatForm.hideTxBxNLable(ref sp.textureNamePath_txBx, sp);

            DialogResult dr = splatForm.ShowDialog();
            
            if (dr == DialogResult.OK)
            {
                string inpVal = splatForm.userInput;

                if (inpVal.Trim().Length > 0)
                {
                    updatetFxEventSplat();

                    string tagPath = ((string) sp.textureNamePath_txBx.Tag).Trim();

                    if (tagPath !=null && !tgaFilesDictionary.ContainsKey(tagPath))
                    {
                        tgaFilesDictionary[tagPath] = System.IO.Path.GetFileName(tagPath);
                        tgaFilesDictionary = common.COH_IO.sortDictionaryKeys(tgaFilesDictionary);
                    }

                    ev.eParameters["Splat"] = inpVal;

                    splat_tabPage.SuspendLayout();

                    clearSplatPanle();

                    splat_tabPage.ResumeLayout(false);

                    setSplatPanel(fx);
                }
            }
        }
        
        private void addInputToolStripMenuItem_Click(object sender, EventArgs e)
        {
            common.UserPrompt userP = new common.UserPrompt("Fx Input", "Please Enter the desired FX Input name", "InpName:");
            
            DialogResult dr = userP.ShowDialog();
            
            if (dr == DialogResult.OK)
            {
                string inpVal = userP.userInput;

                if (inpVal.Trim().Length > 0)
                {
                    FX fx = (FX)this.Tag;

                    getFxInputControls(ref fx);

                    fx.inputs.Add("InpName " + inpVal.Trim());

                    fxInput_panel.SuspendLayout();

                    foreach (Control ctl in fxInput_panel.Controls)
                    {
                        ctl.Dispose();
                    }

                    fxInput_panel.Controls.Clear();

                    fxInput_panel.ResumeLayout(false);

                    setFxInputControls(fx);
                }
            }
        }

        private void enableCtl(object sender, EventArgs e)
        {
            fxPerformanceRadius_numBx.Enabled = fxPerformanceRadius_ckbx.Checked;
            fxLifeSpan_numBx.Enabled = fxLifeSpan_ckbx.Checked;
            fxFileAge_numBx.Enabled = fxFileAge_ckbx.Checked;

            fxOnForceRadius_numBx.Enabled = fxOnForceRadius_ckbx.Checked;
            fxAnimScale_numBx.Enabled = fxAnimScale_ckbx.Checked;

            fxClampMaxScaleX_numBx.Enabled = fxClampMaxScale_ckbx.Checked;
            fxClampMaxScaleZ_numBx.Enabled = fxClampMaxScale_ckbx.Checked;
            fxClampMaxScaleY_numBx.Enabled = fxClampMaxScale_ckbx.Checked;

            fxClampMinScaleZ_numBx.Enabled = fxClampMinScale_ckbx.Checked;
            fxClampMinScaleY_numBx.Enabled = fxClampMinScale_ckbx.Checked;
            fxClampMinScaleX_numBx.Enabled = fxClampMinScale_ckbx.Checked;

            fxLighting_numBx.Enabled = fxLighting_ckbx.Checked;
        }
    }
}
