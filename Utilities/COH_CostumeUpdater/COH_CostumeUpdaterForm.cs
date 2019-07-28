using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Imaging;

namespace COH_CostumeUpdater
{
    public partial class COH_CostumeUpdaterForm : Form
    {
        public Dictionary<string, string> tgaFilesDictionary;
        
        private COH_CostumeUpdaterForm cuf;

        public string currentLoadedFileName;

        private bool fileSaved;

        private string rootPath;
       
        public FxLauncher.FXLauncherForm fxLauncher;

        public COH_CostumeUpdaterForm()
        {
            InitializeComponent();
            fileSaved = false;
            this.revisionLable.Text = "Version: " + GetVersion();

            if (File.Exists(@"C:\Cryptic\scratch\fxConfig"))
            {
                ArrayList configData = new ArrayList();

                configData.Clear();

                common.COH_IO.fillList(configData, @"C:\Cryptic\scratch\fxConfig");

                rootPath = ((string)configData[0]).Split(' ')[1];
            }
            
            openAssociatedFile();
          
            //below is test of future tools don't un comment
            //this.menuStrip1.BackColor = averageColor(@"N:\users\user\thumbNail\ruin_frwy_m_01.jpg");
            //creatThumbNail(64, 64); 
            
            //below is removal of un used tricks 
            //assetEditor.RemoveUnUsedTricks rTrick = new COH_CostumeUpdater.assetEditor.RemoveUnUsedTricks(@"c:\test\unreferenced_tricks.csv");
            //rTrick.removeUnusedTricks();
            //rTrick.compareMissingTextures();

            //below finds EndlessTricks
            //assetEditor.materialTrick.EndlessTricks.findEndlessTricks();
        }

        private void openAssociatedFile()
        {
            if (AppDomain.CurrentDomain.SetupInformation.ActivationArguments != null)
            {
                //   Get the ActivationArguments from the SetupInformation property of the domain.
                string[] activationData = AppDomain.CurrentDomain.SetupInformation.ActivationArguments.ActivationData;

                if (activationData != null && activationData.Length > 0)
                {
                    try
                    {
                        Uri uri = new Uri(activationData[0]); // SDI only allows one file

                        if (uri.IsFile)
                        {
                            string uriFile = uri.AbsolutePath;
                            FileInfo fi = new FileInfo(uriFile);

                            uriFile = fi.FullName;

                            if(Path.GetExtension(uriFile).ToLower().Equals(".fx"))
                                this.loadFxFile(uriFile);
                            else if (Path.GetExtension(uriFile).ToLower().Equals(".txt"))
                                this.loadAssetsTrick(uriFile);
                            else if (Path.GetExtension(uriFile).ToLower().Contains(".par"))
                                this.loadPartFile(uriFile);                            
                        }
                    }
                    catch (UriFormatException)
                    {
                        MessageBox.Show("Invalid file specified.");
                    }
                }
            }
        }

        private void saveFormLocInReg(Point formLoc, int screenIndx)
        {
            int dwrdX = formLoc.X;
            int dwrdY = formLoc.Y;
            string data = string.Format("{0} {1} {2}", dwrdX, dwrdY, screenIndx);

            string regKeyName = "COH_AET_FormLocation_lc";

            Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(regKeyName, true);

            if (key == null)
                key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(regKeyName);

            key.SetValue(regKeyName, data, Microsoft.Win32.RegistryValueKind.String);

            key.Close();

        }

        private string readFormLocInReg()
        {
            string regKeyName = "COH_AET_FormLocation_lc";

            string data = "";

            Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(regKeyName);
            if (key != null)
            {
                object kVal = key.GetValue(regKeyName);

                if (kVal != null)
                    data = (string)kVal;

                key.Close();
            }

            return data;
        }

