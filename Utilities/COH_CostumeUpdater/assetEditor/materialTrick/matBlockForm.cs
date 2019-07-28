using System;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.assetEditor.materialTrick
{
    public partial class matBlockForm :
      Panel
      //Form
    {
        private ArrayList mats;
        private ArrayList fileContent;
        private bool useFallback;
        private bool updateMatListComboBx;
        private int bumpMap2Indx;
        private string fileName;
        private RichTextBox rTextBox;
        private Object selectdMatBtn;
        private ArrayList matTricksList;
        private ArrayList dataSanpShot01;
        private bool finishedLoading;

        private System.Collections.Generic.Dictionary<string, string> tgaFilesDictionary;

        public matBlockForm(string file_name, ref RichTextBox rtBx, ref ArrayList file_content, System.Collections.Generic.Dictionary<string, string> tga_files_dictionary)
        {
            finishedLoading = true;
            common.WatingIconAnim wia = new COH_CostumeUpdater.common.WatingIconAnim();
            
            wia.Show();

            Application.DoEvents();

            this.Cursor = Cursors.WaitCursor;
            
            selectdMatBtn = null;

            rTextBox = rtBx;

            bumpMap2Indx = -1;

            updateMatListComboBx = true;

            fileName = file_name;

            System.Windows.Forms.Application.DoEvents();

            InitializeComponent();

            System.Windows.Forms.Application.DoEvents();          

            matListCmboBx.Items.Clear();

            fileContent = (ArrayList)file_content.Clone();

            tgaFilesDictionary = tga_files_dictionary;

            if (matTricksList != null)

            {
                matTricksList.Clear();
            }

            System.Windows.Forms.Application.DoEvents();
           
            matTricksList = MatParser.parse(file_name, ref fileContent);
            
            foreach (object obj in matTricksList)
            {
                System.Windows.Forms.Application.DoEvents();
                MatTrick mt = (MatTrick) obj;
                if (!mt.deleted)
                {
                    buildMatBtn(mt.name, mt);
                }
            }

            if (matListCmboBx.Items.Count > 0)
            {
                System.Windows.Forms.Application.DoEvents();

                System.Windows.Forms.Application.DoEvents();

                buildMatProperties();

                matListCmboBx.SelectedItem = matListCmboBx.Items[0];

                matListCmboBx_SelectionChangeCommitted(matListCmboBx, new EventArgs());
            }
            else
            {
                string fName = System.IO.Path.GetFileName(fileName);

                string warning = string.Format("\"{0}\" is not a Material Trick File.\r\nIt dose not contain (Texture X_...End) block.\r\n", fName);
                
                //MessageBox.Show(warning);

                rTextBox.SelectionColor = Color.Red;
                
                rTextBox.SelectedText += warning;

                dupMatBtn.Enabled = false;

                deleteMatBtn.Enabled = false;

                renameMatBtn.Enabled = false;
            }

            System.Windows.Forms.Application.DoEvents();
            
            wia.Close();
            
            wia.Dispose();

            this.Cursor = Cursors.Arrow;
        }


        public bool isEmpty()
        {
            return !(matTricksList.Count > 0);
        }

        private string getRoot()
        {
            string root = @"C:\game\";

            if (this.fileName.ToLower().Contains("gamefix2"))
                root = @"C:\gamefix2\";

            else if (this.fileName.ToLower().Contains("gamefix"))
                root = @"C:\gamefix\";

            return root;
        }

        private void buildMatProperties()
        {
            this.SuspendLayout();

            this.matTrickPropertiesPanel.SuspendLayout();

            this.mats = new ArrayList();

            this.mats.Clear();

            this.matTrickPropertiesPanel.Controls.Clear();

            string[] fieldsList1 = {"Base1", 
                                    "Multiply1,Multiply Reflect", 
                                    "DualColor1", 
                                    "BumpMap1", 
                                    "AddGlow1, AddGlowMat2"};
 
            string[] fieldsList2 = {"Mask,Alpha"};

            string[] fieldsList3 = {"Base2", 
                                    "Multiply2,Multiply Reflect", 
                                    "DualColor2", 
                                    "BumpMap2" };

            string[] fbFieldsList = {"Base", 
                                    "BumpMap", 
                                    "Blend" };

            
            buildMatSec("SUM-MATERIAL 1", fieldsList1, false);
            
            System.Windows.Forms.Application.DoEvents();
           
            buildMatSec("---MASK---", fieldsList2, false);

            System.Windows.Forms.Application.DoEvents();

            buildMatSec("SUM-MATERIAL 2", fieldsList3, false);

            System.Windows.Forms.Application.DoEvents();

            buildMatSec("FALLBACK / LEGACY", fbFieldsList, true);

            System.Windows.Forms.Application.DoEvents();

            buildSpecLightCMapTntFlgSec();

            System.Windows.Forms.Application.DoEvents();

            setControlsLocationNtag();

            System.Windows.Forms.Application.DoEvents();

            this.matTrickPropertiesPanel.ResumeLayout(false);

            this.ResumeLayout(false);

            if(this.matTrickPropertiesPanel.Controls.Count>0)
                this.matTrickPropertiesPanel.ScrollControlIntoView(this.matTrickPropertiesPanel.Controls[0]);
        }

        private void setMatProperties(Button matTrickBtn)
        {
            finishedLoading = false;

            //Addison Delany requested to set multiply 1 defaulr to "c:\game\src\_texture_library\World\Filler\white.tga"
            // and to if fallback base and bump is empty fill them automatically with base1 and bump1 values without activating fall back checkbox
            this.SuspendLayout();

            matTrickPropertiesPanel.SuspendLayout();

            MatTrick mt = (MatTrick)matTrickBtn.Tag;

            ArrayList data = (ArrayList)mt.data;

            string base1Texture = "";
            string bump1Texture = "";
            string tPath = "";
            string name = "";
            bool isFallb = false;
            string multiply1DefaultPath = getRoot() + @"src\_texture_library\World\Filler\white.tga";

            string [] tgafileNames = tgaFilesDictionary.Values.ToArray();

            string [] tgafilePaths = tgaFilesDictionary.Keys.ToArray();

            foreach (object obj in this.mats)
            {
                switch(obj.GetType().ToString())
                {
                    case "COH_CostumeUpdater.assetEditor.aeCommon.MatBlock":
                        aeCommon.MatBlock mb = ((aeCommon.MatBlock)obj);
                        isFallb = mb.isFallBack;
                        mb.setData(ref data, tgafilePaths, tgafileNames);
                        tPath = mb.getTexturePath();
                        name = mb.getName();
                        if (name.ToLower().Equals("base1"))
                            base1Texture = tPath;

                        else if (name.ToLower().Equals("bumpmap1"))
                            bump1Texture = tPath;

                        else if (name.ToLower().Equals("multiply1") && !(tPath.Length > 0))
                            mb.setTexturePath(multiply1DefaultPath);

                        else if (name.ToLower().Equals("base") &&
                            isFallb && !(tPath.Length > 0))
                            mb.setTexturePath(base1Texture);

                        else if (name.ToLower().Equals("bumpmap") &&
                            isFallb && !(tPath.Length > 0))
                            mb.setTexturePath(bump1Texture);

                        break;

                    case "COH_CostumeUpdater.assetEditor.aeCommon.MatSpecLitCubeMapTintFlags":
                        ((aeCommon.MatSpecLitCubeMapTintFlags)obj).setData(data, tgafilePaths, tgafileNames);
                        break;

                    case "COH_CostumeUpdater.assetEditor.materialTrick.FB_GlobalOptionsPnl":
                        ((FB_GlobalOptionsPnl)obj).setData(data, tgaFilesDictionary);
                        break;
                }
            }
            string k = base1Texture;
            string m = bump1Texture;

            setUseFallbackCkBx(data);


            if (this.matTrickPropertiesPanel.Controls.Count > 0)
                this.matTrickPropertiesPanel.ScrollControlIntoView(this.matTrickPropertiesPanel.Controls[0]);

            matTrickPropertiesPanel.ResumeLayout(false);

            this.ResumeLayout(false);

            finishedLoading = true;
        }

        private void setUseFallbackCkBx(ArrayList data)
        {
            if (this.matTrickPropertiesPanel.Controls.Count > 0)
            {
                //reset useFallbackChBx
                Control[] ctls = this.matTrickPropertiesPanel.Controls.Find("useFallbackChBx", true);

                CheckBox uFBchbx = (CheckBox)ctls[0];

                uFBchbx.Checked = false;

                useFallbackChBx_CheckedChanged(uFBchbx, new EventArgs());

                foreach (object obj in data)
                {
                    string line = common.COH_IO.removeExtraSpaceBetweenWords(((string)obj).Replace("\t", " ")).Trim();

                    line = line.Replace("//", "#");

                    if (line.ToLower().StartsWith("UseFallback ".ToLower()))
                    {
                        string useFallback = line.Replace("UseFallback ", "");

                        string[] useFbSplit = useFallback.Split('#');

                        decimal useFB = 0;

                        if (Decimal.TryParse(useFbSplit[0], out useFB))
                        {
                            uFBchbx.Checked = useFB > 0 ? true : false;

                            useFallbackChBx_CheckedChanged(uFBchbx, new EventArgs());
                        }
                    }
                }
            }
        }
        
        private void initMatTrickListComboBox(string matName)
        {
            this.matListCmboBx.Items.Add(matName);
        }

        private void buildMatBtn(string matName, MatTrick mt)
        {
            int locX = 3, locY = 3;
            string btnName = matName + "_btn";
            
            if (matTrickListPanel.Controls.Count > 0)
            {
                Button lastMatBtn = (Button)matTrickListPanel.Controls[matTrickListPanel.Controls.Count - 1];
                locX = lastMatBtn.Right + 10;
            }
            
            Button btn = new Button();
            btn.Tag = mt;
            btn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrick;
            btn.Location = new System.Drawing.Point(locX, locY);
            btn.Name = btnName;
            btn.Size = new System.Drawing.Size(75, 75);
            btn.TabIndex = 0;
            btn.Text = matName;
            btn.ImageAlign = System.Drawing.ContentAlignment.TopCenter;
            btn.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
            btn.UseVisualStyleBackColor = true;
            btn.Click += new System.EventHandler(this.matTrickBtn_Click);

            this.matTrickListPanel.Controls.Add(btn);

            initMatTrickListComboBox(matName);
        }

        private void buildSpecLightCMapTntFlgSec()
        {
            int mWidth = 1002,  mHeight = 390;

            System.Drawing.Size gSize = new System.Drawing.Size(mWidth, mHeight);

            aeCommon.CollapsablePanel cp = new aeCommon.CollapsablePanel("Extra Setting");

            aeCommon.MatSpecLitCubeMapTintFlags mslctf = new COH_CostumeUpdater.assetEditor.aeCommon.MatSpecLitCubeMapTintFlags(openFileDialog1);

            cp.Add(mslctf);

            cp.Size = gSize;

            cp.fixHeight();

            this.matTrickPropertiesPanel.Controls.Add(cp);

            if (bumpMap2Indx > 0)
            {
                this.mats.Insert(bumpMap2Indx, mslctf);
            }
            else
                this.mats.Add(mslctf);
        }
        
        private void buildMatSec(string matSecName, string [] matFieldsList, bool isFallBackSec)
        {
            int mWidth = 970,
                mHeight = 80;

            FB_GlobalOptionsPnl fb_gopt = null;
            System.Drawing.Size gSize = new System.Drawing.Size(mWidth, mHeight);
            aeCommon.CollapsablePanel cp = new aeCommon.CollapsablePanel(matSecName);
            
            populateMatPanel(matFieldsList, cp, mHeight, gSize, isFallBackSec);
            
            if (isFallBackSec)
            {
                
                CheckBox useFallbackChBx = new CheckBox();
                useFallbackChBx.AutoSize = true;
                useFallbackChBx.Location = new System.Drawing.Point(10, 0);
                useFallbackChBx.Name = "useFallbackChBx";
                useFallbackChBx.Size = new System.Drawing.Size(88, 17);
                useFallbackChBx.TabIndex = 0;
                useFallbackChBx.Text = "Use Fallback";                
                useFallbackChBx.UseVisualStyleBackColor = true;
                useFallbackChBx.Checked = true;
                useFallbackChBx.CheckedChanged -= new EventHandler(useFallbackChBx_CheckedChanged);
                useFallbackChBx.CheckedChanged += new EventHandler(useFallbackChBx_CheckedChanged);
                cp.Add(useFallbackChBx);
                this.useFallback = true;
                
                fb_gopt = new FB_GlobalOptionsPnl();
                
                cp.Add(fb_gopt);

                cp.fixHeight();

                fb_gopt.Location = new Point(10, cp.Height - 150);
            }

            //populateMatPanel(matFieldsList, cp, mHeight, gSize, isFallBackSec);

            if (isFallBackSec)
            {                
                this.mats.Add(fb_gopt);
            }
            this.matTrickPropertiesPanel.Controls.Add(cp);
        }

        private void useFallbackChBx_CheckedChanged(object sender, EventArgs e)
        {
            Control[] ctls = this.matTrickPropertiesPanel.Controls.Find("FALLBACK___LEGACY_colPnl", false);
            aeCommon.CollapsablePanel cp = (aeCommon.CollapsablePanel)ctls[0];
            cp.setContainerEnable(((CheckBox)sender).Checked);
            this.useFallback = ((CheckBox)sender).Checked;
        }

        private void setControlsLocationNtag()
        {
            int len = this.matTrickPropertiesPanel.Controls.Count;

            this.matTrickPropertiesPanel.Controls[0].Location = new Point(0, 0);

            for (int i = 1; i < len; i++)
            {
                this.matTrickPropertiesPanel.Controls[i].Location = new Point(0, this.matTrickPropertiesPanel.Controls[i - 1].Bottom);
                this.matTrickPropertiesPanel.Controls[i - 1].Tag = this.matTrickPropertiesPanel.Controls[i];
            }

            this.matTrickPropertiesPanel.Controls[len-1].Tag = null;
        }

        private void populateMatPanel(string[] fieldsList, aeCommon.CollapsablePanel cp, int matBlckHeight, Size gSize, bool isFallbackSec)
        {
            aeCommon.MatBlock matBlck = null;
            int i = 0;
            foreach (string mat in fieldsList)
            {
                matBlck = buildMatBlock(ref i, matBlckHeight, mat, gSize, isFallbackSec);
                cp.Add(matBlck);
                mats.Add(matBlck);

                if (mat.Equals("BumpMap2"))
                    bumpMap2Indx = this.mats.Count;

            }
            cp.fixHeight();
        }

        private aeCommon.MatBlock buildMatBlock(ref int i,int mHeight, string mat, Size gSize, bool isFallbackSec)
        {
            int firstMatBlcLocation = isFallbackSec ? 15 : 12;//isFallbackSec ? 135 : 12;

            int vLoc = firstMatBlcLocation + (mHeight * (i++));

            Point matBLocation = new System.Drawing.Point(10, vLoc);

            string[] split = mat.Split(",".ToCharArray());
            string matName = split[0] + "Gbx";
            string matText = split[0];
            string extraOpName = split.Length > 1 ? split[1] : "noExtra";
            bool useExtraChBx = split.Length > 1 ? true : false;
            aeCommon.MatBlock matBlck = new aeCommon.MatBlock(matName, matText, matBLocation, openFileDialog1, 
                                                              useExtraChBx, extraOpName, isFallbackSec);
            matBlck.Size = gSize;


            return matBlck;
        }
        
        private void matListCmboBx_SelectionChangeCommitted(object sender, System.EventArgs e)
        {
            if (finishedLoading)
            {
                if (matListCmboBx.Items.Count > 0)
                {
                    string btnName = matListCmboBx.SelectedItem.ToString() + "_btn";
                    Control[] ctl = matTrickListPanel.Controls.Find(btnName, false);
                    if (ctl.Length > 0 && updateMatListComboBx)
                    {
                        Button btn = (Button)ctl[0];
                        if (selectdMatBtn != null)
                        {
                            deSelectMatBtn((Button)selectdMatBtn);
                        }
                        selectMatBtn(btn);
                        matTrickBtn_Click(btn, new EventArgs());
                    }
                }
            }
            else
            {
                MessageBox.Show("Sorry, still loading Mat data. Please wait :)");
            }

        }

        private void matTrickBtn_Click(object sender, EventArgs e)
        {
            if (finishedLoading)
            {
                Button oldButton = (Button)sender;

                Button bt = (Button)sender;

                if (selectdMatBtn != null)
                {
                    oldButton = (Button)selectdMatBtn;

                    if (!bt.Equals(oldButton))
                    {
                        deSelectMatBtn(oldButton);
                        saveCurrentMatData(oldButton);
                        selectMatBtn(bt);
                    }
                }
                else
                {
                    selectMatBtn(bt);
                }
            }
            else
            {
                MessageBox.Show("Sorry, still loading Mat data. Please wait :)");
            }
        }

        private void deSelectMatBtn(Button btn)
        {
            if (btn != null)
            {
                btn.BackColor = System.Drawing.SystemColors.Control;

                btn.UseVisualStyleBackColor = true;

                Point pl = btn.Location;

                btn.Location = new Point(pl.X + 2, pl.Y + 2);

                btn.Size = new Size(75, 75);
            }
        }

        private void selectMatBtn(Button btn)
        {
            if (btn.Visible)
            {
                Color clr = Color.LightYellow;

                Point p = btn.Location;

                updateMatListComboBx = false;

                btn.Parent.SuspendLayout();

                btn.BackColor = clr;

                btn.Location = new Point(p.X - 2, p.Y - 2);

                btn.Size = new Size(80, 80);

                int i = matListCmboBx.Items.IndexOf(btn.Name.Replace("_btn", ""));

                matListCmboBx.SelectedItem = matListCmboBx.Items[i];

                selectdMatBtn = btn;

                btn.Parent.ResumeLayout(false);

                ((Panel)btn.Parent).ScrollControlIntoView(btn);

                updateMatListComboBx = true;

                setMatProperties(btn);
            }
        }

        private void saveCurrentMatData(Button btn)
        {
            materialTrick.MatTrick mat = (materialTrick.MatTrick)btn.Tag;
            saveCurrentMatData(mat);
        }
        
        private void saveCurrentMatData(MatTrick mat)
        {
            if (finishedLoading)
                mat.data = getCurrentData(mat.name);
        }

        private ArrayList getCurrentData(string matName)
        {
            System.Collections.ArrayList data = new System.Collections.ArrayList();

            data.Clear();

            bool writeFallback = true;
            
            data.Add("Texture " + matName);

            foreach (object obj in mats)
            {
                if (obj.GetType() == typeof(aeCommon.MatBlock))
                {
                    aeCommon.MatBlock mb = (aeCommon.MatBlock)obj;

                    if (mb.isFallBack)
                    {
                        if (writeFallback)
                        {
                            writeFallback = false;
                            data.Add("\tFallback");
                            int useFallbackVal = this.useFallback ? 1 : 0;
                            data.Add(string.Format("\t\tUseFallback {0}", useFallbackVal));
                        }
                        mb.getData(ref data);
                    }
                    else
                    {
                        mb.getData(ref data);
                        data.Add("");
                    }
                    
                    
                }
                else if (obj.GetType() == typeof(FB_GlobalOptionsPnl))
                {
                    FB_GlobalOptionsPnl fbg = (FB_GlobalOptionsPnl)obj;
                    fbg.getData(ref data);
                }
                else if (obj.GetType() == typeof(aeCommon.MatSpecLitCubeMapTintFlags))
                {
                    aeCommon.MatSpecLitCubeMapTintFlags mslcmtf = (aeCommon.MatSpecLitCubeMapTintFlags)obj;
                    mslcmtf.getData(ref data);
                }
            }
            
            data.Add("\tEnd");

            data.Add("End");

            return data;
        }
        /*
        private void saveMatDataToFileContent(Button matBtn)
        {
            materialTrick.MatTrick matTrick = (materialTrick.MatTrick)matBtn.Tag;
            saveMatDataToFileContent(matTrick);
        }
        
        private void saveMatDataToFileContent(MatTrick matTrick)
        {
            int startInd = matTrick.startIndex;
            int endInd = matTrick.endIndex;

            fileContent.RemoveRange(startInd, matTrick.data.Count);
            fileContent.InsertRange(startInd, matTrick.data);
        }
        */
        private void commentDeletedMat()
        {
            foreach (object obj in matTricksList)
            {
                MatTrick mt = (MatTrick)obj;
                if (mt.deleted)
                {
                    for (int i = 0; i < mt.data.Count; i++)
                    {
                        mt.data[i] = string.Format("//{0}", (string)mt.data[i]);
                    }
                }
            }
        }

        private void reOrderMatButtons()
        {
            int locX = 3, locY = 3;
            matTrickListPanel.AutoScrollPosition = new Point(0, 0);
            matTrickListPanel.SuspendLayout();

            Button selBtn = (Button)selectdMatBtn;

            deSelectMatBtn(selBtn);

            foreach (object obj in matTrickListPanel.Controls)
            {
                Button btn = (Button)obj;
                MatTrick mt = (MatTrick)btn.Tag;
                if (!mt.deleted)
                {
                    btn.Location = new Point(locX, locY);
                    locX = btn.Right + 10;
                }
            }

            matTrickListPanel.ResumeLayout(false);

            if (selBtn != null)
                selectMatBtn((Button)selBtn);
        }

        private Button getFirstVisibleButton(Button btn)
        {
            Button nextMatButton = null;

            foreach( Control ctl in matTrickListPanel.Controls)
            {
                if (((Button)ctl).Visible)
                {
                    nextMatButton = (Button)ctl;
                    return nextMatButton;
                }
            }  

            return nextMatButton;
        }

        private MatTrick getNewMatTrick(ArrayList data, string fileName, string matName)
        {
            int mCount = matTricksList.Count;

            int startIndex = mCount > 0 ? ((MatTrick)matTricksList[mCount - 1]).endIndex + 5000 : 5000;

            int endIndex = startIndex + data.Count;

            MatTrick mt = new MatTrick(startIndex, endIndex, data, fileName, matName);

            matTricksList.Add(mt);

            buildMatBtn(mt.name, mt);

            reOrderMatButtons();

            matListCmboBx.SelectedItem = matListCmboBx.Items[matListCmboBx.Items.Count - 1];

            matListCmboBx_SelectionChangeCommitted(matListCmboBx, new EventArgs());

            return mt;
        }

        private void renameMat(ref Button matBtn, string newName)
        {
            string matName = matBtn.Text;

            MatTrick mt = (MatTrick)matBtn.Tag;

            mt.name = newName;

            matBtn.Text = newName;

            matBtn.Name = newName + "_btn";

            matListCmboBx.SuspendLayout();

            matListCmboBx.Items[matListCmboBx.Items.IndexOf(matName)] = newName;

            matListCmboBx.ResumeLayout(false);
        }

        public ArrayList getPreSaveData()
        {
            commentDeletedMat();

            if (selectdMatBtn != null)
            {
                saveCurrentMatData((Button)selectdMatBtn);
            }

            return matTricksList;
        }

        private void saveMatButton_Click(object sender, System.EventArgs e)
        {
            if (finishedLoading)
            {
                commentDeletedMat();

                if (selectdMatBtn != null)
                {
                    saveCurrentMatData((Button)selectdMatBtn);
                }

                ArrayList newFileContent = new ArrayList();

                MatTrick mtMA = (MatTrick)matTricksList[0];

                int sIndM1 = mtMA.startIndex;

                //copy original file content before first edited matTrick
                for (int j = 0; j < sIndM1; j++)
                {
                    newFileContent.Add(fileContent[j]);
                }

                //insert first edited matTrick after copied file content
                newFileContent.AddRange(mtMA.data);

                //go through and merge file content not in matTricl
                for (int i = 1; i < matTricksList.Count; i++)
                {
                    MatTrick mtM1 = (MatTrick)matTricksList[i - 1];

                    MatTrick mtM2 = (MatTrick)matTricksList[i];

                    int eIndM1 = mtM1.endIndex;

                    int sIndM2 = mtM2.startIndex;

                    for (int j = eIndM1 + 1; j < sIndM2 && j < fileContent.Count; j++)
                    {
                        newFileContent.Add(fileContent[j]);
                    }
                    if (sIndM2 >= fileContent.Count)
                    {
                        newFileContent.Add("");
                    }
                    newFileContent.AddRange(mtM2.data);
                }


                string results = assetsMangement.CohAssetMang.checkout(fileName);
                if (results.Contains("can't edit exclusive file"))
                {
                    MessageBox.Show(results);
                    rTextBox.SelectionColor = Color.Red;
                    rTextBox.SelectedText += results;
                }
                else
                {
                    common.COH_IO.writeDistFile(newFileContent, fileName);
                }
                rTextBox.Text += results;
            }
            else
            {
                MessageBox.Show("Still loading Mat Setting. Can't save!");
            }
        }

        private void deleteMatBtn_Click(object sender, System.EventArgs e)
        {
            if (selectdMatBtn != null)
            {
                Button matBtn = (Button)selectdMatBtn;

                string matName = matBtn.Text;

                string deleteWarning = string.Format("Are your sure you want to delete \"{0}\"?", matName);

                DialogResult dr = MessageBox.Show(  deleteWarning, "Warning...", 
                                                    MessageBoxButtons.YesNo, 
                                                    MessageBoxIcon.Warning, 
                                                    MessageBoxDefaultButton.Button2);

                if (dr == DialogResult.Yes)
                {
                    matBtn.Visible = false;

                    matListCmboBx.SuspendLayout();

                    matListCmboBx.Items.Remove(matName);

                    matListCmboBx.ResumeLayout(false);

                    MatTrick mt = (MatTrick)matBtn.Tag;

                    mt.deleted = true;

                    reOrderMatButtons();

                    Button nextMatButton = getFirstVisibleButton(matBtn);

                    if (nextMatButton != null)
                    {
                        selectMatBtn(nextMatButton);
                    }
                    else
                    {
                        selectdMatBtn = null;
                        this.matTrickPropertiesPanel.Controls.Clear();
                        deleteMatBtn.Enabled = false;
                        renameMatBtn.Enabled = false;
                        dupMatBtn.Enabled = false;
                        matListCmboBx.Text = "";
                        //saveMatButton.Enabled = false;

                    }
                }
            }            
        }

        private void renameMatBtn_Click(object sender, System.EventArgs e)
        {
            if (selectdMatBtn != null)
            { 
                Button matBtn = (Button)selectdMatBtn;

                string matName = matBtn.Text;

                DialogResult dr = DialogResult.Yes;

                if (((MatTrick)matBtn.Tag).startIndex < 5000)
                {
                    string deleteWarning = string.Format("Renaming \"{0}\" will break all references to this Material Trick!\n\nAre you sure you want to continue with this operation?", matName);

                    dr = MessageBox.Show(  deleteWarning, "Warning...", 
                                           MessageBoxButtons.YesNo, 
                                           MessageBoxIcon.Warning, 
                                           MessageBoxDefaultButton.Button2);
                }

                if (dr == DialogResult.Yes)
                {

                    common.UserPrompt uInput = new COH_CostumeUpdater.common.UserPrompt("Material Name", "Please enter New Material Name");

                    uInput.setTextBoxText(matName);

                    DialogResult uInputDr = uInput.ShowDialog();

                    if (uInputDr == DialogResult.OK)
                    {
                        string matNewName = uInput.userInput;

                        if (!matNewName.ToLower().StartsWith("X_".ToLower()))
                        {
                            matNewName = "X_" + matNewName;
                        }

                        renameMat(ref matBtn, matNewName);
                    }
                }
            }
        }

        private void dupMatBtn_Click(object sender, System.EventArgs e)
        {
            if (selectdMatBtn != null)
            {
                Button matBtn = (Button)selectdMatBtn;

                string matName = matBtn.Text;

                MatTrick mt = (MatTrick)matBtn.Tag;

                MatTrick newMat = getNewMatTrick(mt.data, mt.fileName, mt.name + "_COPY");

                renameMatBtn_Click(selectdMatBtn, new EventArgs());

                this.reOrderMatButtons();
            }

        }

        private void newMatBtn_Click(object sender, System.EventArgs e)
        {
            if (this.matTrickPropertiesPanel.Controls.Count > 0)
                this.matTrickPropertiesPanel.ScrollControlIntoView(this.matTrickPropertiesPanel.Controls[0]);

            common.UserPrompt uInput = new COH_CostumeUpdater.common.UserPrompt("Material Name", "Please enter New Material Name");

            DialogResult dr = uInput.ShowDialog();

            if (dr == DialogResult.OK && uInput.userInput.Trim().Length > 0)
            {
                if (tgaFilesDictionary == null)
                {
                    tgaFilesDictionary = common.COH_IO.getFilesDictionary(getRoot() + @"src\Texture_Library", "*.tga");
                }
                string matName = uInput.userInput.Trim().Replace(" ","_");

                if(!matName.ToLower().StartsWith("X_".ToLower()))
                {
                    matName = "X_" + matName;
                }

                buildMatProperties();

                ArrayList data = getCurrentData(matName);

                MatTrick mt = getNewMatTrick(data, fileName, matName);

                deleteMatBtn.Enabled = true;

                renameMatBtn.Enabled = true;

                dupMatBtn.Enabled = true;

                //saveMatButton.Enabled = true;

                reOrderMatButtons();
            }
        }

        private void matTrickPropertiesPanel_MouseEnter(object sender, System.EventArgs e)
        {
            matTrickPropertiesPanel.Focus();
        }
    }
}
