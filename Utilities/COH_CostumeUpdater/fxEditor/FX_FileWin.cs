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
    public partial class FX_FileWin : Panel
        //Form
    {
        private ArrayList fxList;
        
        private String fileName;
        
        private System.Windows.Forms.ImageList imgList;
        
        public Dictionary<string, string> tgaFilesDictionary;

        public FX_FileWin(string fxFileName, ref Dictionary<string, string> tgaDic)
        {
            initializeImageList();

            fxList = new ArrayList();

            fileName = fxFileName;

            if (tgaDic != null)
            {
                tgaFilesDictionary = tgaDic.ToDictionary(k => k.Key, k => k.Value);
            }
            else
            {
                MessageBox.Show("Building TGA texture Dictionary...\r\nThis may take few seconds!");

                tgaFilesDictionary = common.COH_IO.getFilesDictionary(common.COH_IO.getRootPath(fileName).Replace(@"\data", "") + @"src\Texture_Library", "*.tga");
                tgaDic = tgaFilesDictionary.ToDictionary(k => k.Key, k => k.Value);
            }

            this.fxWin = new COH_CostumeUpdater.fxEditor.FXWin(fxFileName, ref tgaDic, imgList);
            this.bhvrWin = new COH_CostumeUpdater.fxEditor.BehaviorWin(imgList, this.fileName);
            this.partWin = new COH_CostumeUpdater.fxEditor.ParticalsWin(this.fxWin.RootPath, ref tgaDic, imgList);
            InitializeComponent();
            readFxFile(fileName);
        }

        private void initializeImageList()
        {
            this.imgList = new ImageList();
            this.imgList.ImageSize = new Size(16, 8);
            this.imgList.Images.Add(Properties.Resources.ImgGreenGreen);
            this.imgList.Images.Add(Properties.Resources.ImgRedGreen);
            this.imgList.Images.Add(Properties.Resources.ImgRedRed);
            this.imgList.Images.Add(Properties.Resources.ImgRedBlack);
        }

        public void readFxFile(string file_Name)
        {
            ArrayList fileContent = new ArrayList();
            common.COH_IO.fillList(fileContent, fileName);
            fxList = FX_Parser.parse(file_Name, ref fileContent);

            populateFxWin();
        }
        
        public void saveAs(string file_Name)
        {
            fileName = file_Name;
            FX fx = (FX)fxWin.Tag;
            ArrayList data = fxWin.getData();
            common.COH_IO.writeDistFile(data, fileName);
            FxLauncher.FXLauncherForm.runFx(fileName, (COH_CostumeUpdaterForm)this.FindForm());
        }

        public void save()
        {
            FX fx = (FX)fxWin.Tag;

            if (fx.isReadOnly)
            {
                MessageBox.Show("File is ReadOnly!\r\nDid you forget to check it out...", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
            {
                ArrayList data = fxWin.getData();
                common.COH_IO.writeDistFile(data, fileName);
                COH_CostumeUpdaterForm cForm = (COH_CostumeUpdaterForm)this.FindForm();
                if (cForm != null)
                {
                    COH_CostumeUpdaterForm cTopForm = (COH_CostumeUpdaterForm)cForm.Owner;
                    if (cTopForm != null)
                    {
                        string fxFile = cTopForm.currentLoadedFileName.ToLower().EndsWith(".fx") ? cTopForm.currentLoadedFileName : fileName;
                        FxLauncher.FXLauncherForm.runFx(fxFile, cTopForm); 
                    }
                    else
                        FxLauncher.FXLauncherForm.runFx(fileName, cForm); 
                }               
            }
        }

        private void populateFxWin()
        {
            this.fxWin.fillFxTree(fxList, this.bhvrWin, this.partWin);
        }
    }
}
