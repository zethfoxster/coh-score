using System;
using System.IO;
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
    public partial class BhvrNPartCommonFunctionPanel : Panel
        //Form
    {
        private OpenFileDialog openFileDialog1;
        
        private SaveFileDialog saveFileDialog1;
        
        private Form ownerForm;
        
        private Form floatingBhvrWin;
        
        private Control ownerParentControl;
        
        private Panel ownerDockablePanel;
        
        private string ownerName;
        
        private int width;
        
        private bool isfloating;
        
        public event EventHandler PartOrBhvrComboBoxChanged;
        
        public ArrayList bhvrStandAloneFile;
        
        public ArrayList partStandAloneFile;
        
        public object selectedBhvrOrPartObject;
        
        public bool settingCbx;
        
        public string fxFile;

        public BhvrNPartCommonFunctionPanel(Panel oDockableWin, string name, ImageList imglist)
        {
            width = 580;

            selectedBhvrOrPartObject = null;
            
            ownerForm = null;
            
            ownerParentControl = null;
            
            ownerDockablePanel = oDockableWin;
            
            ownerName = name;

            bhvrStandAloneFile = new ArrayList();
            
            partStandAloneFile = new ArrayList();

            floatingBhvrWin = new Form();
            
            floatingBhvrWin.Text = ownerName + " Editor: ";
            
            floatingBhvrWin.FormClosing += new FormClosingEventHandler(floatingBhvrWin_FormClosing);
            
            InitializeComponent();

            this.Bhvr4Event_label.Text = name + " for Event:";
            
            this.bhvr_label.Text = name + " file:";
            
            this.Width = width;
            
            initializeFileDialogs();

            this.eventsBhvrFilesMComboBx.ImageList = imglist;
        }

        private void initializeFileDialogs()
        {
            string dialogfilter = "all file: (*.*) | *.*";

            openFileDialog1 = new OpenFileDialog();
            saveFileDialog1 = new SaveFileDialog();

            switch (ownerName.ToLower())
            {
                case "part":
                    dialogfilter = "Particle file: (*.part) | *.part";
                    break;

                case "behavior":
                    dialogfilter = "Behavior file: (*.bhvr) | *.bhvr";
                    break;

                case "fx":
                    dialogfilter = "FX file: (*.fx) | *.fx";
                    break;
            }
            openFileDialog1.Filter = dialogfilter;
            saveFileDialog1.Filter = dialogfilter;
        }
        
        public void initializeComboBxs(ArrayList evList)
        {
            enableElements(true);

            this.eventsComboBx.Items.Clear();
            
            if (evList != null && evList.Count > 0)
            {
                int i = 0;

                string[] evComboList = new string[evList.Count];

                foreach (Event ev in evList)
                {
                    string eName = ev.eName != null ? ev.eName : "Event" + i;

                    if (ev.eParameters["EName"] != null)
                        eName = eName + ": " + (string)ev.eParameters["EName"];

                    evComboList[i++] = eName;
                }

                this.eventsComboBx.Items.AddRange(evComboList);

                this.eventsComboBx.Tag = evList;

                this.ownerDockablePanel.Enabled = true;
            }

            if (this.bhvrStandAloneFile.Count > 0 || this.partStandAloneFile.Count > 0)
            {
                if (!this.eventsComboBx.Items.Contains("StandAloneFiles"))
                    this.eventsComboBx.Items.Add("StandAloneFiles");
            }

            if (evList != null && 
                evList.Count == 0 && 
                bhvrStandAloneFile.Count == 0 && 
                partStandAloneFile.Count ==0)
            {
                this.eventsBhvrFilesMComboBx.Items.Clear();
                //this.ownerDockablePanel.Enabled = false;
                enableElements(false);
                this.eventsComboBx.Text = "";
            }
        }

        public void invokeSelectionChanged()
        {
            if (PartOrBhvrComboBoxChanged != null)
                PartOrBhvrComboBoxChanged(this, new EventArgs());
        }

        public void enableElements(bool enable)
        {
            eventsComboBx.Enabled = enable;
            eventsBhvrFilesMComboBx.Enabled = enable;

            floatWin_btn.Enabled = true; 
            new_btn.Enabled = true;
            load_btn.Enabled = true;

            save_btn.Enabled = enable;
            saveAs_btn.Enabled = enable;

            p4CheckOut_btn.Enabled = enable;
            p4CheckIn_btn.Enabled = enable;
            p4Add_btn.Enabled = enable;           
        }
        
        private void eventsBhvrFilesMComboBx_SelectedIndexChanged(object sender, System.EventArgs e)
        {            
            string fileName = null;
            
            int pOrB_index = eventsBhvrFilesMComboBx.SelectedIndex;

            int eIndex = eventsComboBx.SelectedIndex;

            if (eIndex > -1 && 
                eventsComboBx.Tag != null &&
                eIndex < ((ArrayList)eventsComboBx.Tag).Count)
            {
                Event ev = (Event)((ArrayList)eventsComboBx.Tag)[eIndex];
                
                if (ownerName.Equals("Behavior") &&
                    pOrB_index < ev.eBhvrsObject.Count)
                {
                    Behavior bhvr = (Behavior)ev.eBhvrsObject[pOrB_index];
                    
                    selectedBhvrOrPartObject = bhvr;

                    if (bhvr != null)
                    {
                        bhvr.Tag = ev;
                        
                        ((BehaviorWin)ownerDockablePanel).setBhvrPanels(bhvr);
                        
                        fileName = bhvr.fileName;
                        enableBtns(!Path.GetFileName(fileName).ToLower().Equals("null.bhvr"));
                    }
                }
                else if (ownerName.Equals("Part") &&
                         pOrB_index < ev.epartsObject.Count)
                {
                    Partical part = (Partical)ev.epartsObject[pOrB_index];

                    if (selectedBhvrOrPartObject != part)
                    {
                        selectedBhvrOrPartObject = part;

                        if (part != null)
                        {
                            part.Tag = ev;

                            ((ParticalsWin)ownerDockablePanel).setPartPanels(part);

                            fileName = part.fileName;
                        }
                    }
                }

                invokeSelectionChanged();

            }
            
            //stand alone file not part of fx file
            else
            {
                common.MComboBoxItem item = (common.MComboBoxItem)eventsBhvrFilesMComboBx.SelectedItem;

                if (ownerName.Equals("Behavior"))
                {
                    Behavior bhvr = (Behavior)item.Tag;
                    
                    selectedBhvrOrPartObject = bhvr;

                    if (bhvr != null)
                    {
                        ((BehaviorWin)ownerDockablePanel).setBhvrPanels(bhvr);
                        fileName = bhvr.fileName;
                    }
                }
                else
                {
                    Partical part = (Partical)item.Tag;

                    selectedBhvrOrPartObject = part;

                    if (part != null)
                    {
                        ((ParticalsWin)ownerDockablePanel).setPartPanels(part);
                        fileName = part.fileName;
                    }
                }
            }

            if (fileName != null)
            {
                FileInfo fi = new FileInfo(fileName);
                save_btn.Enabled = !((fi.Attributes & FileAttributes.ReadOnly) != 0);
            }
             
        }

        private void removeNullItem(ref ArrayList list)
        {
            if (list != null)
            {
                int i = 0;
                while (i < list.Count)
                {
                    if (list[i] == null)
                    {
                        list.RemoveAt(i);
                        i--;
                    }
                    i++;
                }
            }
        }

        private void eventsComboBx_SelectedIndexChanged(object sender, System.EventArgs e)
        {
            ArrayList eventList = (ArrayList)this.eventsComboBx.Tag;

            removeNullItem(ref eventList);

            Event tag = null;

            ArrayList tmpComboBxItemList = new ArrayList() ;

            COH_CostumeUpdater.common.MComboBoxItem[] comboBxItemList = null;

            int i = 0;

            if (eventsComboBx.SelectedIndex > -1 && 
                eventList != null &&
                eventsComboBx.SelectedIndex < eventList.Count)
            {
                Event ev = (Event)eventList[eventsComboBx.SelectedIndex];

                tag = ev;

                ArrayList partOrBhvrArrayList = ev.eparts;

                if (ownerName.Equals("Behavior"))
                {
                    partOrBhvrArrayList = ev.eBhvrs;

                    if (ev.eBhvrs.Count == 0)
                    {
                        partOrBhvrArrayList = new ArrayList();
                        partOrBhvrArrayList.Add(@"behaviors\null.bhvr");
                    }
                }

                removeNullItem(ref partOrBhvrArrayList);

                foreach (string partOrBhvr in partOrBhvrArrayList)
                {
                    string fName = System.IO.Path.GetFileName(partOrBhvr);

                    bool addToItemList = false;
                   
                    int imgInd = 0;

                    if (ownerName.Equals("Behavior") && 
                        i < ev.eBhvrsObject.Count && 
                        ev.isInBhvrObject(partOrBhvr))
                    {
                        imgInd = ((Behavior)ev.eBhvrsObject[i]).ImageIndex;
                        addToItemList = true;
                    }
                    else if (i < ev.epartsObject.Count && 
                        ev.isInPartObject(partOrBhvr))
                    {
                        imgInd = ((Partical)ev.epartsObject[i]).ImageIndex;
                        addToItemList = true;
                    }
                    if(addToItemList)
                        tmpComboBxItemList.Add(new COH_CostumeUpdater.common.MComboBoxItem(fName, imgInd, false, true, false, ev));
                }
            }

            else
            {
                ArrayList standAloneList = partStandAloneFile;

                if (ownerName.Equals("Behavior"))
                    standAloneList = bhvrStandAloneFile;

                comboBxItemList = new COH_CostumeUpdater.common.MComboBoxItem[standAloneList.Count];
                
                foreach (object obj in standAloneList)
                {
                    string file_name = null;

                    if (ownerName.Equals("Behavior"))
                        file_name = ((Behavior)obj).fileName;
                    else
                        file_name = ((Partical)obj).fileName;

                    string fName = System.IO.Path.GetFileName(file_name);

                    tmpComboBxItemList.Add(new COH_CostumeUpdater.common.MComboBoxItem(fName, 0, false, true, false, obj));
                }
            }

            comboBxItemList = new COH_CostumeUpdater.common.MComboBoxItem[tmpComboBxItemList.Count];

            int c = 0;

            foreach (COH_CostumeUpdater.common.MComboBoxItem mci in tmpComboBxItemList)
            {
                comboBxItemList[c++] = mci;
            }


            eventsBhvrFilesMComboBx.Items.Clear();

            if (comboBxItemList.Length > 0 && comboBxItemList[0] != null)
            {
                eventsBhvrFilesMComboBx.Items.AddRange(comboBxItemList);

                eventsBhvrFilesMComboBx.Tag = tag;

                if (comboBxItemList.Length > 0 && !settingCbx)
                    eventsBhvrFilesMComboBx.Text = comboBxItemList[0].Text;
            }

        }

        #region Common behavior and particals button functions
        
        private void enableBtns(bool enabel)
        {
            this.p4Add_btn.Enabled = enabel;
            this.p4CheckIn_btn.Enabled = enabel;
            this.p4CheckOut_btn.Enabled = enabel;
        }
        
        private void new_btn_Click(object sender, System.EventArgs e)
        {
            if (saveFileDialog1.InitialDirectory == null &&
                saveFileDialog1.FileName != null &&
                saveFileDialog1.FileName.ToLower().Contains(@"data\fx"))
            {
                saveFileDialog1.InitialDirectory = Path.GetDirectoryName(openFileDialog1.FileName);
            }
            else if (saveFileDialog1.InitialDirectory == null &&
                openFileDialog1.FileName != null &&
                openFileDialog1.FileName.ToLower().Contains(@"data\fx"))
            {
                saveFileDialog1.InitialDirectory = Path.GetDirectoryName(openFileDialog1.FileName);
            }
            if (!saveFileDialog1.InitialDirectory.ToLower().Contains(@"data\fx") && fxFile != null)
            {
                saveFileDialog1.InitialDirectory = Path.GetDirectoryName(fxFile);
            }

            if (eventsBhvrFilesMComboBx.Text != null)
            {
                string currentFileName = eventsBhvrFilesMComboBx.Text;

                saveFileDialog1.FileName = currentFileName;
            }

            DialogResult dr = saveFileDialog1.ShowDialog();

            if (dr == DialogResult.OK)
            {
                if (dr == DialogResult.OK)
                {
                    if (ownerName.Equals("Behavior"))
                    {
                        BehaviorWin bWin = (BehaviorWin)this.ownerDockablePanel;
                        bWin.setBhvrPanels(new Behavior("null.bhvr"));
                        common.COH_IO.writeDistFile(bWin.getData(), saveFileDialog1.FileName);
                        bWin.loadStandAloneBhvrFile(saveFileDialog1.FileName);
                    }
                    else
                    {
                        ParticalsWin pWin = (ParticalsWin)this.ownerDockablePanel;
                        pWin.setPartPanels(new Partical("null.part", false));
                        common.COH_IO.writeDistFile(pWin.getData(), saveFileDialog1.FileName);
                        pWin.loadStandAlonePartFile(saveFileDialog1.FileName);
                    }
                    enableElements(true);
                }
            }
        }

        private void load_btn_Click(object sender, System.EventArgs e)
        {
            if ((openFileDialog1.InitialDirectory == null || 
                !openFileDialog1.InitialDirectory.ToLower().Contains(@"data\fx")) &&
                openFileDialog1.FileName != null &&
                openFileDialog1.FileName.ToLower().Contains(@"data\fx"))
            {
                openFileDialog1.InitialDirectory = Path.GetDirectoryName(openFileDialog1.FileName);
            }

            else if (!openFileDialog1.InitialDirectory.ToLower().Contains(@"data\fx") && fxFile != null)
            {
                openFileDialog1.InitialDirectory = Path.GetDirectoryName(fxFile);
            }

            if (eventsBhvrFilesMComboBx.Text != null)
            {
                string currentFileName = eventsBhvrFilesMComboBx.Text;

                openFileDialog1.FileName = currentFileName;
            }

            DialogResult dr = openFileDialog1.ShowDialog();

            if (dr == DialogResult.OK)
            {
                if (ownerName.Equals("Behavior"))
                {
                    BehaviorWin bWin = (BehaviorWin)this.ownerDockablePanel;
                    bWin.loadStandAloneBhvrFile(openFileDialog1.FileName);
                    this.eventsComboBx.Enabled = true;
                    this.eventsBhvrFilesMComboBx.Enabled = true;
                    this.saveAs_btn.Enabled = true;
                }
                else
                {
                    ParticalsWin pWin = (ParticalsWin)this.ownerDockablePanel;
                    pWin.loadStandAlonePartFile(openFileDialog1.FileName);
                }

                openFileDialog1.InitialDirectory = Path.GetDirectoryName(openFileDialog1.FileName);
                
                eventsComboBx_SelectedIndexChanged(eventsComboBx, new System.EventArgs());
                
                if(eventsBhvrFilesMComboBx.Items.Count >0)
                    eventsBhvrFilesMComboBx.SelectedIndex = eventsBhvrFilesMComboBx.Items.Count - 1;
            }
        }

        private void save_btn_Click(object sender, System.EventArgs e)
        {
            string path = getComboBoxSelectedItemFilePath();

            FileInfo fi = new FileInfo(path);

            if ((fi.Attributes & FileAttributes.ReadOnly) != 0)
            {
                MessageBox.Show(string.Format("{0} is ReadOnly!\r\nDid you forget to check it out?", path),
                    "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            else
            {               
                saveFile(path);
                if(fxFile != null && ownerName.Equals("Behavior"))
                    FxLauncher.FXLauncherForm.runFx(fxFile, (COH_CostumeUpdaterForm) this.FindForm());
            }
        }

        private void saveAs_btn_Click(object sender, System.EventArgs e)
        {

            if ((saveFileDialog1.InitialDirectory == null ||
                !saveFileDialog1.InitialDirectory.ToLower().Contains(@"data\fx")) &&
                openFileDialog1.FileName != null &&
                openFileDialog1.FileName.ToLower().Contains(@"data\fx"))
            {
                saveFileDialog1.InitialDirectory = Path.GetDirectoryName(openFileDialog1.FileName);
            }

            else if (!saveFileDialog1.InitialDirectory.ToLower().Contains(@"data\fx")&& fxFile != null)
            {
                saveFileDialog1.InitialDirectory = Path.GetDirectoryName(fxFile);
            }
            
            if (eventsBhvrFilesMComboBx.Text != null)
            {
                string currentFileName = eventsBhvrFilesMComboBx.Text;

                saveFileDialog1.FileName = currentFileName;
            }

            DialogResult dr = saveFileDialog1.ShowDialog();

            if (dr == DialogResult.OK)
            {
                saveFile(saveFileDialog1.FileName);

                if (ownerName.Equals("Behavior"))
                    ((BehaviorWin)this.ownerDockablePanel).loadStandAloneBhvrFile(saveFileDialog1.FileName);
                else
                    ((ParticalsWin)this.ownerDockablePanel).loadStandAlonePartFile(saveFileDialog1.FileName);

                saveFileDialog1.InitialDirectory = Path.GetDirectoryName(saveFileDialog1.FileName);
            }
        }

        private void saveFile(string file_name)
        {
            if (ownerName.Equals("Behavior"))
            {
                BehaviorWin bWin = (BehaviorWin)this.ownerDockablePanel;

                ArrayList data = bWin.getData();

                common.COH_IO.writeDistFile(data, file_name);

                //updateBhvr(file_name, data);
            }
            else
            {
                ParticalsWin pWin = (ParticalsWin)this.ownerDockablePanel;

                ArrayList data = pWin.getData();

                common.COH_IO.writeDistFile(data, file_name);

                //updatePart(file_name, data);
            }
        }

        private void updateBhvr(string fileName, ArrayList data)
        {
            Behavior bhvr = BehaviorParser.parse(fileName, ref data);

            BehaviorWin bWin = (BehaviorWin)ownerDockablePanel;

            foreach (KeyValuePair<string, object> kvp in bhvr.bParameters)
            {
                bWin.BhvrTag.bParameters[kvp.Key] = kvp.Value;
            }

            bWin.BhvrTag.fileName = bhvr.fileName;

            bWin.BhvrTag.isCommented = bhvr.isCommented;

            bWin.BhvrTag.isDirty = bhvr.isDirty;

            bWin.BhvrTag.isP4 = bhvr.isP4;

            bWin.BhvrTag.isReadOnly = bhvr.isReadOnly;

            ((BehaviorWin)ownerDockablePanel).setIsDirty(false);
        }

        private void updatePart(string fileName, ArrayList data)
        {
            Partical part = ParticalParser.parse(fileName, ref data);

            ParticalsWin pWin = (ParticalsWin)ownerDockablePanel;

            foreach (KeyValuePair<string, object> kvp in part.pParameters)
            {
                pWin.Tag.pParameters[kvp.Key] = kvp.Value;
            }

            pWin.Tag.fileName = part.fileName;

            pWin.Tag.isCommented = part.isCommented;

            pWin.Tag.isDirty = part.isDirty;

            pWin.Tag.isP4 = part.isP4;

            pWin.Tag.isReadOnly = part.isReadOnly;

            ((ParticalsWin)ownerDockablePanel).setIsDirty(false);
        }

        private string getComboBoxSelectedItemFilePath()
        {
            string fileName = null;

            int pOrB_index = eventsBhvrFilesMComboBx.SelectedIndex;

            int eIndex = eventsComboBx.SelectedIndex;

            if (eIndex >-1 && 
                eventsComboBx.Tag != null &&
                eIndex < ((ArrayList)eventsComboBx.Tag).Count)
            {
                Event ev = (Event)((ArrayList)eventsComboBx.Tag)[eIndex];                

                if (ownerName.Equals("Behavior"))
                {
                    Behavior bhvr = (Behavior)ev.eBhvrsObject[pOrB_index];
                    if (bhvr != null)
                    {
                        fileName = bhvr.fileName;
                    }
                }
                else
                {
                    Partical part = (Partical)ev.epartsObject[pOrB_index];
                    if (part != null)
                    {
                        fileName = part.fileName;
                    }
                }
            }
            else
            {
                if (ownerName.Equals("Behavior"))
                {
                    Behavior bhvr = (Behavior)bhvrStandAloneFile[pOrB_index];
                    if (bhvr != null)
                    {
                        fileName = bhvr.fileName;
                    }
                }
                else
                {
                    Partical part = (Partical)partStandAloneFile[pOrB_index];
                    if (part != null)
                    {
                        fileName = part.fileName;
                    }
                }
            }
            return fileName;
        }

        private void setStatus()
        {
            if (ownerName.Equals("Behavior"))
            {
                BehaviorWin bWin = (BehaviorWin)ownerDockablePanel;
                
                Behavior bhvr = (Behavior)(bWin).BhvrTag;
                
                bhvr.refreshP4ROAttributes();
                
                save_btn.Enabled = !bhvr.isReadOnly;

                bool lockCtl = bhvr.isReadOnly;

                if (Path.GetFileName(bhvr.fileName).ToLower().Equals("null.bhvr")) 
                    lockCtl = false;


                bWin.enable(!lockCtl);
                
                bWin.isLocked = lockCtl;
            }
            else
            {
                ParticalsWin pWin = (ParticalsWin)ownerDockablePanel;
                
                Partical part = (Partical)(pWin).Tag;
                
                part.refreshP4ROAttributes();
                
                save_btn.Enabled = !part.isReadOnly;
                
                pWin.enable(!part.isReadOnly);
                
                pWin.isLocked = part.isReadOnly;
            }

            eventsBhvrFilesMComboBx_SelectedIndexChanged(eventsBhvrFilesMComboBx, new EventArgs());
        }

        private void p4Add_btn_Click(object sender, System.EventArgs e)
        {
            string path = getComboBoxSelectedItemFilePath();       
     
            string results = assetsMangement.CohAssetMang.addP4(path);

            MessageBox.Show(results);

            setStatus();
        }

        private void p4CheckOut_btn_Click(object sender, System.EventArgs e)
        {
            string path = getComboBoxSelectedItemFilePath();

            string results = assetsMangement.CohAssetMang.checkout(path);

            MessageBox.Show(results);

            setStatus();   
        }

        private void p4CheckIn_btn_Click(object sender, System.EventArgs e)
        {
            string path = getComboBoxSelectedItemFilePath();

            string results = assetsMangement.CohAssetMang.p4CheckIn(path);

            MessageBox.Show(results);

            if (ownerName.Equals("Behavior"))
            {
                ((BehaviorWin)ownerDockablePanel).setIsDirty(false);
            }
            else
            {
                ((ParticalsWin)ownerDockablePanel).setIsDirty(false);
            }

            setStatus();
        }
        
        private void floatWin_btn_Click(object sender, EventArgs e)
        {
            if (!isfloating)
            {
                floatDockedWindow();
            }
            else
            {
                dockFloatingWindow();
            }
        }

        private void floatDockedWindow()
        {
            if (ownerForm == null)
            {
                ownerForm = this.FindForm();
            }
            if (ownerParentControl == null)
            {
                ownerParentControl = this.Parent.Parent;
            }
            if (floatingBhvrWin == null || floatingBhvrWin.IsDisposed)
            {
                floatingBhvrWin = new Form();
                floatingBhvrWin.Text = ownerName + " Editor: ";
            }

            this.floatWin_btn.Text = "Dock Window";

            floatingBhvrWin.Size = ownerDockablePanel.Size;
            ownerForm.AddOwnedForm(floatingBhvrWin);
            ownerForm.Controls.Remove(ownerDockablePanel);
            floatingBhvrWin.FormClosing += new FormClosingEventHandler(floatingBhvrWin_FormClosing);
            floatingBhvrWin.Controls.Add(ownerDockablePanel);
            floatingBhvrWin.Show();
            //((SplitContainer)(ownerParentControl.Parent)).Panel2Collapsed = true;
            isfloating = !isfloating;
        }
        
        private void dockFloatingWindow()
        {
            floatWin_btn.Text = "Float Window";
            //((SplitContainer)(ownerParentControl.Parent)).Panel2Collapsed = false;
            ownerForm.SuspendLayout();
            ownerDockablePanel.SuspendLayout();
            floatingBhvrWin.SuspendLayout();
            floatingBhvrWin.Controls.Remove(ownerDockablePanel);
            ownerParentControl.Controls.Add(ownerDockablePanel);
            ownerDockablePanel.Dock = DockStyle.Fill;
            ownerDockablePanel.Size = ownerForm.Size;
            floatingBhvrWin.ResumeLayout();
            ownerDockablePanel.ResumeLayout();
            ownerForm.ResumeLayout();
            isfloating = !isfloating;
            floatingBhvrWin.Hide();
            ownerParentControl.Refresh();
            ownerForm.Refresh();
            Application.DoEvents();
        }

        private void floatingBhvrWin_FormClosing(object sender, FormClosingEventArgs e)
        {
            dockFloatingWindow();
            ((Form)sender).Dispose();
        }
        #endregion     
    }
}