        private void COH_CostumeUpdaterForm_Load(object sender, System.EventArgs e)
        {
            string data = readFormLocInReg();

            if (data.Length > 0)
            {
                string[] dataSplit = data.Split(' ');
                if (dataSplit.Length == 3)
                {
                    int x, y, ind;

                    bool success = int.TryParse(dataSplit[0], out x);

                    success = int.TryParse(dataSplit[1], out y);

                    success = int.TryParse(dataSplit[2], out ind);

                    if (success && Screen.AllScreens.Length > ind && ind != -1)
                    {
                        Screen currentScreen = Screen.AllScreens[ind];
                        
                        Point p = new Point(x,y);
                        
                        if (currentScreen.Bounds.Contains(p))
                        {
                            p.X = x - currentScreen.Bounds.X;
                            p.Y = y - currentScreen.Bounds.Y;
                        }
                        else
                        {
                            p.X = 0;
                            p.Y = 0;
                        }                        
                        
                        this.StartPosition = FormStartPosition.Manual;
                        
                        this.Location = new Point(currentScreen.Bounds.Location.X + p.X,currentScreen.Bounds.Location.Y + p.Y);
                    }
                }
            }
        }

        private void COH_CostumeUpdaterForm_FormClosing(object sender, System.Windows.Forms.FormClosingEventArgs e)
        {
            int screenIndx = -1;

            Screen currentScreen = Screen.FromControl(this);

            for (int i=0; i < Screen.AllScreens.Length; i++)
            {
                Screen s= Screen.AllScreens[i];

                if (s.DeviceName.Equals(currentScreen.DeviceName))
                {
                    screenIndx = i;
                    break;
                }
            }
            saveFormLocInReg(this.Location, screenIndx);
        }

        private string GetVersion()
        {
            string ourVersion = string.Empty;
            // This will only work when running from deployed app not from VS.
            if (System.Deployment.Application.ApplicationDeployment.IsNetworkDeployed)
                ourVersion = System.Deployment.Application.ApplicationDeployment.CurrentDeployment.CurrentVersion.ToString();
            else
            {
                System.Reflection.Assembly assemblyInfo = System.Reflection.Assembly.GetExecutingAssembly();
                if (assemblyInfo != null)
                    ourVersion = assemblyInfo.GetName().Version.ToString();
            }
            return ourVersion;
        }

        //start of asset browser
        private void creatThumbNail(int w, int h)
        {
            Dictionary<string, string> textureFilesDictionary;
            textureFilesDictionary = common.COH_IO.getFilesDictionary(@"c:\game\data\texture_library", "*.texture");
            costume.ddsViewer dds = new COH_CostumeUpdater.costume.ddsViewer(@"C:\Test\thumbNail", w, h, false);
            int i = 0;
            foreach (KeyValuePair<string, string> kvp in textureFilesDictionary)
            {
                if (dds == null)
                {
                    dds = new COH_CostumeUpdater.costume.ddsViewer(@"C:\Test\thumbNail", w, h, false);
                }
                try
                {
                    dds.saveImage(kvp.Key);
                }
                catch (Exception ex) 
                {
                    dds = null;
                }
            }
        }

        //color tag
        private Color averageColor(string imageFile)
        {
            Bitmap btmap = new Bitmap(imageFile);

            System.Drawing.Imaging.BitmapData bmpData = btmap.LockBits(new Rectangle(0, 0, btmap.Width, btmap.Height),
                                                                        ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);

            // Get the address of the first line.
            IntPtr ptr = bmpData.Scan0;

            // Declare an array to hold the bytes of the bitmap.
            int bytes = Math.Abs(bmpData.Stride) * btmap.Height;

            byte[] rgbaValues = new byte[bytes];

            // Copy the RGB values into the array.
            System.Runtime.InteropServices.Marshal.Copy(ptr, rgbaValues, 0, bytes);

            long totalA = 0;
            long totalR = 0;
            long totalG = 0;
            long totalB = 0;
            long widthXheight = bmpData.Width * bmpData.Height;

            int i = 0;

            for (int y = 0; y < bmpData.Height; y++)
            {
                for (int x = 0; x < bmpData.Width; x++)
                {
                    totalB += rgbaValues[i++];
                    totalG += rgbaValues[i++];
                    totalR += rgbaValues[i++];
                    totalA += rgbaValues[i++];

               }

            }

            btmap.UnlockBits(bmpData);            

            int avgR = (int)(totalR / widthXheight);
            int avgG = (int)(totalG / widthXheight);
            int avgB = (int)(totalB / widthXheight);

            return Color.FromArgb(avgR, avgG, avgB);
        }

