using System;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using COH_CostumeUpdater.assetEditor.objectTrick;

namespace COH_CostumeUpdater.assetEditor.textureTrick
{
    public partial class TextureBlockForm :
      Panel
      //Form
    {
        private ArrayList textureTricks;
        private ArrayList fileContent;
        private bool updateTextureTrickListComboBx;
        private string fileName;
        private RichTextBox rTextBox;
        private Object selectdTextureTrickBtn;
        private ArrayList textureTricksList;
        private System.Collections.Generic.Dictionary<string, string> tgaFilesDictionary;
        private bool finishedLoading;


        public TextureBlockForm(string file_name, ref RichTextBox rtBx, ref ArrayList file_content,System.Collections.Generic.Dictionary<string, string> tga_files_dictionary)
        {
            finishedLoading = true;

            common.WatingIconAnim wia = new COH_CostumeUpdater.common.WatingIconAnim();
            
            wia.Show();

            Application.DoEvents();

            this.Cursor = Cursors.WaitCursor;
            
            selectdTextureTrickBtn = null;

            rTextBox = rtBx;

            updateTextureTrickListComboBx = true;

            fileName = file_name;

            System.Windows.Forms.Application.DoEvents();

            InitializeComponent();
                        
            System.Windows.Forms.Application.DoEvents();          

            textureListCmboBx.Items.Clear();

            fileContent = (ArrayList)file_content.Clone();

            tgaFilesDictionary = tga_files_dictionary;

            if (textureTricksList != null)

            {
                textureTricksList.Clear();
            }

            System.Windows.Forms.Application.DoEvents();
           
            textureTricksList = TextureTrickParser.parse(file_name, ref fileContent);

            
            foreach (object obj in textureTricksList)
            {
                System.Windows.Forms.Application.DoEvents();
                TextureTrick tt = (TextureTrick)obj;
                if (!tt.deleted)
                {
                    buildTextureTrickBtn(tt.name, tt);
                }
            }

            if (textureListCmboBx.Items.Count > 0)
            {
                System.Windows.Forms.Application.DoEvents();

                System.Windows.Forms.Application.DoEvents();

                buildTextureTrickProperties();

                textureListCmboBx.SelectedItem = textureListCmboBx.Items[0];

                tTrickListCmboBx_SelectionChangeCommitted(textureListCmboBx, new EventArgs());
            }
            else
            {
                string fName = System.IO.Path.GetFileName(fileName);

                string warning = string.Format("\"{0}\" is not an Texture Trick File.\r\nIt dose not contain (Texture ...End) block.\r\n", fName);
                
                //MessageBox.Show(warning);

                rTextBox.SelectionColor = Color.Red;
                
                rTextBox.SelectedText += warning;

                dupTextureTrickBtn.Enabled = false;

                deleteTextureTrickBtn.Enabled = false;

                renameTextureTrickBtn.Enabled = false;
            }

            System.Windows.Forms.Application.DoEvents();
            
            wia.Close();
            
            wia.Dispose();
            
            renameTextureTrickBtn.Enabled = false;

            this.Cursor = Cursors.Arrow;
        }

        public bool isEmpty()
        {
            return !(textureTricksList.Count > 0);
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

        private void buildTextureTrickProperties()
        {
            this.SuspendLayout();

            this.textureTricks = new ArrayList();

            this.textureTricks.Clear();

            this.textureTrickPropertiesPanel.Controls.Clear();

            buildTextureTrickSec();

            System.Windows.Forms.Application.DoEvents();      

            setControlsLocationNtag();

            System.Windows.Forms.Application.DoEvents();

            this.ResumeLayout(false);
        }

        private void setTextureTrickProperties(Button matTrickBtn)
        {
            finishedLoading = false;

            this.SuspendLayout();

            TextureTrick mt = (TextureTrick)matTrickBtn.Tag;

            ArrayList data = (ArrayList)mt.data;

            TextureTrickPanel ttPnl = (TextureTrickPanel)this.textureTrickPropertiesPanel.Controls[0];

            ttPnl.setData(data);

            this.ResumeLayout();

            finishedLoading = true;
        }
        
        private void initTextureTrickListComboBox(string tTrickName)
        {
            this.textureListCmboBx.Items.Add(tTrickName);
        }

        private void buildTextureTrickBtn(string ttName, TextureTrick tt)
        {
            int locX = 3, locY = 3;
            string btnName = ttName + "_btn";
            
            if (textureTrickListPanel.Controls.Count > 0)
            {
                Button lastMatBtn = (Button)textureTrickListPanel.Controls[textureTrickListPanel.Controls.Count - 1];
                locX = lastMatBtn.Right + 10;
            }
            
            Button btn = new Button();
            btn.Tag = tt;
            btn.Image = global::COH_CostumeUpdater.Properties.Resources.AE_UI_MaterialTrick;
            if (ttName.Contains("/"))
                btn.Image = global::COH_CostumeUpdater.Properties.Resources.OPENFOLD;
            btn.Location = new System.Drawing.Point(locX, locY);
            btn.Name = btnName;
            btn.Size = new System.Drawing.Size(75, 75);
            btn.TabIndex = 0;
            btn.Text = ttName;
            btn.ImageAlign = System.Drawing.ContentAlignment.TopCenter;
            btn.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
            btn.UseVisualStyleBackColor = true;
            btn.Click += new System.EventHandler(this.textureTrickkBtn_Click);

            this.textureTrickListPanel.Controls.Add(btn);

            initTextureTrickListComboBox(ttName);
        }
       
        private void buildTextureTrickSec()
        {
            TextureTrickPanel ttPnl = new TextureTrickPanel(this.openFileDialog1, this.tgaFilesDictionary);
            ttPnl.Dock = DockStyle.Fill;
            this.textureTrickPropertiesPanel.Controls.Add(ttPnl);
        }

        private void setControlsLocationNtag()
        {
            int len = this.textureTrickPropertiesPanel.Controls.Count;

            this.textureTrickPropertiesPanel.Controls[0].Location = new Point(0, 0);

            for (int i = 1; i < len; i++)
            {
                this.textureTrickPropertiesPanel.Controls[i].Location = new Point(0, this.textureTrickPropertiesPanel.Controls[i - 1].Bottom);
                this.textureTrickPropertiesPanel.Controls[i - 1].Tag = this.textureTrickPropertiesPanel.Controls[i];
            }

            this.textureTrickPropertiesPanel.Controls[len-1].Tag = null;
        }

        private void tTrickListCmboBx_SelectionChangeCommitted(object sender, System.EventArgs e)
        {
            if (finishedLoading)
            {
                if (textureListCmboBx.Items.Count > 0)
                {
                    string btnName = textureListCmboBx.SelectedItem.ToString() + "_btn";

                    Control[] ctl = textureTrickListPanel.Controls.Find(btnName, false);

                    if (ctl.Length > 0 && updateTextureTrickListComboBx)
                    {
                        Button btn = (Button)ctl[0];

                        if (selectdTextureTrickBtn != null)
                        {
                            deSelectTextureTrickBtn((Button)selectdTextureTrickBtn);
                        }

                        selectTextureTrickBtn(btn);

                        textureTrickkBtn_Click(btn, new EventArgs());
                    }
                }
            }
            else
            {
                MessageBox.Show("Sorry, still loading Mat data. Please wait :)");
            }
        }

        private void textureTrickkBtn_Click(object sender, EventArgs e)
        {
            if (finishedLoading)
            {
                Button oldButton = (Button)sender;

                Button bt = (Button)sender;

                if (selectdTextureTrickBtn != null)
                {
                    oldButton = (Button)selectdTextureTrickBtn;

                    if (!bt.Equals(oldButton))
                    {
                        deSelectTextureTrickBtn(oldButton);
                        saveCurrentMatData(oldButton);
                        selectTextureTrickBtn(bt);
                    }
                }
                else
                {
                    selectTextureTrickBtn(bt);
                }
            }
            else
            {
                MessageBox.Show("Sorry, still loading Mat data. Please wait :)");
            }
        }

        private void deSelectTextureTrickBtn(Button btn)
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

        private void selectTextureTrickBtn(Button btn)
        {
            if (btn.Visible)
            {
                Color clr = Color.LightYellow;

                Point p = btn.Location;

                updateTextureTrickListComboBx = false;

                btn.Parent.SuspendLayout();

                btn.BackColor = clr;

                btn.Location = new Point(p.X - 2, p.Y - 2);

                btn.Size = new Size(80, 80);

                int i = textureListCmboBx.Items.IndexOf(btn.Name.Replace("_btn", ""));

                textureListCmboBx.SelectedItem = textureListCmboBx.Items[i];

                selectdTextureTrickBtn = btn;

                btn.Parent.ResumeLayout(false);

                ((Panel)btn.Parent).ScrollControlIntoView(btn);

                updateTextureTrickListComboBx = true;

                setTextureTrickProperties(btn);

                dupTextureTrickBtn.Enabled = !btn.Name.Contains("/");
            }
        }

        private void saveCurrentMatData(Button btn)
        {
            TextureTrick tt = (TextureTrick)btn.Tag;
            saveCurrentTextureTrickData(tt);
        }

        private void saveCurrentTextureTrickData(TextureTrick ot)
        {
            if(finishedLoading)
                ot.data = getCurrentData(ot.name);
        }

        private ArrayList getCurrentData(string oTrickName)
        {
            System.Collections.ArrayList data = new System.Collections.ArrayList();

            data.Clear();

            data.Add("Texture " + oTrickName);

            TextureTrickPanel ttPnl = (TextureTrickPanel)this.textureTrickPropertiesPanel.Controls[0];

            ttPnl.getData(ref data);

            data.Add("End");

            return data;
        }

        private void saveTextureTrickDataToFileContent(Button oTrickBtn)
        {
            TextureTrick TextureTrick = (TextureTrick)oTrickBtn.Tag;
            saveTextureTrickDataToFileContent(TextureTrick);
        }

        private void saveTextureTrickDataToFileContent(TextureTrick textureTrick)
        {
            int startInd = textureTrick.startIndex;
            int endInd = textureTrick.endIndex;

            fileContent.RemoveRange(startInd, textureTrick.data.Count);
            fileContent.InsertRange(startInd, textureTrick.data);
        }

        private void commentDeletedTextureTrick()
        {
            foreach (object obj in textureTricksList)
            {
                TextureTrick tt = (TextureTrick)obj;
                if (tt.deleted)
                {
                    for (int i = 0; i < tt.data.Count; i++)
                    {
                        tt.data[i] = string.Format("//{0}", (string)tt.data[i]);
                    }
                }
            }
        }

        private void reOrderTextureTrickButtons()
        {
            int locX = 3, locY = 3;
            textureTrickListPanel.AutoScrollPosition = new Point(0, 0);
            textureTrickListPanel.SuspendLayout();

            Button selBtn = (Button)selectdTextureTrickBtn;

            deSelectTextureTrickBtn(selBtn);

            foreach (object obj in textureTrickListPanel.Controls)
            {
                Button btn = (Button)obj;
                TextureTrick tt = (TextureTrick)btn.Tag;
                if (!tt.deleted)
                {
                    btn.Location = new Point(locX, locY);
                    locX = btn.Right + 10;
                }
            }

            textureTrickListPanel.ResumeLayout(false);

            if (selBtn != null)
                selectTextureTrickBtn((Button)selBtn);
        }

        private Button getFirstVisibleButton(Button btn)
        {
            Button nextMatButton = null;

            foreach( Control ctl in textureTrickListPanel.Controls)
            {
                if (((Button)ctl).Visible)
                {
                    nextMatButton = (Button)ctl;
                    return nextMatButton;
                }
            }  

            return nextMatButton;
        }

        private TextureTrick getNewTextureTrick(ArrayList data, string fileName, string tTrickName)
        {
            int mCount = textureTricksList.Count;

            int startIndex = mCount > 0 ? ((TextureTrick)textureTricksList[mCount - 1]).endIndex + 5000 : 5000;

            int endIndex = startIndex + data.Count;

            TextureTrick tt = new TextureTrick(startIndex, endIndex, data, fileName, tTrickName);

            textureTricksList.Add(tt);

            buildTextureTrickBtn(tt.name, tt);

            reOrderTextureTrickButtons();

            textureListCmboBx.SelectedItem = textureListCmboBx.Items[textureListCmboBx.Items.Count - 1];

            tTrickListCmboBx_SelectionChangeCommitted(textureListCmboBx, new EventArgs());

            return tt;
        }

        private void renameTextureTrick(ref Button tTrickBtn, string newName)
        {
            string ttName = tTrickBtn.Text;

            TextureTrick tt = (TextureTrick)tTrickBtn.Tag;

            tt.name = newName;

            tTrickBtn.Text = newName;

            tTrickBtn.Name = newName + "_btn";

            textureListCmboBx.SuspendLayout();

            textureListCmboBx.Items[textureListCmboBx.Items.IndexOf(ttName)] = newName;

            textureListCmboBx.ResumeLayout(false);
        }
        
        public void saveLoadedFile()
        {
            //saveObjTrickButton_Click(this.saveTextureTrickButton, new System.EventArgs());
        }

        public ArrayList getPreSaveData()
        {
            commentDeletedTextureTrick();

            if (selectdTextureTrickBtn != null)
            {
                saveCurrentMatData((Button)selectdTextureTrickBtn);
            }

            return textureTricksList;
        }

        private void saveObjTrickButton_Click(object sender, System.EventArgs e)
        {
            if (finishedLoading)
            {
                commentDeletedTextureTrick();

                if (selectdTextureTrickBtn != null)
                {
                    saveCurrentMatData((Button)selectdTextureTrickBtn);
                }

                ArrayList newFileContent = new ArrayList();

                TextureTrick ttMA = (TextureTrick)textureTricksList[0];

                int sIndM1 = ttMA.startIndex;

                //copy original file content before first edited textureTrick
                for (int j = 0; j < sIndM1; j++)
                {
                    newFileContent.Add(fileContent[j]);
                }

                //insert first edited textureTrick after copied file content
                newFileContent.AddRange(ttMA.data);

                //go through and merge file content not in textureTrick
                for (int i = 1; i < textureTricksList.Count; i++)
                {
                    TextureTrick ttM1 = (TextureTrick)textureTricksList[i - 1];

                    TextureTrick ttM2 = (TextureTrick)textureTricksList[i];

                    int eIndM1 = ttM1.endIndex;

                    int sIndM2 = ttM2.startIndex;

                    for (int j = eIndM1 + 1; j < sIndM2 && j < fileContent.Count; j++)
                    {
                        newFileContent.Add(fileContent[j]);
                    }
                    if (sIndM2 >= fileContent.Count)
                    {
                        newFileContent.Add("");
                    }
                    newFileContent.AddRange(ttM2.data);
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

        private void deleteTextureTrickBtn_Click(object sender, System.EventArgs e)
        {
            if (selectdTextureTrickBtn != null)
            {
                Button ttBtn = (Button)selectdTextureTrickBtn;

                string ttName = ttBtn.Text;

                string deleteWarning = string.Format("Are your sure you want to delete \"{0}\"?", ttName);

                DialogResult dr = MessageBox.Show(  deleteWarning, "Warning...", 
                                                    MessageBoxButtons.YesNo, 
                                                    MessageBoxIcon.Warning, 
                                                    MessageBoxDefaultButton.Button2);

                if (dr == DialogResult.Yes)
                {
                    ttBtn.Visible = false;

                    textureListCmboBx.SuspendLayout();

                    textureListCmboBx.Items.Remove(ttName);

                    textureListCmboBx.ResumeLayout(false);

                    TextureTrick mt = (TextureTrick)ttBtn.Tag;

                    mt.deleted = true;

                    reOrderTextureTrickButtons();

                    Button nextTextureButton = getFirstVisibleButton(ttBtn);

                    if (nextTextureButton != null)
                    {
                        selectTextureTrickBtn(nextTextureButton);
                    }
                    else
                    {
                        selectdTextureTrickBtn = null;
                        this.textureTrickPropertiesPanel.Controls.Clear();
                        deleteTextureTrickBtn.Enabled = false;
                        renameTextureTrickBtn.Enabled = false;
                        dupTextureTrickBtn.Enabled = false;
                        textureListCmboBx.Text = "";
                        //saveTextureTrickButton.Enabled = false;

                    }
                }
            }            
        }

        private void renameTextureTrickBtn_Click(object sender, System.EventArgs e)
        {
            if (selectdTextureTrickBtn != null)
            { 
                Button ttBtn = (Button)selectdTextureTrickBtn;

                string ttName = ttBtn.Text;

                DialogResult dr = DialogResult.Yes;

                if (((TextureTrick)ttBtn.Tag).startIndex < 5000)
                {
                    string deleteWarning = string.Format("Renaming \"{0}\" will break all references to this Trick!\n\nAre you sure you want to continue with this operation?", ttName);

                    dr = MessageBox.Show(  deleteWarning, "Warning...", 
                                           MessageBoxButtons.YesNo, 
                                           MessageBoxIcon.Warning, 
                                           MessageBoxDefaultButton.Button2);
                }

                if (dr == DialogResult.Yes)
                {

                    common.UserPrompt uInput = new COH_CostumeUpdater.common.UserPrompt("Texture Trick Name", "Please enter New Object Trick Name");

                    uInput.setTextBoxText(ttName);

                    DialogResult uInputDr = uInput.ShowDialog();

                    if (uInputDr == DialogResult.OK)
                    {
                        string matNewName = uInput.userInput;

                        renameTextureTrick(ref ttBtn, matNewName);
                    }
                }
            }
        }

        private void duplicateTextureTrick(ArrayList data)
        {
            TextureTrickPanel ttPnl = null;
            if (textureTrickPropertiesPanel.Controls.Count > 0)
            {
                ttPnl = (TextureTrickPanel)textureTrickPropertiesPanel.Controls[0];
            }
            else
            {
                //buildTextureTrickProperties();
                buildTextureTrickSec();
                ttPnl = (TextureTrickPanel)textureTrickPropertiesPanel.Controls[0];
            }

            string tgaPath = ttPnl.getTextureTrickTgaPath();

            if (tgaPath.Length > 1)
            {
                openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(tgaPath);
                openFileDialog1.FileName = System.IO.Path.GetFileName(tgaPath);
            }

            DialogResult dr = openFileDialog1.ShowDialog();

            if (dr == DialogResult.OK)
            {
                string ttName = System.IO.Path.GetFileName(openFileDialog1.FileName);

                ArrayList newData = new ArrayList();

                newData.Add("Texture " + ttName);

                if (data != null)
                {
                    for (int i = 1; i < data.Count; i++)
                    {
                        newData.Add(data[i]);
                    }
                }
                else
                {
                    newData.Add("End");
                }

                TextureTrick newTT = getNewTextureTrick(newData, fileName, ttName);

                reOrderTextureTrickButtons();

                this.textureTrickPropertiesPanel.SuspendLayout();

                ttPnl.setData(newTT.data);

                this.textureTrickPropertiesPanel.ResumeLayout();
            }
        }

        private void dupTextureTrickBtn_Click(object sender, System.EventArgs e)
        {
            if (selectdTextureTrickBtn != null)
            {
                Button ttBtn = (Button)selectdTextureTrickBtn;

                TextureTrick tt = (TextureTrick)ttBtn.Tag;

                duplicateTextureTrick(tt.data);       
            }
        }

        private void newTextureTrickBtn_Click(object sender, System.EventArgs e)
        {
            duplicateTextureTrick(null);

            deleteTextureTrickBtn.Enabled = true;

            //renameTextureTrickBtn.Enabled = true;

            dupTextureTrickBtn.Enabled = true;

            //saveTextureTrickButton.Enabled = true;
        }

    }
}
