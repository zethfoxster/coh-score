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
    public partial class ObjectBlockForm :
      Panel
      //Form
    {
        private ArrayList objectTricks;
        private ArrayList fileContent;
        private bool updateObjectTrickListComboBx;
        private string fileName;
        private RichTextBox rTextBox;
        private Object selectdObjTrickBtn;
        private ArrayList objTricksList;
        private System.Collections.Generic.Dictionary<string, string> tgaFilesDictionary;
        private bool finishedLoading;


        public ObjectBlockForm(string file_name, ref RichTextBox rtBx, ref ArrayList file_content, System.Collections.Generic.Dictionary<string, string> tga_files_dictionary)
        {
            finishedLoading = true;

            common.WatingIconAnim wia = new COH_CostumeUpdater.common.WatingIconAnim();
            
            wia.Show();

            Application.DoEvents();

            this.Cursor = Cursors.WaitCursor;
            
            selectdObjTrickBtn = null;

            rTextBox = rtBx;

            updateObjectTrickListComboBx = true;

            fileName = file_name;

            System.Windows.Forms.Application.DoEvents();

            InitializeComponent();

            System.Windows.Forms.Application.DoEvents();          

            objListCmboBx.Items.Clear();

            fileContent = (ArrayList)file_content.Clone();

            tgaFilesDictionary = tga_files_dictionary;

            if (objTricksList != null)

            {
                objTricksList.Clear();
            }

            System.Windows.Forms.Application.DoEvents();
           
            objTricksList = ObjectTrickParser.parse(file_name, ref fileContent);
            
            foreach (object obj in objTricksList)
            {
                System.Windows.Forms.Application.DoEvents();
                ObjectTrick ot = (ObjectTrick) obj;
                if (!ot.deleted)
                {
                    buildMatBtn(ot.name, ot);
                }
            }

            if (objListCmboBx.Items.Count > 0)
            {
                System.Windows.Forms.Application.DoEvents();

                buildObjectTrickProperties();

                objListCmboBx.SelectedItem = objListCmboBx.Items[0];

                matListCmboBx_SelectionChangeCommitted(objListCmboBx, new EventArgs());
            }
            else
            {
                string fName = System.IO.Path.GetFileName(fileName);

                string warning = string.Format("\"{0}\" is not an Object Trick File.\r\nIt dose not contain (Trick ...End) block.\r\n", fName);
                
                //MessageBox.Show(warning);

                rTextBox.SelectionColor = Color.Red;
                
                rTextBox.SelectedText += warning;

                dupObjTrickBtn.Enabled = false;

                deleteObjTrickBtn.Enabled = false;

                renameObjTrickBtn.Enabled = false;
            }

            System.Windows.Forms.Application.DoEvents();
            
            wia.Close();
            
            wia.Dispose();

            this.Cursor = Cursors.Arrow;
        }

        public bool isEmpty()
        {
            return !(objTricksList.Count > 0);
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

        private void buildObjectTrickProperties()
        {
            this.SuspendLayout();

            this.objectTricks = new ArrayList();

            this.objectTricks.Clear();

            this.objTrickPropertiesPanel.Controls.Clear();

            buildObjTrickSec();

            System.Windows.Forms.Application.DoEvents();            

            System.Windows.Forms.Application.DoEvents();

            setControlsLocationNtag();

            System.Windows.Forms.Application.DoEvents();

            this.ResumeLayout(false);
        }

        private void setMatProperties(Button matTrickBtn)
        {
            finishedLoading = false;

            this.SuspendLayout();

            ObjectTrick mt = (ObjectTrick)matTrickBtn.Tag;

            ArrayList data = (ArrayList)mt.data;

            ObjectTrickPanel otPnl = (ObjectTrickPanel) this.objTrickPropertiesPanel.Controls[0];

            otPnl.setData(data);

            this.ResumeLayout(false);

            finishedLoading = true;
        }
        
        private void initMatTrickListComboBox(string matName)
        {
            this.objListCmboBx.Items.Add(matName);
        }

        private void buildMatBtn(string matName, ObjectTrick mt)
        {
            int locX = 3, locY = 3;
            string btnName = matName + "_btn";
            
            if (objTrickListPanel.Controls.Count > 0)
            {
                Button lastMatBtn = (Button)objTrickListPanel.Controls[objTrickListPanel.Controls.Count - 1];
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
            btn.Click += new System.EventHandler(this.ObjTrickkBtn_Click);

            this.objTrickListPanel.Controls.Add(btn);

            initMatTrickListComboBox(matName);
        }
        
        private void buildObjTrickSec()
        {
            string gameBranch = common.COH_IO.getRootPath(this.fileName);

            gameBranch = gameBranch.Substring(@"C:\".Length).Replace(@"\data\", "");

            ObjectTrickPanel otPnl = new COH_CostumeUpdater.assetEditor.objectTrick.ObjectTrickPanel(gameBranch);

            otPnl.Dock = DockStyle.Fill;

            this.objTrickPropertiesPanel.Controls.Add(otPnl);
        }

        private void setControlsLocationNtag()
        {
            int len = this.objTrickPropertiesPanel.Controls.Count;

            this.objTrickPropertiesPanel.Controls[0].Location = new Point(0, 0);

            for (int i = 1; i < len; i++)
            {
                this.objTrickPropertiesPanel.Controls[i].Location = new Point(0, this.objTrickPropertiesPanel.Controls[i - 1].Bottom);
                this.objTrickPropertiesPanel.Controls[i - 1].Tag = this.objTrickPropertiesPanel.Controls[i];
            }

            this.objTrickPropertiesPanel.Controls[len-1].Tag = null;
        }

        private void matListCmboBx_SelectionChangeCommitted(object sender, System.EventArgs e)
        {
            if (finishedLoading)
            {
                if (objListCmboBx.Items.Count > 0)
                {
                    string btnName = objListCmboBx.SelectedItem.ToString() + "_btn";
                    Control[] ctl = objTrickListPanel.Controls.Find(btnName, false);
                    if (ctl.Length > 0 && updateObjectTrickListComboBx)
                    {
                        Button btn = (Button)ctl[0];
                        if (selectdObjTrickBtn != null)
                        {
                            deSelectObjTrickBtn((Button)selectdObjTrickBtn);
                        }
                        selectObjTrickBtn(btn);
                        ObjTrickkBtn_Click(btn, new EventArgs());
                    }
                }
            }
            else
            {
                MessageBox.Show("Sorry, still loading Mat data. Please wait :)");
            }
        }

        private void ObjTrickkBtn_Click(object sender, EventArgs e)
        {
            if (finishedLoading)
            {
                Button oldButton = (Button)sender;

                Button bt = (Button)sender;

                if (selectdObjTrickBtn != null)
                {
                    oldButton = (Button)selectdObjTrickBtn;

                    if (!bt.Equals(oldButton))
                    {
                        deSelectObjTrickBtn(oldButton);
                        saveCurrentMatData(oldButton);
                        selectObjTrickBtn(bt);
                    }
                }
                else
                {
                    selectObjTrickBtn(bt);
                }
            }
            else
            {
                MessageBox.Show("Sorry, still loading Mat data. Please wait :)");
            }
        }

        private void deSelectObjTrickBtn(Button btn)
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

        private void selectObjTrickBtn(Button btn)
        {
            if (btn.Visible)
            {
                Color clr = Color.LightYellow;

                Point p = btn.Location;

                updateObjectTrickListComboBx = false;

                btn.Parent.SuspendLayout();

                btn.BackColor = clr;

                btn.Location = new Point(p.X - 2, p.Y - 2);

                btn.Size = new Size(80, 80);

                int i = objListCmboBx.Items.IndexOf(btn.Name.Replace("_btn", ""));

                objListCmboBx.SelectedItem = objListCmboBx.Items[i];

                selectdObjTrickBtn = btn;

                btn.Parent.ResumeLayout(false);

                ((Panel)btn.Parent).ScrollControlIntoView(btn);

                updateObjectTrickListComboBx = true;

                setMatProperties(btn);
            }
        }

        private void saveCurrentMatData(Button btn)
        {
            ObjectTrick ot = (ObjectTrick)btn.Tag;
            saveCurrentObjTrickData(ot);
        }

        private void saveCurrentObjTrickData(ObjectTrick ot)
        {
            if(finishedLoading)
                ot.data = getCurrentData(ot.name);
        }

        private ArrayList getCurrentData(string oTrickName)
        {
            System.Collections.ArrayList data = new System.Collections.ArrayList();

            data.Clear();

            data.Add("Trick  " + oTrickName);

            ObjectTrickPanel otPnl = (ObjectTrickPanel)this.objTrickPropertiesPanel.Controls[0];

            otPnl.getData(ref data);

            data.Add("End");

            return data;
        }

        private void saveObjTrickDataToFileContent(Button oTrickBtn)
        {
            ObjectTrick ObjectTrick = (ObjectTrick)oTrickBtn.Tag;
            saveObjTrickDataToFileContent(ObjectTrick);
        }

        private void saveObjTrickDataToFileContent(ObjectTrick objectTrick)
        {
            int startInd = objectTrick.startIndex;
            int endInd = objectTrick.endIndex;

            fileContent.RemoveRange(startInd, objectTrick.data.Count);
            fileContent.InsertRange(startInd, objectTrick.data);
        }

        private void commentDeletedObjTrick()
        {
            foreach (object obj in objTricksList)
            {
                ObjectTrick mt = (ObjectTrick)obj;
                if (mt.deleted)
                {
                    for (int i = 0; i < mt.data.Count; i++)
                    {
                        mt.data[i] = string.Format("//{0}", (string)mt.data[i]);
                    }
                }
            }
        }

        private void reOrderObjTrickButtons()
        {
            int locX = 3, locY = 3;
            objTrickListPanel.AutoScrollPosition = new Point(0, 0);
            objTrickListPanel.SuspendLayout();

            Button selBtn = (Button)selectdObjTrickBtn;

            deSelectObjTrickBtn(selBtn);

            foreach (object obj in objTrickListPanel.Controls)
            {
                Button btn = (Button)obj;
                ObjectTrick mt = (ObjectTrick)btn.Tag;
                if (!mt.deleted)
                {
                    btn.Location = new Point(locX, locY);
                    locX = btn.Right + 10;
                }
            }

            objTrickListPanel.ResumeLayout(false);

            if (selBtn != null)
                selectObjTrickBtn((Button)selBtn);
        }

        private Button getFirstVisibleButton(Button btn)
        {
            Button nextMatButton = null;

            foreach( Control ctl in objTrickListPanel.Controls)
            {
                if (((Button)ctl).Visible)
                {
                    nextMatButton = (Button)ctl;
                    return nextMatButton;
                }
            }  

            return nextMatButton;
        }

        private ObjectTrick getNewObjTrick(ArrayList data, string fileName, string oTrickName)
        {
            int mCount = objTricksList.Count;

            int startIndex = mCount > 0 ? ((ObjectTrick)objTricksList[mCount - 1]).endIndex + 5000 : 5000;

            int endIndex = startIndex + data.Count;

            ObjectTrick mt = new ObjectTrick(startIndex, endIndex, data, fileName, oTrickName);

            objTricksList.Add(mt);

            buildMatBtn(mt.name, mt);

            reOrderObjTrickButtons();

            objListCmboBx.SelectedItem = objListCmboBx.Items[objListCmboBx.Items.Count - 1];

            matListCmboBx_SelectionChangeCommitted(objListCmboBx, new EventArgs());

            return mt;
        }

        private void renameObjTrick(ref Button oTrickBtn, string newName)
        {
            string matName = oTrickBtn.Text;

            ObjectTrick mt = (ObjectTrick)oTrickBtn.Tag;

            mt.name = newName;

            oTrickBtn.Text = newName;

            oTrickBtn.Name = newName + "_btn";

            objListCmboBx.SuspendLayout();

            objListCmboBx.Items[objListCmboBx.Items.IndexOf(matName)] = newName;

            objListCmboBx.ResumeLayout(false);
        }
        
        public void saveLoadedFile()
        {
            //saveObjTrickButton_Click(this.saveObjTrickButton, new System.EventArgs());
        }

        public ArrayList getPreSaveData()
        {
            commentDeletedObjTrick();

            if (selectdObjTrickBtn != null)
            {
                saveCurrentMatData((Button)selectdObjTrickBtn);
            }

            return objTricksList;
        }

        private void saveObjTrickButton_Click(object sender, System.EventArgs e)
        {
            if (finishedLoading)
            {
                commentDeletedObjTrick();

                if (selectdObjTrickBtn != null)
                {
                    saveCurrentMatData((Button)selectdObjTrickBtn);
                }

                ArrayList newFileContent = new ArrayList();

                ObjectTrick mtMA = (ObjectTrick)objTricksList[0];

                int sIndM1 = mtMA.startIndex;

                //copy original file content before first edited matTrick
                for (int j = 0; j < sIndM1; j++)
                {
                    newFileContent.Add(fileContent[j]);
                }

                //insert first edited ObjectTrick after copied file content
                newFileContent.AddRange(mtMA.data);

                //go through and merge file content not in matTricl
                for (int i = 1; i < objTricksList.Count; i++)
                {
                    ObjectTrick mtM1 = (ObjectTrick)objTricksList[i - 1];

                    ObjectTrick mtM2 = (ObjectTrick)objTricksList[i];

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

        private void deleteObjTrickBtn_Click(object sender, System.EventArgs e)
        {
            if (selectdObjTrickBtn != null)
            {
                Button matBtn = (Button)selectdObjTrickBtn;

                string matName = matBtn.Text;

                string deleteWarning = string.Format("Are your sure you want to delete \"{0}\"?", matName);

                DialogResult dr = MessageBox.Show(  deleteWarning, "Warning...", 
                                                    MessageBoxButtons.YesNo, 
                                                    MessageBoxIcon.Warning, 
                                                    MessageBoxDefaultButton.Button2);

                if (dr == DialogResult.Yes)
                {
                    matBtn.Visible = false;

                    objListCmboBx.SuspendLayout();

                    objListCmboBx.Items.Remove(matName);

                    objListCmboBx.ResumeLayout(false);

                    ObjectTrick mt = (ObjectTrick)matBtn.Tag;

                    mt.deleted = true;

                    reOrderObjTrickButtons();

                    Button nextMatButton = getFirstVisibleButton(matBtn);

                    if (nextMatButton != null)
                    {
                        selectObjTrickBtn(nextMatButton);
                    }
                    else
                    {
                        selectdObjTrickBtn = null;
                        this.objTrickPropertiesPanel.Controls.Clear();
                        deleteObjTrickBtn.Enabled = false;
                        renameObjTrickBtn.Enabled = false;
                        dupObjTrickBtn.Enabled = false;
                        objListCmboBx.Text = "";
                        //saveObjTrickButton.Enabled = false;

                    }
                }
            }            
        }

        private void renameObjTrickBtn_Click(object sender, System.EventArgs e)
        {
            if (selectdObjTrickBtn != null)
            { 
                Button matBtn = (Button)selectdObjTrickBtn;

                string matName = matBtn.Text;

                DialogResult dr = DialogResult.Yes;

                if (((ObjectTrick)matBtn.Tag).startIndex < 5000)
                {
                    string deleteWarning = string.Format("Renaming \"{0}\" will break all references to this Trick!\n\nAre you sure you want to continue with this operation?", matName);

                    dr = MessageBox.Show(  deleteWarning, "Warning...", 
                                           MessageBoxButtons.YesNo, 
                                           MessageBoxIcon.Warning, 
                                           MessageBoxDefaultButton.Button2);
                }

                if (dr == DialogResult.Yes)
                {

                    common.UserPrompt uInput = new COH_CostumeUpdater.common.UserPrompt("Object Trick Name", "Please enter New Object Trick Name");

                    uInput.setTextBoxText(matName);

                    DialogResult uInputDr = uInput.ShowDialog();

                    if (uInputDr == DialogResult.OK)
                    {
                        string matNewName = uInput.userInput;

                        renameObjTrick(ref matBtn, matNewName);
                    }
                }
            }
        }

        private void dupObjTrickBtn_Click(object sender, System.EventArgs e)
        {
            if (selectdObjTrickBtn != null)
            {
                Button matBtn = (Button)selectdObjTrickBtn;

                string matName = matBtn.Text;

                ObjectTrick mt = (ObjectTrick)matBtn.Tag;

                ObjectTrick newMat = getNewObjTrick(mt.data, mt.fileName, mt.name + "_COPY");

                renameObjTrickBtn_Click(selectdObjTrickBtn, new EventArgs());

                reOrderObjTrickButtons();
            }

        }

        private void newObjTrickBtn_Click(object sender, System.EventArgs e)
        {
            common.UserPrompt uInput = new COH_CostumeUpdater.common.UserPrompt("Object Trick Name", "Please enter New Object Trick Name");

            DialogResult dr = uInput.ShowDialog();

            if (dr == DialogResult.OK && uInput.userInput.Trim().Length > 0)
            {

                string matName = uInput.userInput.Trim().Replace(" ", "_");

                buildObjectTrickProperties();

                ArrayList data = getCurrentData(matName);

                ObjectTrick mt = getNewObjTrick(data, fileName, matName);

                deleteObjTrickBtn.Enabled = true;

                renameObjTrickBtn.Enabled = true;

                dupObjTrickBtn.Enabled = true;

                //saveObjTrickButton.Enabled = true;

                reOrderObjTrickButtons();
            }
        }

    }
}
