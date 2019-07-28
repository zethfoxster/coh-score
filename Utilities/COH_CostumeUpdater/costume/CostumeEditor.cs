using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.costume
{
    public partial class CostumeEditor : Panel//Form
    {
        public CostumeEditor(string file_name,ref System.Windows.Forms.RichTextBox textBox)
        {
            this.fileName = file_name;

            this.logTxBx = textBox;

            InitializeComponent();

            this.imgeList.Images.AddRange(new Image[]{
                COH_CostumeUpdater.Properties.Resources.gset,
                COH_CostumeUpdater.Properties.Resources.geoP,               
                COH_CostumeUpdater.Properties.Resources.displayName,
                COH_CostumeUpdater.Properties.Resources.geo,
                COH_CostumeUpdater.Properties.Resources.fx,
                COH_CostumeUpdater.Properties.Resources.texture,
                COH_CostumeUpdater.Properties.Resources.key,
                COH_CostumeUpdater.Properties.Resources.tag,
                COH_CostumeUpdater.Properties.Resources.mask,
                COH_CostumeUpdater.Properties.Resources.include,
                COH_CostumeUpdater.Properties.Resources.geoPDevOnly,
                COH_CostumeUpdater.Properties.Resources.boneSet}
                );

            this.imgeList.ImageSize = new Size(24, 24);

            imgeList.TransparentColor = Color.White;

            loadCostumeTool();
        }

        void loadCostumeTool()
        {
            this.CostumeTV.ImageList = this.imgeList;

            if (buildCostumeTree())
                this.tvContextMenu.Popup += new EventHandler(tvContextMenue_Popup);
        }       

        bool buildCostumeTree()
        {
            if (ccf != null)
            {
                ccf = null;
            }
            
            ccf = new CharacterCostumeFile(fileName);
            
            string results = ccf.buildTree(this.CostumeTV, CostumeTV_NodeMouseDoubleClick);
            
            isCostumeFile = !results.Contains("ERROR:");

            if (!isCostumeFile)
            {
                logTxBx.SelectionColor = Color.Red;
                this.logTxBx.SelectedText = results;
            }
            else
            {
                this.CostumeTV_lable.Text = ccf.filePath;
                logTxBx.SelectionColor = logTxBx.ForeColor;
                logTxBx.SelectedText = results;
            }

            return isCostumeFile;
        }

        void buildCostumeTVcontextMenu()
        {
            this.tvContextMenu.MenuItems.Clear();

            System.Windows.Forms.MenuItem copyGeoP = new System.Windows.Forms.MenuItem();

            copyGeoP.Name = "copyGeoP";

            copyGeoP.Text = "Copy Selected Costume Piece";

            copyGeoP.Click += new EventHandler(copyCostume_Click);

            System.Windows.Forms.MenuItem pasteGeoP = new System.Windows.Forms.MenuItem();

            pasteGeoP.Name = "pasteGeoP";

            pasteGeoP.Text = "Paste under Selected Costume Piece";

            pasteGeoP.Click += new EventHandler(pasteCostume_Click);

            System.Windows.Forms.MenuItem editCostume = new System.Windows.Forms.MenuItem();

            editCostume.Name = "editCostume";

            editCostume.Text = "Edit Selected Costume Piece";

            editCostume.Click += new EventHandler(editCostume_Click);

            System.Windows.Forms.MenuItem addCostume = new System.Windows.Forms.MenuItem();

            addCostume.Name = "addCostume";

            addCostume.Text = "Add Costume Piece";

            addCostume.Click += new EventHandler(addCostume_Click);

            System.Windows.Forms.MenuItem propCostume = new System.Windows.Forms.MenuItem();

            propCostume.Name = "propCostume";

            propCostume.Text = "Propagate Geo Piece";

            propCostume.Click += new EventHandler(propCostume_Click);

            this.tvContextMenu.MenuItems.Add(copyGeoP);

            this.tvContextMenu.MenuItems.Add(pasteGeoP);
            
            this.tvContextMenu.MenuItems.Add(editCostume);

            this.tvContextMenu.MenuItems.Add(addCostume);

            this.tvContextMenu.MenuItems.Add(propCostume);
        }

        void tvContextMenue_Popup(object sender, EventArgs e)
        {
            Point p = this.CostumeTV.PointToClient(Control.MousePosition);

            TreeNode tn = this.CostumeTV.GetNodeAt(p);

            this.CostumeTV.SelectedNode = tn;

            try
            {
                if (tn.Tag.GetType().Name.ToLower().Contains("geopiece"))
                {
                    buildCostumeTVcontextMenu();

                    MenuItem[] mis = this.tvContextMenu.MenuItems.Find("pasteGeoP", false);

                    if (copySource == null)
                    {
                        mis[0].Enabled = false;
                    }
                    else
                    {
                        mis[0].Enabled = true;
                    }
                }
                else
                {
                    this.tvContextMenu.MenuItems.Clear();
                }
            }
            catch (Exception ex)
            {
            }

        }

        void editCostume_Click(object sender, EventArgs e)
        {
            TreeNode tn = CostumeTV.SelectedNode;

            ccf.editCostume_Piece(tn, this.CostumeTV, CostumeTV_NodeMouseDoubleClick);

            CostumeTV.SelectedNode = this.CostumeTV.Nodes.Find(tn.Name, true)[0];

            CostumeTV.SelectedNode.Expand();          
        }

        void copyCostume_Click(object sender, EventArgs e)
        {
            copySource = CostumeTV.SelectedNode;
        }

        void pasteCostume_Click(object sender, EventArgs e)
        {
            TreeNode tn = CostumeTV.SelectedNode;

            ccf.pasteCostume_Piece(copySource, tn,this.CostumeTV, CostumeTV_NodeMouseDoubleClick);

            CostumeTV.SelectedNode = this.CostumeTV.Nodes.Find(tn.Name,true)[0].NextNode;

            CostumeTV.SelectedNode.Expand();

            copySource = null;
        }

        void addCostume_Click(object sender, EventArgs e)
        {
            TreeNode tn = CostumeTV.SelectedNode;

            ccf.AddCostume_Piece(tn, this.CostumeTV, CostumeTV_NodeMouseDoubleClick);

            CostumeTV.SelectedNode = this.CostumeTV.Nodes.Find(tn.Name, true)[0].NextNode;

            CostumeTV.SelectedNode.Expand();
        }

        void propCostume_Click(object sender, EventArgs e)
        {
            string fileName = this.CostumeTV_lable.Text;

            costume.GeoPiece gp = (costume.GeoPiece) CostumeTV.SelectedNode.Tag;

            int startIndex = gp.getStartIndex();

            int lastIndex = gp.getLastIndex();

            string geoset = null;
                
            if(CostumeTV.SelectedNode.Parent.Tag.GetType() == typeof(costume.GeoSet))
            {
                geoset = CostumeTV.SelectedNode.Parent.Text;

                geoset = geoset.Substring(0, geoset.ToLower().IndexOf("_geoset"));
            }

            System.Collections.ArrayList fContent = ccf.fileContent;          

            CostumePropagationForm cpf = new CostumePropagationForm(fileName, fContent, startIndex, lastIndex);

            cpf.geosetName = geoset;

            cpf.ShowDialog();

            cpf.Dispose();
        }

        public void saveLoadedFile()
        {
            if (ccf != null && this.isCostumeFile)
            {
                SaveFileDialog saveFileDialog1 = new SaveFileDialog();

                saveFileDialog1.FileName = ccf.fileName;

                saveFileDialog1.Filter = "ctm files (*.ctm)|*.ctm";

                saveFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(ccf.filePath);

                DialogResult dr = saveFileDialog1.ShowDialog();

                if (saveFileDialog1.FileName != "" )
                {
                    string results = COH_CostumeUpdater.common.COH_IO.writeDistFile(ccf.fileContent, saveFileDialog1.FileName);

                    this.logTxBx.Text += results;
                }
                saveFileDialog1 = null;
            }
            else
                MessageBox.Show("Nothing saved! No Costume File was loaded", "Warning");
        }

        void CostumeTV_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            try
            {
                if (e.Node.Tag != null)
                {
                    if (isCostumeFile)
                    {
                        Type type =  e.Node.Tag.GetType();

                        if (type.Name.ToLower().Contains("texture") && e.Node.ForeColor!=Color.Red)
                        {
                            costume.Texture texture = (costume.Texture)e.Node.Tag;

                            if (texture.path == "")
                            {
                                logTxBx.SelectionColor = Color.Red;

                                logTxBx.SelectedText = "Warrning: if this is the first time you search for this object, it MAY TAKE OVER 2 SECONDS\r\n";

                                logTxBx.SelectionColor = logTxBx.ForeColor;
                            }
                         
                            string path = texture.path == ""  ? texture.findTexture():texture.path;
                            
                            if (path.ToLower().EndsWith(".texture"))
                            {
                                if (File.Exists(path))
                                {
                                    viewTexture(path, false);
                                }
                            }
                            else
                            {
                                e.Node.ForeColor = Color.Red;

                                MessageBox.Show("Could Not find " + texture.texName);
                            }
                        }

                        else if (type.Name.ToLower().Contains("fx"))
                        {
                            costume.Fx tFx = (costume.Fx)e.Node.Tag;

                            if (tFx.fileExists)
                            {
                                System.Diagnostics.Process.Start(tFx.path);
                            }

                            else
                            {
                                e.Node.ForeColor = Color.Red;
                            }
                        }

                    }

                    else if (File.Exists(e.Node.Tag.ToString()))
                    {
                        System.Diagnostics.Process.Start(e.Node.Tag.ToString());
                    }

                    else
                    {
                        MessageBox.Show("File Does not Exists!", "Error");
                    }
                }
            }

            catch (Exception ex)
            {
                logTxBx.SelectionColor = logTxBx.ForeColor;

                logTxBx.SelectedText = ex.Message + "\r\n";
            }
        }     
    
        private void viewTexture(string fileName, bool alphaOn)
        {
            costume.ddsViewer dd = new costume.ddsViewer(fileName, alphaOn);

            dd.ShowDialog();

            dd.Dispose();   
        }
    }
}