        private void registerUser(string toolName, string fileName)
        {
            System.Security.Principal.WindowsIdentity user;

            user = System.Security.Principal.WindowsIdentity.GetCurrent();

            if (user.Name.Contains("user"))
                return;

            ArrayList userList = new ArrayList();

            string path = @"\\ncncfolders.paragon.ncwest.ncsoft.corp\dodo\users\user\users.txt";

            string userStr = string.Format("\"{0}\" opened with: {1} by: {2} @ {3}", fileName, toolName, user.Name, DateTime.Now);

            common.COH_IO.fillList(userList, path);

            userList.Add(userStr);

            common.COH_IO.writeDistFile(userList, path);
        }

        //tint menustrip darker to indicate parent/child relation
        public void showCfx(Color parentColor)
        {
            int red = Math.Min(255, parentColor.R + 10);
            int green = Math.Max(0, parentColor.G - 60);
            int blue = Math.Max(0, parentColor.B - 60);

            this.menuStrip1.BackColor = Color.FromArgb(red,green,blue);

            this.Show();
        }

        public void writeToLogBox(string txt, Color slColor)
        {
            Color selClr = logTxBx.SelectionColor;

            this.logTxBx.SelectionColor = slColor;

            this.logTxBx.SelectedText += txt;

            this.logTxBx.SelectionColor = selClr;
        }

        public Color MenuStripColor
        {
            get
            {
                return this.menuStrip1.BackColor;
            }
        }

        public void loadLauncherFile(string file_name, string fileType, bool newWin)
        {
            switch (fileType)
            {
                case "fx":
                    if (newWin)
                        loadFxFileInNewWin(file_name);
                    else
                        loadFxFile(file_name);
                    break;

                case "txt":
                    if (newWin)
                        loadAssetsFileInNewWin(file_name);
                    else
                        loadAssetsTrick(file_name);
                    break;

                case "part":
                    if (newWin)
                        loadPartFileInNewWin(file_name);
                    else
                        loadPartFile(file_name);
                    break;
            }
        }

        public void loadAssetsFileInNewWin(string file_name)
        {
            if (cuf == null)
            {
                cuf = new COH_CostumeUpdaterForm();

                cuf.FormClosing += new FormClosingEventHandler(cuf_FormClosing);
            }

            cuf.loadAssetsTrick(file_name);

            cuf.showCfx(this.MenuStripColor);

            cuf.fxLauncher.selectFxFile(file_name);
        }

        public void loadFxFileInNewWin(string file_name)
        {
            if (cuf == null)
            {
                cuf = new COH_CostumeUpdaterForm();

                cuf.FormClosing -= new FormClosingEventHandler(cuf_FormClosing);
                cuf.FormClosing += new FormClosingEventHandler(cuf_FormClosing);
            }

            if (cuf.tgaFilesDictionary == null ||cuf.tgaFilesDictionary.Count == 0)
                cuf.tgaFilesDictionary = this.tgaFilesDictionary.ToDictionary(k => k.Key, k => k.Value);

            cuf.loadFxFile(file_name);

            cuf.showCfx(this.MenuStripColor);

            cuf.fxLauncher.selectFxFile(file_name);
        }

        public void loadPartFileInNewWin(string file_name)
        {
            if (cuf == null)
            {
                cuf = new COH_CostumeUpdaterForm();

                cuf.FormClosing -= new FormClosingEventHandler(cuf_FormClosing);
                cuf.FormClosing += new FormClosingEventHandler(cuf_FormClosing);
            }

            if (cuf.tgaFilesDictionary == null || cuf.tgaFilesDictionary.Count == 0)
                cuf.tgaFilesDictionary = this.tgaFilesDictionary.ToDictionary(k => k.Key, k => k.Value);

            cuf.loadPartFile(file_name);

            cuf.showCfx(this.MenuStripColor);

            cuf.fxLauncher.selectFxFile(file_name);
        }

        private void cuf_FormClosing(object sender, FormClosingEventArgs e)
        {
            cuf = null;
            ((Form)sender).Dispose();
        }

