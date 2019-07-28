using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using System.IO;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace FX_TreeParser
{
    public partial class FX_TreeParser : Form
    {
        string FX_File;
        string root;

        public FX_TreeParser(string filePath)
        {
            this.FX_File = filePath;
            this.root = @"C:\game\";

            if (this.FX_File.ToLower().Contains("gamefix2"))
                this.root = @"C:\gamefix2\";

            else if (this.FX_File.ToLower().Contains("gamefix"))
                this.root = @"C:\gamefix\";

            InitializeComponent();
            Cursor c = this.Cursor;
            this.Cursor = Cursors.WaitCursor;
            ImageList iml = new ImageList();
            iml.ImageSize = new Size(16, 16);
            iml.Images.Add(COH_CostumeUpdater.Properties.Resources.FXIcon);
            iml.Images.Add(COH_CostumeUpdater.Properties.Resources.PARTIcon);
            iml.Images.Add(COH_CostumeUpdater.Properties.Resources.BHVRIcon);
            iml.Images.Add(COH_CostumeUpdater.Properties.Resources.TGAIcon);
            iml.Images.Add(COH_CostumeUpdater.Properties.Resources.GEOIcon);

            treeView1.ImageList = iml;

            TreeNode tn = new TreeNode(Path.GetFileName(filePath));
            setTNicon(ref tn);
            tn.Tag = filePath;
            tn.ToolTipText = filePath;
            treeView1.Nodes.Add(tn);
            praseFile(filePath, tn);

            this.Cursor = c;

            treeView1.ExpandAll();
        }
        private void setTNicon(ref TreeNode tn)
        {
            if (tn.Text.ToLower().EndsWith(".fx"))
                tn.ImageIndex = 0;

            else if (tn.Text.ToLower().EndsWith(".part"))
                tn.ImageIndex = 1;

            else if (tn.Text.ToLower().EndsWith(".bhvr"))
                tn.ImageIndex = 2;

            else if (tn.Text.ToLower().Contains(".tga "))
                tn.ImageIndex = 3;
            else
                tn.ImageIndex = 4;

            tn.SelectedImageIndex = tn.ImageIndex;
        }
        public void praseFile(string path, TreeNode tn)
        {
            read(this.FX_File, tn);
        }
        private string removeConWspace(string str)
        {
            char[] ca = str.ToCharArray();
            string nStr = "";
            int i = 0;
            char k;

            foreach (char c in ca)
            {
                if (c == ' ')
                    i++;
                else
                    i = 0;
                if (i > 1)
                    continue;

                nStr += c;
            }
            return nStr;
        }
        private void read(string path, TreeNode tn)
        {
            StreamReader sr = File.OpenText(path);

            string s = "";

            while ((s = sr.ReadLine()) != null)
            {
                string str = s.Replace('\t',' ').Trim();

                if (!str.StartsWith("#") &&

                    (str.ToLower().Contains("geo") ||
                    str.ToLower().EndsWith(".fx") ||
                    str.ToLower().EndsWith(".part") ||
                    str.ToLower().EndsWith(".tga") ||
                    str.ToLower().EndsWith(".bhvr")))
                {
                    if (!str.ToLower().Contains("tintgeom"))
                    {

                        Color forClr = Color.Red;

                        string ctnText = "";

                        str = removeConWspace(str);
                        
                        string[] strSplt = str.Split(" ".ToCharArray());

                        if (str.Contains("\t"))
                            strSplt = str.Split("\t".ToCharArray());

                        string incFile = strSplt[1];

                        if (incFile.Contains("#"))
                        {
                            if (incFile.StartsWith("#"))
                            {
                                forClr = Color.DarkGreen;
                                ctnText = incFile;
                                incFile = incFile.Substring(incFile.IndexOf("#") + 1);
                            }
                            else
                            {
                                ctnText = incFile.Replace(":", "");
                                incFile = incFile.Substring(0, incFile.IndexOf("#")-1);
                            }
                        }


                        string nPath = this.root + @"data\fx\" + incFile.Replace(":", "");

                        if (incFile.Contains(":"))
                            nPath = Path.GetDirectoryName(path) + "\\" + incFile.Replace(":", "");

                        if (incFile.ToLower().Contains(".tga"))
                        {
                            nPath = findTgaPath(incFile);
                            ctnText = incFile;
                            if (File.Exists(nPath))
                            {
                                ctnText = ctnText + "   " +viewTga(nPath, false);
                            }
                        }

                        if (!incFile.Contains("."))
                            nPath = findWRLPath(incFile+".wrl");

                        if (ctnText == "")
                            ctnText = Path.GetFileName(nPath);

                        TreeNode ctn = new TreeNode(ctnText);

                        setTNicon(ref ctn);
                        ctn.Tag = nPath;
                        ctn.ToolTipText = nPath;
                        if (File.Exists(nPath))
                        {
                            if (!str.ToLower().EndsWith(".tga") && !nPath.ToLower().EndsWith(".wrl"))
                                read(nPath, ctn);
                        }
                        else
                            ctn.ForeColor = forClr;

                        tn.Nodes.Add(ctn);
                    }
                }
            }
            sr.Close();
        }
        private string findTgaPath(string path)
        {
            return findFxPath(path, root + @"src\Texture_Library\fx", "*.tga");
        }
        private string findWRLPath(string path)
        {
            return findFxPath(path, root + @"src\object_library\fx", "*.wrl");

        }
        private string findFxPath(string path, string searchLocation,string ext)
        {
            string nPath = "";

            string[] files = Directory.GetFiles(searchLocation, ext, System.IO.SearchOption.AllDirectories);

            foreach (string p in files)
            {
                if (p.ToLower().Contains(path.ToLower()))
                {
                    nPath = p;
                    break;
                }
            }
            if (nPath == "")
            {
                if(searchLocation.EndsWith(@"\"))
                    nPath = searchLocation + path;
                else
                    nPath = searchLocation + @"\" + path;
            }
            return nPath;
        }

        void ctxMn_Popup(object sender, System.EventArgs e)
        {
            Point p = this.treeView1.PointToClient(Control.MousePosition);
            TreeNode tn = this.treeView1.GetNodeAt(p);
            this.treeView1.SelectedNode = tn;

            try
            {
                if (tn.Text.ToLower().Contains(".tga"))
                {
                    this.ctxMn.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                    this.menuItem1});
                }
                else
                {
                    this.ctxMn.MenuItems.Clear();
                }
            }
            catch (Exception ex)
            {
            }
        }


        void menuItem1_Click(object sender, System.EventArgs e)
        {
            string path = (string)this.treeView1.SelectedNode.Tag;
            if (File.Exists(path))
            {
                string psPath = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
                string[] files = Directory.GetFiles(psPath + "\\Adobe", "Photoshop.exe", System.IO.SearchOption.AllDirectories);

                if (files.Length > 0)
                {
                    psPath = files[0];

                    if (File.Exists(psPath))
                    {
                        System.Diagnostics.Process.Start(psPath, path);
                    }
                }
            }
        }
        private string viewTga(string path, bool show)
        {
            string results = "";
            tgaBox tbx = new tgaBox(path);
            if (show)
                tbx.ShowDialog();
            else
            {
                results = tbx.getImageSize();
            }
            return results;
        }
        void treeView1_NodeMouseDoubleClick(object sender, System.Windows.Forms.TreeNodeMouseClickEventArgs e)
        {
            string tFx = (string)e.Node.Tag;

            if (File.Exists(tFx))
            {
                if (tFx.ToLower().EndsWith(".tga"))
                    viewTga(tFx, true);
                else
                    System.Diagnostics.Process.Start(tFx);
            }
        }
    }
}
