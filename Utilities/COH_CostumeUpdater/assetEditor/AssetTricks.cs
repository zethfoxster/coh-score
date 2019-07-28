using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using COH_CostumeUpdater.assetEditor.materialTrick;

namespace COH_CostumeUpdater.assetEditor
{
    public partial class AssetTricks : Panel 
        //Form
    {
        private System.Windows.Forms.RichTextBox logTxBx;
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        private string fileName;
        private System.Collections.Generic.Dictionary<string, string> tgaFilesDictionary;
        private ArrayList fileContent;

        public AssetTricks(string file_name,ref RichTextBox rtbx, OpenFileDialog open_file_dialog)
        {
            InitializeComponent();

            logTxBx = rtbx;

            fileName = file_name;

            openFileDialog1 = open_file_dialog;

            fileContent = new ArrayList();

            fileContent.Clear();

            common.COH_IO.fillList(fileContent, fileName);

            string root = @"C:\game\";

            if (this.fileName.ToLower().Contains("gamefix2"))
                root = @"C:\gamefix2\";

            else if (this.fileName.ToLower().Contains("gamefix"))
                root = @"C:\gamefix\";

            tgaFilesDictionary = common.COH_IO.getFilesDictionary(root + @"src\Texture_Library", "*.tga");

            readTricks();
        }

        public void saveAs(string file_name)
        {
            fileName = file_name;
            save(true);
        }

        public void save(bool isSaveAsCmd)
        {
            System.IO.FileInfo fi = new System.IO.FileInfo(fileName);

            bool ro = fi.IsReadOnly;
            
            if (ro && fi.Exists)
            {
                MessageBox.Show("File is ReadOnly!\r\nDid you forget to check it out...", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else //if(isSaveAsCmd || !ro)
            {
                textureTrick.TextureBlockForm tbf = (textureTrick.TextureBlockForm)textureTricks_tabPage.Controls[0];
                objectTrick.ObjectBlockForm otf = (objectTrick.ObjectBlockForm)objectTricks_tabPage.Controls[0];
                materialTrick.matBlockForm mbf = (materialTrick.matBlockForm)materialTricks_tabPage.Controls[0];

                ArrayList matTricksList = mbf.getPreSaveData();
                ArrayList objTricksList = otf.getPreSaveData();
                ArrayList texTricksList = tbf.getPreSaveData();

                ArrayList tmpFileContent = (ArrayList)fileContent.Clone();

                if (matTricksList.Count > 0)
                    preProcessSaveFile(matTricksList, ref tmpFileContent);

                if (objTricksList.Count > 0)
                    preProcessSaveFile(objTricksList, ref tmpFileContent);

                if (texTricksList.Count > 0)
                    preProcessSaveFile(texTricksList, ref tmpFileContent);

                //remove all empty strings in order to have unknow data only
                while (tmpFileContent.Contains(""))
                {
                    tmpFileContent.Remove("");
                }

                ArrayList assetsTricksList = buildNewFileData(matTricksList, objTricksList, texTricksList, tmpFileContent);

                /*
                 string results = assetsMangement.CohAssetMang.checkout(fileName);

                if (results.Contains("can't edit exclusive file"))
                {
                    MessageBox.Show(results);
                    logTxBx.SelectionColor = Color.Red;
                    logTxBx.SelectedText += results;
                }
                 */

                COH_CostumeUpdater.common.COH_IO.writeDistFile(assetsTricksList, fileName);

                //logTxBx.SelectionColor = System.Drawing.SystemColors.WindowText;
                //logTxBx.SelectedText += results;
            }
        }

        private ArrayList buildNewFileData(ArrayList matTricksList, ArrayList objTricksList, ArrayList texTricksList, ArrayList tmpFileContent)
        {
            ArrayList assetsTricksList = new ArrayList();

            addListContent(matTricksList, ref assetsTricksList);
            addListContent(objTricksList, ref assetsTricksList);
            addListContent(texTricksList, ref assetsTricksList);
            assetsTricksList.AddRange(tmpFileContent);

            return assetsTricksList;
        }

        private void addListContent(ArrayList sourceList, ref ArrayList destList)
        {
            for (int i = 0; i < sourceList.Count; i++)
            {
                Trick trick = (Trick)sourceList[i];
                destList.AddRange(trick.data);
                destList.Add("");
            }
        }

        private void preProcessSaveFile(ArrayList tricksList, ref ArrayList tmpFileContent)
        {
            //Go through every trick and replace it wih empty string keep unknow data in original file
            for (int i = 0; i < tricksList.Count; i++)
            {
                Trick currentTrick = (Trick)tricksList[i];

                if(currentTrick.startIndex < tmpFileContent.Count)
                    clearExistingTricksData(currentTrick.startIndex, currentTrick.endIndex, ref tmpFileContent);
            }
        }
        
        private void clearExistingTricksData(int start, int end, ref ArrayList tmpFileContent)
        {
            //Replace known trick data with empty string for removal later
            for (int j = start; j <= end && j < tmpFileContent.Count; j++)
            {
                tmpFileContent[j] = "";
            }
        }
        
        private void readTricks()
        {
            this.assetTricks_tabControl.SuspendLayout();
            this.SuspendLayout();
            bool matIsEmpty = loadMatTrick();
            bool objIsEmpty = loadObjectTrick();
            loadTextureTrick();

            assetTricks_tabControl.SelectedTab = !matIsEmpty ? materialTricks_tabPage :
                (!objIsEmpty ? objectTricks_tabPage : textureTricks_tabPage);

            this.assetTricks_tabControl.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        private bool loadTextureTrick()
        {
            this.textureTricks_tabPage.Controls.Clear();

            this.textureTricks_tabPage.Text = "Texture Tricks";

            textureTrick.TextureBlockForm tbf = new textureTrick.TextureBlockForm(this.fileName, ref this.logTxBx, ref this.fileContent, this.tgaFilesDictionary);

            tbf.Dock = DockStyle.Fill;

            this.textureTricks_tabPage.Controls.Add(tbf);

            this.textureTricks_tabPage.ResumeLayout(false);

            return tbf.isEmpty();
        }

        private bool loadObjectTrick()
        {
            this.objectTricks_tabPage.SuspendLayout();

            this.objectTricks_tabPage.Controls.Clear();

            this.objectTricks_tabPage.Text = "Object Tricks";

            objectTrick.ObjectBlockForm otf = new objectTrick.ObjectBlockForm(fileName, ref this.logTxBx, ref this.fileContent, tgaFilesDictionary);

            otf.Dock = DockStyle.Fill;

            this.objectTricks_tabPage.Controls.Add(otf);

            this.objectTricks_tabPage.ResumeLayout(false);

            return otf.isEmpty();
        }

        private bool loadMatTrick()
        {
            this.materialTricks_tabPage.SuspendLayout();

            this.materialTricks_tabPage.Controls.Clear();

            this.materialTricks_tabPage.Text = "Material Tricks";

            materialTrick.matBlockForm mbf = new materialTrick.matBlockForm(fileName, ref this.logTxBx, ref this.fileContent, tgaFilesDictionary);

            mbf.Dock = DockStyle.Fill;

            this.materialTricks_tabPage.Controls.Add(mbf);

            this.materialTricks_tabPage.ResumeLayout(false);

            return mbf.isEmpty();
        }
    }
}