        public void loadPartFile(string file_name)
        {
            if (cEditor != null)
            {
                cEditor.Dispose();

                cEditor = null;
            }

            if (fxLauncher != null && !fxLauncher.fileType.ToLower().Equals("part"))
            {
                this.CostumeSplitContainer.Panel1.Controls.Remove(this.fxLauncher);
                this.fxLauncher.Dispose();
                this.fxLauncher = null;
            }

            if (fxLauncher == null)
            {
                this.fxLauncher = new FxLauncher.FXLauncherForm(@"C:\Cryptic\scratch\partHistory.txt", @"C:\Cryptic\scratch\myFavorite.txt", "part", rootPath);
                fxLauncher.Dock = DockStyle.Fill;
                this.CostumeSplitContainer.Panel1.Controls.Add(this.fxLauncher);
            }

            currentLoadedFileName = file_name;

            this.CostumeSplitContainer.SuspendLayout();
            this.SuspendLayout();

            if (this.CostumeSplitContainer.Panel2.Controls.Count > 0)
                this.CostumeSplitContainer.Panel2.Controls[0].Dispose();

            this.CostumeSplitContainer.Panel2.Controls.Clear();
            this.tabPage1.Text = "Part File";

            this.Text = "Part Editor: " + file_name;


            if (rootPath == null)
            {
                rootPath = common.COH_IO.getRootPath(file_name);
            }

            fxEditor.ParticalsWin partWin = new COH_CostumeUpdater.fxEditor.ParticalsWin(this.rootPath, file_name, ref tgaFilesDictionary);
            partWin.Dock = DockStyle.Fill;
            this.CostumeSplitContainer.Panel2.Controls.Add(partWin);
            this.CostumeSplitContainer.ResumeLayout(false);
            this.ResumeLayout(false);
            loadedObject = "partFile";
            fxLauncher.selectFxFile(file_name);
            registerUser("PartEditor", file_name);
        }

        public void loadFxFile(string file_name)
        {
            if (cEditor != null)
            {
                cEditor.Dispose();

                cEditor = null;
            }

            if (fxLauncher != null && !fxLauncher.fileType.ToLower().Equals("fx"))
            {
                this.CostumeSplitContainer.Panel1.Controls.Remove(this.fxLauncher);
                this.fxLauncher.Dispose();
                this.fxLauncher = null;
            }

            if (fxLauncher == null)
            {
                this.fxLauncher = new FxLauncher.FXLauncherForm(@"C:\Cryptic\scratch\tfx.txt", @"C:\Cryptic\scratch\myFavorite.txt", "fx", rootPath);
                fxLauncher.Dock = DockStyle.Fill;
                this.CostumeSplitContainer.Panel1.Controls.Add(this.fxLauncher);
            }

            currentLoadedFileName = file_name;

            this.CostumeSplitContainer.SuspendLayout();
            this.SuspendLayout();

            //disposing causes refersh bug (8/5/2011) need to rework later
            if(this.CostumeSplitContainer.Panel2.Controls.Count>0)
                this.CostumeSplitContainer.Panel2.Controls[0].Dispose();

            this.CostumeSplitContainer.Panel2.Controls.Clear();

            this.tabPage1.Text = "FX/Bhvr/Part File";

            this.Text = "FX Editor: " + file_name;
            fxEditor.FX_FileWin ffw = new COH_CostumeUpdater.fxEditor.FX_FileWin(file_name, ref tgaFilesDictionary);
            ffw.Dock = DockStyle.Fill;
            this.CostumeSplitContainer.Panel2.Controls.Add(ffw);
            this.CostumeSplitContainer.ResumeLayout(false);

            this.ResumeLayout(false);
            loadedObject = "fxFile";
            fxLauncher.selectFxFile(file_name);
            registerUser("FxEditor", file_name);
        }

        private void loadAssetsTrick(string file_name)
        {
            if (cEditor != null)
            {
                cEditor.Dispose();

                cEditor = null;
            }

            if (fxLauncher != null && !fxLauncher.fileType.ToLower().Equals("txt"))
            {
                this.CostumeSplitContainer.Panel1.Controls.Remove(this.fxLauncher);
                this.fxLauncher.Dispose();
                this.fxLauncher = null;
            }

            if (fxLauncher == null)
            {
                this.fxLauncher = new FxLauncher.FXLauncherForm(@"C:\Cryptic\scratch\assetsTFX.txt", @"C:\Cryptic\scratch\myFavoriteAssetsFiles.txt", "txt", rootPath);
                fxLauncher.Dock = DockStyle.Fill;
                this.CostumeSplitContainer.Panel1.Controls.Add(this.fxLauncher);
            }

            this.CostumeSplitContainer.SuspendLayout();

            this.SuspendLayout();

            if (this.CostumeSplitContainer.Panel2.Controls.Count > 0)
                this.CostumeSplitContainer.Panel2.Controls[0].Dispose();

            this.CostumeSplitContainer.Panel2.Controls.Clear();

            this.tabPage1.Text = "Asset Tricks File";

            this.Text = "Asset Tricks Editor: " + file_name;

            assetEditor.AssetTricks at = new COH_CostumeUpdater.assetEditor.AssetTricks(file_name, ref this.logTxBx,this.openFileDialog1);

            at.Dock = DockStyle.Fill;

            this.CostumeSplitContainer.Panel2.Controls.Add(at);

            this.CostumeSplitContainer.ResumeLayout(false);

            this.ResumeLayout(false);

            loadedObject = "assetsTrick";

            fxLauncher.selectFxFile(file_name);

            currentLoadedFileName = file_name;

            registerUser("AssetsEditor", file_name);
        }

