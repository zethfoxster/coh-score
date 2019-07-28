using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.assetsMangement
{
    public partial class ChkdOutFiles : Form
    {
        public ChkdOutFiles()
        {
            InitializeComponent();
        }
        public ChkdOutFiles(string searchPath, System.Windows.Forms.RichTextBox logTextBox)
        {
            InitializeComponent();
            this.logTxBox = logTextBox;
            
            ImageList fImageList = new ImageList();
            fImageList.Images.Add(COH_CostumeUpdater.Properties.Resources.DOC);
            int formWidth = this.Width;
            int nameWd = (int)(formWidth/5.0f);
            int locationWd = formWidth - nameWd -195;
            this.cofListView.Columns.Add("Name", nameWd, HorizontalAlignment.Left);
            this.cofListView.Columns.Add("Size", 75, HorizontalAlignment.Right);
            this.cofListView.Columns.Add("User", 75, HorizontalAlignment.Center);
            this.cofListView.Columns.Add("Modified", 120, HorizontalAlignment.Left);
            this.cofListView.Columns.Add("Location", locationWd, HorizontalAlignment.Left);
            this.cofListView.View = View.Details;
            this.cofListView.SmallImageList = fImageList;
            this.cofListView.LargeImageList = fImageList;
            this.checkedOutFiless = new System.Collections.ArrayList();
            displayCheckedOutFiles(searchPath);
        }
        private void getP4openedFiles()
        {
            string list = assetsMangement.CohAssetMang.listCheckedout();
            string[] listOutFiles = list.Replace("\r","").Split('\n');
            foreach(string line in listOutFiles)
            {
                this.checkedOutFiless.Add(line);
            }
        }
        private void getCheckoutFiles(string searchPath)
        {
            string tmpPath = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            tmpPath += @"\Temp\nROfiles.txt";
            logTxBox.SelectionColor = this.logTxBox.ForeColor;
            logTxBox.SelectedText = "TempFilePath: \"" + tmpPath + "\"\r\n";
            System.Diagnostics.Process cmdProc = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo cmdProcInfo = new System.Diagnostics.ProcessStartInfo("cmd.exe");
            //cmdProcInfo.UseShellExecute = false;
            //cmdProcInfo.CreateNoWindow = true;
            //cmdProcInfo.RedirectStandardOutput = true;
            //to run any dos command use '/c' before the command and its arguments
            //cmdProcInfo.Arguments = @"/c dir /b /A:-R-H-D-S /S " + searchPath;
            cmdProcInfo.Arguments = @"/c dir /b /A:-R-H-D-S /S " + searchPath + "> " + tmpPath;
            cmdProc.StartInfo = cmdProcInfo;
            cmdProc.Start();
            //StreamReader cmdProcSR = cmdProc.StandardOutput;
            int elapsedTime = 0;
            while (!cmdProc.HasExited)
            {
                this.searchProgBar.PerformStep();
                this.searchProgBar.Invalidate();
                this.searchProgBar.Refresh();
                this.Refresh();
                if (elapsedTime > (1000 * 60))
                    cmdProc.Kill();

                System.Threading.Thread.Sleep(5);
                elapsedTime += 5;
                if (this.searchProgBar.Value == this.searchProgBar.Maximum)
                {
                    this.searchProgBar.Value = 0;
                }
            }
            
            if(cmdProc != null)
                cmdProc.Close();

            System.Collections.ArrayList mList = new System.Collections.ArrayList();
            if (System.IO.File.Exists(tmpPath))
            {

                StreamReader cmdProcSR = new StreamReader(tmpPath);
                string line;
                while ((line = cmdProcSR.ReadLine()) != null)
                {
                    if (isDBfile(line))
                        this.checkedOutFiless.Add(line);
                }
                cmdProcSR.Close();
            }
            else
            {
                logTxBox.SelectionColor = this.logTxBox.ForeColor;
                logTxBox.SelectedText = "60 sec has elapsed before search is completed\r\n";
            }
        }
        private void displayCheckedOutFiles(string searchPath)
        {
            this.Show();
            //getCheckoutFiles(searchPath);
            getP4openedFiles();
            this.searchProgBar.Hide();
            
            if (this.checkedOutFiless.Count > 0)
            {
                int i = 0;
                foreach (object obj in this.checkedOutFiless)
                {
                    string ckoFile = (string)obj;

                    System.Security.Principal.WindowsIdentity user = System.Security.Principal.WindowsIdentity.GetCurrent();
                    string fileName = ckoFile.Replace("//" + user.Name.Replace("PRG\\","").ToUpper(), @"C:");
                    
                    if (File.Exists(fileName))
                    {
                        FileInfo fi = new FileInfo(fileName);
                        ListViewItem lvi = new ListViewItem(fi.Name);
                        lvi.Name = fi.Name;
                        lvi.Tag = fileName;
                        lvi.ToolTipText = ckoFile;
                        lvi.ImageIndex = 0;
                        lvi.SubItems.Add(fi.Length.ToString("N0"));
                        lvi.SubItems.Add(ckoFile.Replace("//","").Replace(fileName.Replace("C:",""),""));
                        lvi.SubItems.Add(fi.LastWriteTime.ToString());
                        lvi.SubItems.Add(fi.DirectoryName);
                        if(i%2 != 0)
                        {
                            lvi.BackColor = SystemColors.ControlLight;
                        }
                        i++;
                        this.cofListView.Items.Add(lvi);
                    }
                }
                this.cofListView.Alignment = ListViewAlignment.Left;
                this.Refresh();                  
            }
            else
            {
                this.Hide();
                MessageBox.Show("There are no files checkedout by you!", "Files Checked out by you");
            }
        }
        private bool isDBfile(string path)
        {
            return COH_CostumeUpdater.assetsMangement.CohAssetMang.isDBfile(path);
        }

        private void undoCheckoutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string files = "";
            foreach (ListViewItem lvi in this.cofListView.Items)
            {
                if (lvi.Selected)
                {
                    files += lvi.Tag + " ";
                    this.cofListView.Items.Remove(lvi);
                }

            }
            if (files.Length > 2)
            {
                string results = CohAssetMang.undoCheckout(files);
                string filesList = results.Substring(results.ToLower().IndexOf(@"c:\"));
                string cmd = results.Substring(0, results.ToLower().IndexOf(@"c:\"));
                foreach (string str in filesList.Split(" ".ToCharArray()))
                {
                    if (!str.ToLower().Contains("\r\n"))
                    {
                        logTxBox.SelectionColor = this.logTxBox.ForeColor;
                        logTxBox.SelectedText = cmd + str + "\"\r\n";
                    }
                }
            }
            else
                MessageBox.Show("Nothing selected", "Warrning");
        }

        private void openFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string folder = (string)this.cofListView.SelectedItems[0].SubItems[4].Text;
            System.Diagnostics.Process cmdProc = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo cmdProcInfo = new System.Diagnostics.ProcessStartInfo("explorer");
            cmdProcInfo.UseShellExecute = false;
            cmdProcInfo.CreateNoWindow = true;
            cmdProcInfo.Arguments = folder;
            cmdProc.StartInfo = cmdProcInfo;
            cmdProc.Start();

        }

        private void openFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string files = "";
            foreach (ListViewItem lvi in this.cofListView.SelectedItems)
            {
                files += lvi.Tag + " ";

            }
            System.Diagnostics.Process cmdProc = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo cmdProcInfo = new System.Diagnostics.ProcessStartInfo("Uedit32");
            cmdProcInfo.UseShellExecute = false;
            cmdProcInfo.CreateNoWindow = true;
            cmdProcInfo.Arguments = files;
            cmdProc.StartInfo = cmdProcInfo;
            cmdProc.Start();
        }
    }
}
