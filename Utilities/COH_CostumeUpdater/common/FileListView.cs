using System;
using System.IO;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.common
{
    class FileListView:ListView
    {
        private string strDirectory;
        public FileListView()
        {
            this.View = System.Windows.Forms.View.Details;
            ImageList fImageList = new ImageList();
            fImageList.Images.Add(COH_CostumeUpdater.Properties.Resources.DOC);
            fImageList.Images.Add(COH_CostumeUpdater.Properties.Resources.EXE);
            fImageList.Images.Add(COH_CostumeUpdater.Properties.Resources.DOCR);
            fImageList.Images.Add(COH_CostumeUpdater.Properties.Resources.nDBDOC);
            fImageList.Images.Add(COH_CostumeUpdater.Properties.Resources.CLSDFOLD);

            this.SmallImageList = fImageList;
            this.LargeImageList = fImageList;
            this.Columns.Add("Name", 100, HorizontalAlignment.Left);
            this.Columns.Add("Size", 100, HorizontalAlignment.Right);
            this.Columns.Add("Modified", 100, HorizontalAlignment.Left);
            this.Columns.Add("Attribute", 100, HorizontalAlignment.Left);
        }

        protected override void  OnItemActivate(EventArgs e)
        {
 	        base.OnItemActivate(e);
            foreach(ListViewItem lvi in SelectedItems)
            {
                Path.Combine(strDirectory, lvi.Text);
            }
        }
        public void fillInfo(ref ListViewItem lvi, string fullPath)
        {
            FileInfo fi = new FileInfo(fullPath);
            switch (Path.GetExtension(fi.Name).ToUpper())
            {
                case ".EXE":
                    lvi.ImageIndex = 1;
                    break;
                default:
                    lvi.ImageIndex = 0;
                    break;
            }

            lvi.SubItems.Add(fi.Length.ToString("N0"));
            lvi.SubItems.Add(fi.LastWriteTime.ToString());
            string strAttr = "";

            if ((fi.Attributes & FileAttributes.Archive) != 0)
                strAttr += "A";

            if ((fi.Attributes & FileAttributes.Hidden) != 0)
                strAttr += "H";

            if ((fi.Attributes & FileAttributes.System) != 0)
                strAttr += "S";

            if ((fi.Attributes & FileAttributes.ReadOnly) != 0)
            {
                strAttr += "R";
                lvi.ImageIndex = 2;
            }
            else
            {
                string dbRoot = @"N:\revisions\";
                string path = Path.GetDirectoryName(fi.FullName);

                string dbPath = path.Replace(@"C:\game\", dbRoot);
                bool isGamefx2 = path.Contains(@"C:\gamefix2\");
                bool isGamefx = path.Contains(@"C:\gamefix\");

                if (isGamefx2)
                    dbPath = path.Replace(@"C:\gamefix\", dbRoot);

                else if (isGamefx)
                    dbPath = path.Replace(@"C:\gamefix\", dbRoot);

                DirectoryInfo tmpDI = new DirectoryInfo(dbPath);
                FileInfo[] tmpFi = tmpDI.GetFiles(fi.Name + @"_vb*");

                if (tmpFi.Length == 0)
                    lvi.ImageIndex = 3;
            }

            lvi.SubItems.Add(strAttr);
        }

        public void fillInfo(ref ListViewItem lvi, FileSystemInfo fsi)
        {
            lvi.Tag = fsi.FullName;
            lvi.Name = fsi.Name;

            if ((fsi.Attributes & FileAttributes.Directory) == FileAttributes.Directory )
            {
                lvi.ImageIndex = 4;
            }
            else
            {
                switch (Path.GetExtension(fsi.Name).ToUpper())
                {
                    case ".EXE":
                        lvi.ImageIndex = 1;
                        break;
                    default:
                        lvi.ImageIndex = 0;
                        break;
                }
                FileInfo fi = new FileInfo(fsi.FullName);
                lvi.SubItems.Add(fi.Length.ToString("N0"));
                lvi.SubItems.Add(fsi.LastWriteTime.ToString());
                string strAttr = "";
                if ((fsi.Attributes & FileAttributes.Archive) == FileAttributes.Archive)
                    strAttr += "A";

                if ((fsi.Attributes & FileAttributes.Hidden) == FileAttributes.Hidden)
                    strAttr += "H";

                if ((fsi.Attributes & FileAttributes.System) == FileAttributes.System)
                    strAttr += "S";

                if ((fsi.Attributes & FileAttributes.ReadOnly) == FileAttributes.ReadOnly)
                {
                    strAttr += "R";
                    lvi.ImageIndex = 2;
                }
                else
                {
                    bool isDBFile = COH_CostumeUpdater.assetsMangement.CohAssetMang.isDBfile(fsi.FullName);
                    if (!isDBFile)//(tmpFi.Length == 0)
                        lvi.ImageIndex = 3;
                }

                lvi.SubItems.Add(strAttr);
            }
        }

        public void ShowFiles(string strDirectory)
        {
            this.strDirectory = strDirectory;
            Items.Clear();
            DirectoryInfo diDirectories = new DirectoryInfo(strDirectory);
            try
            {
                /*FileInfo[] afiFiles = diDirectories.GetFiles();
                if (afiFiles.Length > 0)
                {
                    foreach (FileInfo fi in afiFiles)
                    {
                        ListViewItem lvi = new ListViewItem(fi.Name);
                        fillInfo(ref lvi, fi.FullName);
                        Items.Add(lvi);
                    }
                }
                else*/
                {
                    FileSystemInfo[] fsis = diDirectories.GetFileSystemInfos();
                    foreach (FileSystemInfo fsi in fsis)
                    {
                        ListViewItem lvi = new ListViewItem(fsi.Name);
                        fillInfo(ref lvi, fsi);
                        Items.Add(lvi);
                    }
                }
                
            }
            catch (Exception exp)
            { }
        }
    }
}