        private bool validateUser()
        {
            string valid_aurasToolStripMenuItem_User = "user";
            string valid_aurasToolStripMenuItem_User2 = "user";
            
            System.Security.Principal.WindowsIdentity user;
            
            user = System.Security.Principal.WindowsIdentity.GetCurrent();
            MessageBox.Show(user.Name);
            if (user.Name.ToLower().EndsWith(valid_aurasToolStripMenuItem_User))
            {
                return true;
            }
            else if (user.Name.ToLower().EndsWith(valid_aurasToolStripMenuItem_User2))
            {
                return true;
            }
            else if (user.Name.ToLower().EndsWith("user"))
            {
                return true;
            }

            return false;
        }

        private void loadToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            bool enable = true;

            if (this.CostumeSplitContainer.Panel2.Controls.Count > 0 && !fileSaved)
            {
                DialogResult dr = MessageBox.Show("The loaded file was not saved!\r\nDo you want to continue Loading new File?",
                    "Warning!", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);

                if (dr == DialogResult.Yes)
                {
                    enable = true;
                    fileSaved = true;
                }
                else
                {
                    enable = false;
                }
            }

            foreach (ToolStripMenuItem mi in this.loadToolStripMenuItem.DropDownItems)
            {
                mi.Enabled = enable;
            }
        }

        private void newToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            bool enable = true;

            if (this.CostumeSplitContainer.Panel2.Controls.Count > 0 && !fileSaved)
            {
                DialogResult dr = MessageBox.Show("The loaded file was not saved!\r\nDo you want to continue Loading new File?",
                    "Warning!", MessageBoxButtons.YesNo, MessageBoxIcon.Warning);

                if (dr == DialogResult.Yes)
                {
                    enable = true;
                    fileSaved = true;
                }
                else
                {
                    enable = false;
                }
            }

