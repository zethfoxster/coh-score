using System;
using System.Drawing;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.common
{
    class DirectoryTreeView:TreeView
    {
        public DirectoryTreeView()
        {
            this.Width *=2;
            ImageList cohImageList = new ImageList();
            cohImageList.Images.Add(COH_CostumeUpdater.Properties.Resources._35FLOPPY);
            cohImageList.Images.Add(COH_CostumeUpdater.Properties.Resources.CLSDFOLD);
            cohImageList.Images.Add(COH_CostumeUpdater.Properties.Resources.OPENFOLD);
            this.ImageList = cohImageList;
            RefreshTree();
        }

        protected override void OnBeforeExpand(TreeViewCancelEventArgs e)
        {
            base.OnBeforeExpand(e);
            this.BeginUpdate();

            foreach(TreeNode tn in e.Node.Nodes)
            {
                AddDirectories(tn);
            }
            this.EndUpdate();
        }

        private void AddDirectories(TreeNode tn)
        {
            tn.Nodes.Clear();
            string strPath = tn.FullPath;
            DirectoryInfo diDirectory = new DirectoryInfo(strPath);
            try
            {
                DirectoryInfo[] diArr = diDirectory.GetDirectories();
                foreach(DirectoryInfo dri in diArr)
                {
                    TreeNode tnDir = new TreeNode(dri.Name, 1, 2);
                    tnDir.Name = dri.Name;
                    tn.Nodes.Add(tnDir);
                }
            }
            catch(Exception exp)
            {                
            }

        }

        public void RefreshTree()
        {
            this.BeginUpdate();
            this.Nodes.Clear();
            //string [] astrDrives = Directory.GetLogicalDrives();
            string[] astrDrives = { @"C:\gamefix\data",@"C:\gamefix2\data", @"C:\game\data" };
            foreach(string strDrive in astrDrives)
            {
                TreeNode tnDrive = new TreeNode(strDrive,0,0);                
                Nodes.Add(tnDrive);
                AddDirectories(tnDrive);
                if(strDrive.Equals(@"C:\"))
                    this.SelectedNode = tnDrive;
            }

            this.EndUpdate();
        }
    }
}
