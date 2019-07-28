using System;
using System.Text.RegularExpressions;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.costume
{
    public partial class AddCostumePiece : Form
    {
        private string rootDir;
        private int mIndex;
        string[] uiDefaultLables = { "Id", "GeoName", "DisplayName", "Geo", "Fx", "Tex1", "Tex2", "Tag", "Key", "IsMask" };
        private ArrayList mElements;
        private bool isEdit;
        private bool saved;

        public AddCostumePiece()
        {
            this.InitializeComponent();
            this.saved = false;
        }

        public AddCostumePiece(string fileName, ref ArrayList elements, int index)
        {
            isEdit = false;

            this.rootDir = @"C:\game";

            if(fileName.ToLower().Contains("gamefix2"))
                this.rootDir = @"C:\gamefix2";

            else if(fileName.ToLower().Contains("gamefix"))
                this.rootDir = @"C:\gamefix";


            this.mElements = elements;

            this.mIndex = index;

            this.extraCuiPanles = new System.Collections.ArrayList();
            
            InitializeComponent();

            this.dontShowWinCkbx.Hide();
            
            int i = 0;

            foreach (string m in uiDefaultLables)
            {
                addUserEnteryPanel(i++, m + extraCuiPanles.Count, m, "", m.Equals("Fx"));
            }
    
        }

        public AddCostumePiece(string fileName, ref ArrayList elements, TreeNode tn, int index, bool editMode, bool isPaste)
        {
            isEdit = editMode;

            this.Tag = (GeoPiece) tn.Tag;

            this.rootDir = @"C:\game";

            if (fileName.ToLower().Contains("gamefix2"))
                this.rootDir = @"C:\gamefix2";

            else if (fileName.ToLower().Contains("gamefix"))
                this.rootDir = @"C:\gamefix";

            this.mElements = elements;

            this.mIndex = index;
            
            this.extraCuiPanles = new System.Collections.ArrayList();

            InitializeComponent();

            this.dontShowWinCkbx.Hide();

            if (isEdit || isPaste)
            {

                int m, i = 0, nCnt = 0;

                string label = "";

                bool addIsMask = true;

                foreach (TreeNode n in tn.Nodes)
                {
                    string str = n.Text;

                    label = processUIStr(str, ref i, ref nCnt, addIsMask);
                 }

                m = findLabel(label.Replace("//","")) + 1;

                fillinMissingUIelements(i, addIsMask);

                sortUI();
            }
            else
            {
                int i = 0;

                foreach (string m in uiDefaultLables)
                {
                    addUserEnteryPanel(i++, m + extraCuiPanles.Count, m, "", m.Equals("Fx"));
                }
            }
        }

        public bool showOptionWindow(string fileName, ref ArrayList elements)
        {
            this.dontShowWinCkbx.Show();

            isEdit = false;

            this.rootDir = @"C:\game";

            if (fileName.ToLower().Contains("gamefix2"))
                this.rootDir = @"C:\gamefix2";

            else if (fileName.ToLower().Contains("gamefix"))
                this.rootDir = @"C:\gamefix";

            this.mElements = elements;

            this.extraCuiPanles = new System.Collections.ArrayList();

            int nCnt = 0, i = 0;

            string label = "";

            bool addIsMask = true;

            foreach (object obj in mElements)
            {
                string str = (string)obj;

                str = common.COH_IO.stripWS_Tabs_newLine_Qts(str, false);

                if (str.ToLower().Contains("info") ||
                    str.ToLower().Contains("end"))
                {
                    continue;
                }
                else
                {
                   label = processUIStr(str, ref i, ref nCnt, addIsMask);
                }
            }
                
            int m = findLabel(label.Replace("//", "")) + 1;
          
            fillinMissingUIelements(i, addIsMask);

            sortUI();

            ShowDialog();

            return dontShowWinCkbx.Checked;
        }

        public bool showAddWindow()
        {
            this.ShowDialog();
            return saved;
        }
        private string processUIStr(string str, ref int uiIncremental, ref int lastLableInd, bool addIsMask)
        {
            int m = 0;

            string label = "";

            string text = "";

            if (uiIncremental == 0 && !hasUIlabel(str))
            {
                label = "Id";
                text = str.Trim();
            }
            else
            {
                int wsLoc = str.IndexOf("\"");
                if (wsLoc < 0)
                    wsLoc = str.IndexOf(" ");

                label = str.Substring(0, wsLoc).Trim().Replace("\"", "");

                text = str.Substring(wsLoc, str.Length - wsLoc).Trim().Replace("\"", "");
            }
            if (!label.ToLower().Equals("ismask"))
            {
                addIsMask = false;
            }

            m = findLabel(label.Replace("//", ""));

            addUserEnteryPanel(uiIncremental++, label + extraCuiPanles.Count, uiDefaultLables[m], text, label.ToLower().Equals("fx"));

            return label;
        }

        private string processUIStr_old(string str, ref int uiIncremental, ref int lastLableInd, bool addIsMask)
        {
            int m = 0;

            string label = "";

            string text = "";

            if (uiIncremental == 0 && !hasUIlabel(str))
            {
                label = "Id";
                text = str.Trim();
            }
            else
            {
                int wsLoc = str.IndexOf("\"");
                if (wsLoc < 0)
                    wsLoc = str.IndexOf(" ");

                label = str.Substring(0, wsLoc).Trim().Replace("\"", "");

                text = str.Substring(wsLoc, str.Length - wsLoc).Trim().Replace("\"", "");
            }
            if (!label.ToLower().Equals("ismask"))
            {
                m = findLabel(label.Replace("//", ""));

                for (int c = lastLableInd; c < m; c++)
                {
                    addUserEnteryPanel(uiIncremental++, uiDefaultLables[c] + extraCuiPanles.Count, uiDefaultLables[c], "", uiDefaultLables[c].ToLower().Equals("fx"));
                }

                lastLableInd = m;
                lastLableInd++;
            }
            else
            {
                addIsMask = false;
            }
            addUserEnteryPanel(uiIncremental++, label + extraCuiPanles.Count, label, text, label.Equals("Fx"));

            return label;
        }

        private void fillinMissingUIelements(int UIincremental, bool addIsMask)
        {
            int i = UIincremental;

            for (int c = 0; c < uiDefaultLables.Length; c++)
            {
                string label = uiDefaultLables[c];

                string text = "";
                
                int uiIndex = getlastIndexOf(label);

                if (uiIndex == -1)
                {
                    if (!label.ToLower().Equals("ismask"))
                    {
                        addUserEnteryPanel(i++, label + extraCuiPanles.Count, label, text, label.Equals("Fx"));
                    }

                    else if (addIsMask)
                    {
                        addUserEnteryPanel(i++, label + extraCuiPanles.Count, label, text, label.Equals("Fx"));
                    }
                }
            }

        }
        private void fillinMissingUIelements_old(int lastExistingUI, int UIincremental, bool addIsMask)
        {
            int i = UIincremental;

            for (int c = lastExistingUI; c < uiDefaultLables.Length; c++)
            {
                string label = uiDefaultLables[c];

                string text = "";

                if (!label.ToLower().Equals("ismask"))
                {
                    addUserEnteryPanel(i++, label + extraCuiPanles.Count, label, text, label.Equals("Fx"));
                }

                else if (addIsMask)
                {
                    addUserEnteryPanel(i++, label + extraCuiPanles.Count, label, text, label.Equals("Fx"));
                }
            }

        }
        private bool hasUIlabel(string str)
        {
            foreach (string s in uiDefaultLables)
            {
                if (str.ToLower().Contains(s.ToLower()))
                {
                    return true;
                }
            }
            return false;
        }

        private int findLabel(string label)
        {
            for (int i = 0; i < uiDefaultLables.Length; i++)
            {
                if (uiDefaultLables[i].ToLower().Equals(label.ToLower()))
                    return i;
            }
            return -1;
        }

        private void sortUI()
        {
            ArrayList sortedCUI = new ArrayList();

            Control ct = this.Controls[0];

            int yLocation = ct.Bottom + 6;

            this.SuspendLayout();

            for (int c = 0; c < uiDefaultLables.Length; c++)
            {
                string label = uiDefaultLables[c];
                ArrayList ctrls = findUI(label);
                foreach (CostumeUserInput cui in ctrls)
                {
                    sortedCUI.Add(cui);
                    cui.Top = yLocation;
                    yLocation = cui.Bottom + 6;
                }

            }
            this.ResumeLayout(false);

            this.PerformLayout();

            extraCuiPanles.Clear();
            foreach (CostumeUserInput cUI in sortedCUI)
            {
                extraCuiPanles.Add(cUI);
            }
        }

        private ArrayList findUI(string kStr)
        {
            ArrayList uis = new ArrayList();
            for (int i = 0; i < extraCuiPanles.Count; i++)
            {
                CostumeUserInput cui = (CostumeUserInput)extraCuiPanles[i];
                if (cui.labelName.Equals(kStr))
                    uis.Add(cui);
            }
            return uis;
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!this.dontShowWinCkbx.Visible)
                saveGeoPieceToTool();
            else
            {
                this.mElements.Clear();
                readUIandPopulate(ref this.mElements);
                this.Close();
            }
        }

        private void fxFieldToolStripMenuItem_Click(object sender, EventArgs e)
        {
            
            addUserEnteryPanel(getlastIndexOf("Fx")+1, "Fx" + this.Controls.Count, "", "Fx", true);
            fixCUIorder();
        }

        private void keyFieldToolStripMenuItem_Click(object sender, EventArgs e)
        {
            addUserEnteryPanel(getlastIndexOf("Key")+1, "Key"+this.Controls.Count, "", "Key", false);
            fixCUIorder();
        }
        
        private int getlastIndexOf(string kStr)
        {
            int m = -1;
            for(int i=0; i < extraCuiPanles.Count; i++)
            {
                CostumeUserInput cui = (CostumeUserInput)extraCuiPanles[i];
                if (cui.labelName.Equals(kStr))
                    m = i;                   
            }
            return m;
        }

        private void addBtn_Click(object sender, EventArgs e)
        {
            Button bt = (Button)sender;

            string btTag = (string) bt.Tag;
            bt.Hide();
            addUserEnteryPanel(getlastIndexOf(btTag) + 1, btTag + this.Controls.Count, btTag, "", btTag.Contains("Fx"));
            fixCUIorder();
        }

        private void addUserEnteryPanel(int index, string panelName, string label, string text, bool isFx)
        {
            Control ct = this.Controls[this.Controls.Count - 1];
            
            bool hasAdd = false;
            
            int yLocation = ct.Bottom + 6;
            
            int height = this.ClientSize.Height;
            
            int width = this.ClientSize.Width;
            
            this.ClientSize = new System.Drawing.Size(width, height + 51);
            
            this.SuspendLayout();

            if (label.ToLower().Contains("key") || label.ToLower().Contains("fx"))
                hasAdd = true;

            CostumeUserInput cui = new CostumeUserInput(this.rootDir, panelName, label, yLocation, width, isFx, hasAdd, addBtn_Click);
            
            cui.Width = this.Width - 5;
            
            cui.Anchor = AnchorStyles.Top|AnchorStyles.Left|AnchorStyles.Right;
            
            cui.setCUITextBox(text);

            this.Controls.Add(cui);
            
            this.ResumeLayout(false);
            
            this.PerformLayout();

            if(index != -1)
                this.extraCuiPanles.Insert(index, cui);
            else
                this.extraCuiPanles.Add(cui);

            this.AddCostumePieceToolTip.SetToolTip(cui, "Enter " + label + " Name");
        }

        private void fixCUIorder()
        {
            this.SuspendLayout();

            Control ct = this.Controls[0];
            int yLocation = ct.Bottom + 6;

            foreach(object obj in extraCuiPanles)
            {                
            
                CostumeUserInput cui = (CostumeUserInput)obj;
                
                cui.Top = yLocation;
                
                yLocation = cui.Bottom + 6;
            }
                                    
            this.ResumeLayout(false);
            
            this.PerformLayout();
        }

        private string validateInput(string iStr, string lable)
        {
            //"!hdjksui6@#$ 5^&*()_!+=%jhd ;;ps,.?/\][poi\\<>
            string regStr = "";
            bool isExcl = iStr.StartsWith("!") || iStr.StartsWith("\"!");
            
            switch (lable)
            {
                
                case "Geo":
                    regStr = @"[^\w*]";
                    isExcl = false;
                    break;

                case "Key":
                    regStr = @"[^\w]";
                    isExcl = false;
                    break;

                case "Id":
                case "GeoName":
                case "DisplayName":
                    regStr = @"[^\w\s]";
                    isExcl = false;
                    break;

                case "Fx":
                    regStr = @"[^\w:\.\\/]";
                    break;

                default:
                    regStr = @"[^\w\./]";
                    break;
            }

            string vStr = Regex.Replace(iStr, regStr,"");
            vStr = isExcl ? ("!" + vStr) : vStr;
            //Regex reg = new Regex(@"^[a-zA-Z'.]{1,40}$");
            return vStr;
        }
       
        private void updateMElements(ArrayList elements)
        {
            bool isEnd = false;

            GeoPiece gp = (GeoPiece) this.Tag;

            int start = gp.getStartIndex();

            int last = gp.getLastIndex();

            int len = last - start + 1;

            if (isEdit)
            {
                this.mElements.RemoveRange(start, len);
            }

            else
            {
                start = getInsertionPos(start) ;
            }   

            string testStr = (string)mElements[start];
            
            testStr = common.COH_IO.stripWS_Tabs_newLine_Qts(testStr,false);


            if (testStr.ToLower().Contains("end"))
            {
                isEnd = true;
                start += 1;
                this.mElements.Insert(start++, "");
            }

            foreach (object obj in elements)
            {
                this.mElements.Insert(start++, obj);
            }
        }

        private int getInsertionPos(int start)
        {
            int i = start;

            bool isEnd = false;

            string testStr = (string)mElements[i];

            testStr = common.COH_IO.stripWS_Tabs_newLine_Qts(testStr, false);
           
            while (!isEnd && i < mElements.Count)
            {
                i++;

                testStr = (string)mElements[i];

                testStr = common.COH_IO.stripWS_Tabs_newLine_Qts(testStr, false);

                isEnd = testStr.ToLower().Contains("end");
            }

            return i;
        }

        private void saveGeoPieceToTool()
        {

            ArrayList newGeoPiece = new ArrayList();

            string results = readUIandPopulate(ref newGeoPiece);
 
            if (newGeoPiece.Count > 3)
            {
                updateMElements(newGeoPiece);
                saved = true;
                this.Close();
            }
            else
            {
                saved = false;
                MessageBox.Show("All Fields are empty. Nothing was saved");
            }
        }
    
        private string readUIandPopulate(ref ArrayList tArrayList)
        {
            string results = "\t\tInfo";

            string str = "";

            tArrayList.Add(results);

            foreach (object en in this.extraCuiPanles)
            {
                CostumeUserInput cui = (CostumeUserInput)en;

                string hQt = " \"";

                string tQt = "\"";

                string tbx = cui.getCUITextBox();

                string label = cui.labelName;

                tbx = validateInput(tbx, label);

                if (label.ToLower().Equals("id"))
                {
                    hQt = "";

                    tQt = "";

                    label = "";
                }
                else if (label.ToLower().Equals("ismask"))
                {
                    hQt = " ";
                    tQt = "";
                }
                if (tbx.Length > 0)
                {
                    str = "\t\t\t" + label + hQt + tbx + tQt;

                    tArrayList.Add(str);

                    results += str;
                }
            }

            str = "\t\tEnd";

            tArrayList.Add(str);

            results += str;

            return results;
        }
    }
}