            foreach (ToolStripMenuItem mi in this.newToolStripMenuItem.DropDownItems)
            {
                mi.Enabled = enable;
            }
        }

        private void loadRegionCTM_MenuItem_Click(object sender, EventArgs e)
        {
            
            if (!this.openFileDialog1.InitialDirectory.ToLower().Contains(@"data\menu\Costume".ToLower()))
            {
                if (rootPath != null)
                    openFileDialog1.InitialDirectory = rootPath + @"menu\Costume";
                else
                    openFileDialog1.InitialDirectory = @"C:\game\data\fx";

                //this.openFileDialog1.InitialDirectory = @"C:\game\data\menu\Costume";
            }
            if (fxLauncher != null)
            {
                this.CostumeSplitContainer.Panel1.Controls.Remove(this.fxLauncher);
                this.fxLauncher.Dispose();
                this.fxLauncher = null;
            }
            openFileDialog1.FileName = "Regions.ctm";
                       
            DialogResult dR = openFileDialog1.ShowDialog();

            if (dR == DialogResult.OK)
            {
                currentLoadedFileName = openFileDialog1.FileName;

                this.openFileDialog1.InitialDirectory = Path.GetDirectoryName(this.openFileDialog1.FileName);
                
                auras.AurasPropagator aProp = new COH_CostumeUpdater.auras.AurasPropagator(this.openFileDialog1.FileName, ref this.logTxBx);
                
                aProp.Dock = DockStyle.Fill;
                
                this.CostumeSplitContainer.SuspendLayout();
                
                this.CostumeSplitContainer.Panel2.Controls.Clear();
                
                this.CostumeSplitContainer.Panel2.Controls.Add(aProp);
                
                this.CostumeSplitContainer.Panel2.ResumeLayout(false);

                loadedObject = "RegionsCTM";

                this.tabPage1.Text = "RegionsCTM File";

                this.Text = "RegionsCTM Editor: " + openFileDialog1.FileName;

                registerUser("AurasEditor", this.openFileDialog1.FileName);
                
                fileSaved = false;
            }
        }

        private void newFXToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();

            sfd.Filter = "FX file (*.FX)|*.fx";

            string iniDir = rootPath;// assetsMangement.CohAssetMang.p4GetWorkspace();
            
            if(iniDir != null)
                sfd.InitialDirectory = iniDir + @"fx";//@"data\fx";
            else
                sfd.InitialDirectory = @"C:\game\data\fx";

            DialogResult dR = sfd.ShowDialog();

            if (dR == DialogResult.OK)
            {
                ArrayList newFx = new ArrayList();
                newFx.Add("###########################################################");
                newFx.Add(string.Format("#{0}#",sfd.FileName));
                newFx.Add("###########################################################");
                newFx.Add("FxInfo");
                newFx.Add(" ");
                newFx.Add(" ");
                newFx.Add("###########################################################");
                newFx.Add(" ");
                newFx.Add(" ");
                newFx.Add(" ");
                newFx.Add("End");

                common.COH_IO.writeDistFile(newFx, sfd.FileName);

                this.openFileDialog1.InitialDirectory = Path.GetDirectoryName(sfd.FileName);

                currentLoadedFileName = sfd.FileName;

                this.loadFxFile(sfd.FileName);

                fileSaved = false;
            }
        }

        private void newAssetsTrickToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();

            sfd.Filter = "Assets Trick document (*.txt)|*.txt";

            string iniDir = rootPath; //assetsMangement.CohAssetMang.p4GetWorkspace();

            if (iniDir != null)
                sfd.InitialDirectory = iniDir + @"tricks";// + @"data\tricks";
            else
                sfd.InitialDirectory = @"C:\game\data\tricks";

            DialogResult dR = sfd.ShowDialog();

            if (dR == DialogResult.OK)
            {
                ArrayList newFx = new ArrayList();
                newFx.Add("###########################################################");
                newFx.Add(string.Format("#{0}#", sfd.FileName));
                newFx.Add("###########################################################");
                newFx.Add(" ");

                common.COH_IO.writeDistFile(newFx, sfd.FileName);

                this.openFileDialog1.InitialDirectory = Path.GetDirectoryName(sfd.FileName);

                currentLoadedFileName = sfd.FileName;

                loadAssetsTrick(sfd.FileName);

                fileSaved = false;
            }
        }

        private void loadFXToolStripMenueItem_Click(object sender, System.EventArgs e)
        {
            openFileDialog1.FileName = "";

            string filter = openFileDialog1.Filter;

            openFileDialog1.Filter = "FX file (*.FX)|*.fx";

            string iniDir = rootPath;

            if (iniDir != null)                 
            {
                if(!openFileDialog1.InitialDirectory.ToLower().Contains(iniDir.ToLower() + @"fx"))
                    openFileDialog1.InitialDirectory = iniDir + @"fx";
            }
            else
                openFileDialog1.InitialDirectory = @"C:\game\data\fx";

            DialogResult dR = openFileDialog1.ShowDialog();

            if (dR == DialogResult.OK)
            {
                currentLoadedFileName = openFileDialog1.FileName;
                this.openFileDialog1.InitialDirectory = Path.GetDirectoryName(this.openFileDialog1.FileName);
                this.loadFxFile(this.openFileDialog1.FileName);
                fileSaved = false;
            }

            openFileDialog1.Filter = filter;
        } 

        private void loadPartToolStripMenueItem_Click(object sender, System.EventArgs e)
        {
            openFileDialog1.FileName = "";

            string filter = openFileDialog1.Filter;

            openFileDialog1.Filter = "Part file (*.Part)|*.part";

            string iniDir = rootPath;//assetsMangement.CohAssetMang.p4GetWorkspace();

            if (iniDir != null)
                openFileDialog1.InitialDirectory = iniDir + @"fx";// + @"data\fx";
            else
                openFileDialog1.InitialDirectory = @"C:\game\data\fx";

            DialogResult dR = openFileDialog1.ShowDialog();

            if (dR == DialogResult.OK)
            {
                currentLoadedFileName = openFileDialog1.FileName;
                this.openFileDialog1.InitialDirectory = Path.GetDirectoryName(this.openFileDialog1.FileName);
                this.loadPartFile(this.openFileDialog1.FileName);
                fileSaved = false;
            }

            openFileDialog1.Filter = filter;
        }       

        private void loadObjectTrickToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            openFileDialog1.FileName = "";

            string initDir = openFileDialog1.InitialDirectory;

            string filter = openFileDialog1.Filter;

            openFileDialog1.Filter = "Assets Trick document (*.txt)|*.txt";

            if (!this.openFileDialog1.InitialDirectory.ToLower().Contains(@"data\tricks".ToLower()))
            {
                if (rootPath != null)
                    openFileDialog1.InitialDirectory = rootPath + @"tricks";
                else
                    this.openFileDialog1.InitialDirectory = @"C:\game\data\tricks";
            }

            DialogResult dR = openFileDialog1.ShowDialog();

            if (dR == DialogResult.OK)
            {
                currentLoadedFileName = openFileDialog1.FileName;
                this.openFileDialog1.InitialDirectory = Path.GetDirectoryName(this.openFileDialog1.FileName);
                loadAssetsTrick(this.openFileDialog1.FileName);
                fileSaved = false;
            }

            openFileDialog1.Filter = filter;
        }

        private void loadCoToolStripMenuItem_Click(object sender, System.EventArgs e)
        {
            openFileDialog1.FileName = "";

            string initDir = openFileDialog1.InitialDirectory;

            if (!initDir.ToLower().Contains(@"data\menu\costume"))
            {
                if (rootPath != null)
                    openFileDialog1.InitialDirectory = rootPath + @"menu\Costume";
                else
                    openFileDialog1.InitialDirectory = @"C:\game\data\menu\Costume";
            }
            if (fxLauncher != null)
            {
                this.CostumeSplitContainer.Panel1.Controls.Remove(this.fxLauncher);
                this.fxLauncher.Dispose();
                this.fxLauncher = null;
            }
            DialogResult dR = openFileDialog1.ShowDialog();

            if (dR == DialogResult.OK)
            {
                currentLoadedFileName = openFileDialog1.FileName;

                this.CostumeSplitContainer.Panel2.Controls.Clear();

                this.openFileDialog1.InitialDirectory = Path.GetDirectoryName(this.openFileDialog1.FileName);

                if (cEditor != null)
                {
                    cEditor.Dispose();

                    cEditor = null;
                }

                cEditor = new COH_CostumeUpdater.costume.CostumeEditor(this.openFileDialog1.FileName, ref this.logTxBx);

                cEditor.Dock = DockStyle.Fill;

                this.CostumeSplitContainer.Panel2.Controls.Add(cEditor);

                loadedObject = "CostumeFile";

                this.tabPage1.Text = "Costume File";

                this.Text = "CostumeFile Editor: " + openFileDialog1.FileName;                

                registerUser("CostumeEditor", openFileDialog1.FileName);
                
                fileSaved = false;
            }
        }

        private void saveAsLoadedFile_Click(object sender, System.EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();

            string sfdFilter = "";

            switch (loadedObject)
            {
                case "assetsTrick":
                    sfdFilter = "Assets Trick File (*.txt) | *.txt";
                    break;

                case "CostumeFile":
                    saveLoadedFile_Click(sender, e);
                    fileSaved = true;
                    return;

                case "fxFile":
                    sfdFilter = "FX File (*.fx) | *.fx";
                    break;

                default:
                    MessageBox.Show("Nothing saved!\r\nSaving This Type of file is not Implemented through this action.\r\nNo File was loaded", "Warning");
                    return;
            }

            sfd.Filter = sfdFilter;

            if (loadedObject != null)
            {
                if (currentLoadedFileName != null)
                {
                    sfd.InitialDirectory = Path.GetDirectoryName(this.currentLoadedFileName);

                    sfd.FileName = Path.GetFileName(this.currentLoadedFileName);
                }

                DialogResult dr = sfd.ShowDialog();

                if (dr == DialogResult.OK)
                {
                    saveLoadedFile_Click(sfd, e);
                    fileSaved = true;
                }
            }
        }

        private void COH_CostumeUpdaterForm_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (e.KeyCode == System.Windows.Forms.Keys.S &&
                e.Modifiers == System.Windows.Forms.Keys.Control)
            {
                saveLoadedFile_Click(sender, e);
            }
        }

        private void saveLoadedFile_Click(object sender, System.EventArgs e)
        {
            string file_name = null;

           switch (loadedObject)
           {
               case "assetsTrick":
                   assetEditor.AssetTricks assetTrick = (assetEditor.AssetTricks)this.CostumeSplitContainer.Panel2.Controls[0];
                   if (sender.GetType() == typeof(SaveFileDialog))
                   {
                       file_name = ((SaveFileDialog)sender).FileName;

                       assetTrick.saveAs(file_name);

                       if (File.Exists(file_name))
                       {
                           loadAssetsTrick(file_name);
                       }
                   }
                   else
                   {
                       assetTrick.save(false);
                   }
                   fileSaved = true;
                   break;

               case "CostumeFile":
                   if (cEditor != null)
                   {
                       cEditor.saveLoadedFile();

                       fileSaved = true;
                   }
                   else
                       MessageBox.Show("Nothing saved! No Costume File was loaded", "Warning");
                   break;

               case "fxFile":
                   if (fxLauncher != null)
                   {
                       fxEditor.FX_FileWin ffw = (fxEditor.FX_FileWin)this.CostumeSplitContainer.Panel2.Controls[0];
                       if (sender.GetType() == typeof(SaveFileDialog))
                       {
                           file_name = ((SaveFileDialog)sender).FileName;

                           ffw.saveAs(file_name);

                           if (File.Exists(file_name))
                           {
                               loadFxFile(file_name);
                           }
                       }
                       else
                       {
                           ffw.save();
                       }

                       fileSaved = true;
                   }
                   else
                       MessageBox.Show("Nothing saved! No Fx File was loaded", "Warning");
                   break;
                   
               case "RegionsCTM":
                   MessageBox.Show("Nothing saved! No Costume File was Saved", "Warning");
                   break;

               default:
                   MessageBox.Show("Nothing saved!\r\nSaving This Type of file is not Implemented through this action.\r\nNo File was loaded", "Warning");
                   break;
           }
        }

        private void cohTabControl_Selected(object sender, System.Windows.Forms.TabControlEventArgs e)
        {
            if (e.TabPageIndex == 1)
            {
                this.Refresh();
            }
        }
       
        private void showReadOnlyFilesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string senderText = ((ToolStripMenuItem)sender).Text;
            string searchPath = @"C:\game";
            //if (senderText.Equals("Show Game Data RO Files"))
                //searchPath = @"C:\game\data\";
            Cursor crs = this.Cursor;
            this.Cursor = System.Windows.Forms.Cursors.WaitCursor;
            if (this.ckoFiles != null)
            {
                this.ckoFiles.Close();
                this.ckoFiles.Dispose();
            }
            ckoFiles = new assetsMangement.ChkdOutFiles(searchPath, this.logTxBx);
            this.Cursor = crs;
           
        }
        
        private void viewTextureFile(object sender, EventArgs e)
        {
            string iDir = openFileDialog1.InitialDirectory;

            string dFilter = openFileDialog1.Filter;
            
            openFileDialog1.RestoreDirectory = false;

            openFileDialog1.FileName = "";

            if (!iDir.ToLower().Contains(@"data\texture_library"))
            {
                string pathRoot = iDir.Substring(0, iDir.IndexOf(@"\data"));
                openFileDialog1.InitialDirectory = pathRoot + @"\data\texture_library";
            }
            
            openFileDialog1.Filter = "Texture Files (*.texture) |*.texture";

            DialogResult dr = openFileDialog1.ShowDialog();

            if (dr == DialogResult.OK)
            {
                string fileName = openFileDialog1.FileName;
                
                openFileDialog1.InitialDirectory = Path.GetDirectoryName(fileName);
                
                viewTexture(fileName);
            }
           
            openFileDialog1.Filter = dFilter;
        }

        private void viewTexture(string fileName)
        {
            viewTexture(fileName, true);
        }   
     
        private void viewTexture(string fileName, bool alphaOn)
        {
            costume.ddsViewer dd = new costume.ddsViewer(fileName, alphaOn);

            dd.Show();

            //dd.ShowDialog();
            
            //dd.Dispose();   
        }

    }
}
