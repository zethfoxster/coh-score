using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.costume
{
    class tNodeTag
    {
        public int index;
        public string path;
        public tNodeTag(int ind, string filePath)
        {
            this.index = ind;
            this.path = filePath;
        }
    }

    class CharacterCostumeFile
    {
        public string fileName;
        public string filePath;
        public string bodyPart;
        public string faceScale;
        public string charType;

        public ArrayList fileContent;
        public ArrayList fileElements;
       

        public CharacterCostumeFile(string path)
        {
            initialize();
            fileName = System.IO.Path.GetFileName(path);
            filePath = path;
            setCharType();
            setBodyPart();
            
            COH_CostumeUpdater.common.COH_IO.fillList(this.fileContent, this.filePath);
        }

        public void AddCostume_Piece(System.Windows.Forms.TreeNode tn,
                                      System.Windows.Forms.TreeView tv,
                                      System.Windows.Forms.TreeNodeMouseClickEventHandler tNmouseClickEH)
        {
            AddCostumePiece acp = new AddCostumePiece(this.filePath, ref this.fileContent, tn, 0, false, false);
            if (acp.showAddWindow())
            {
                buildTree(tv, tNmouseClickEH);
            }
            acp.Dispose();
        }

        public void editCostume_Piece(System.Windows.Forms.TreeNode tn, 
                                      System.Windows.Forms.TreeView tv, 
                                      System.Windows.Forms.TreeNodeMouseClickEventHandler tNmouseClickEH)
        {
            AddCostumePiece acp = new AddCostumePiece(this.filePath, ref this.fileContent, tn, 0, true, false);
            if (acp.showAddWindow())
            {
                buildTree(tv, tNmouseClickEH);
            }
            acp.Dispose();
        }

        public void pasteCostume_Piece(System.Windows.Forms.TreeNode copySource,
                                      System.Windows.Forms.TreeNode tn,
                                      System.Windows.Forms.TreeView tv,
                                      System.Windows.Forms.TreeNodeMouseClickEventHandler tNmouseClickEH)
        {
            GeoPiece srcGP = (GeoPiece)copySource.Tag;
            int len = srcGP.getLastIndex() - srcGP.getStartIndex();
            int start = ((GeoPiece)tn.Tag).getStartIndex();
            int lastInd = start + len;

            GeoPiece gp = (GeoPiece) srcGP.Clone(start, lastInd);
            System.Windows.Forms.TreeNode ctn = (System.Windows.Forms.TreeNode)copySource.Clone();
            ctn.Tag = gp;

            AddCostumePiece acp = new AddCostumePiece(this.filePath, ref this.fileContent, ctn, 0, false, true);
            if (acp.showAddWindow())
            {
                buildTree(tv, tNmouseClickEH);
            }
            acp.Dispose();
        }

        private int buildBoneSet(int index, System.Windows.Forms.TreeView tv)
        {
            BoneSet bSet = new BoneSet(fileContent, index, filePath, tv);
            return bSet.getLastIndex();
        }

        private int buildGeoSet(int index, System.Windows.Forms.TreeView tv)
        {
            GeoSet gSet = new GeoSet(fileContent, index, filePath, tv);
            return gSet.getLastIndex();
        }
        private int buildGeoPiece(int index, System.Windows.Forms.TreeView tv)
        {
            GeoPiece gPiece = new GeoPiece(fileContent, index, filePath, tv);
            
            return gPiece.getLastIndex();
        }
        private void cleanUpTv(System.Windows.Forms.TreeView tv, System.Windows.Forms.TreeNodeMouseClickEventHandler tNmouseClickEH)
        {
            tv.NodeMouseDoubleClick -= tNmouseClickEH;

            foreach (System.Windows.Forms.TreeNode tn in tv.Nodes)
            {
                cleanUpTn(tn);
            }
        }
        private void cleanUpTn(System.Windows.Forms.TreeNode tn)
        {
            if (tn.Nodes.Count > 0)
            {
                foreach (System.Windows.Forms.TreeNode ctn in tn.Nodes)
                {
                    cleanUpTn(ctn);
                }
            }
            tn.Tag = null;
            tn.Remove();
        }
        public string buildTree(System.Windows.Forms.TreeView tv, System.Windows.Forms.TreeNodeMouseClickEventHandler tNmouseClickEH)
        {
            if (isCostumeFile())
            {
                cleanUpTv(tv, tNmouseClickEH);

                tv.BeginUpdate();

                tv.Nodes.Clear();

                tv.NodeMouseDoubleClick += tNmouseClickEH;

                // Add a root TreeNode for each Customer object in the ArrayList.
                for (int i = 0; i < fileContent.Count; i++ )
                {
                    string str = (string)fileContent[i];
                    if (str.StartsWith("//"))
                        continue;
                    else if (common.COH_IO.stripWS_Tabs_newLine_Qts(str, false).ToLower().StartsWith("boneset"))
                    {
                        i = buildBoneSet(i, tv);
                    }
                    else if (common.COH_IO.stripWS_Tabs_newLine_Qts(str, false).ToLower().StartsWith("geoset"))
                    {
                        i = buildGeoSet(i, tv);
                    }

                    else if (str.Contains("Info"))
                    {
                        i = buildGeoPiece(i, tv);
                    }
                    else if (str.Contains("End") || str.Replace("\t","").StartsWith("//"))
                    {
                    }
                    else
                    {
                        if (str.Length > 2)
                        {
                            System.Windows.Forms.TreeNode tn = new System.Windows.Forms.TreeNode(str);
                            tNodeTag tnTag = new tNodeTag(i, str);
                            tn.Tag = tnTag;
                            tn.Name = "tN_" + i;
                            common.COH_IO.setImageIndex(ref tn, str);
                            tv.Nodes.Add(tn);
                        }
                    }

                }

                tv.EndUpdate();

                return "\"" + fileName +"\" treeview created.\r\n";
            }
            else
                fileContent.Clear();

            return "ERROR: \"" + fileName + " IS NOT A COSTUME FILE!!!\"\r\n";
        }

        private bool isCostumeFile()
        {
            bool isCFile = false;

            isCFile = //common.COH_IO.contains(fileContent,"GeoSet") &&
                      common.COH_IO.contains(fileContent, "Info") &&
                      common.COH_IO.contains(fileContent, "End");

            return isCFile;
        }

        private void initialize()
        {
            this.fileContent = new ArrayList();
            this.fileElements = new ArrayList();
        }

        private void setCharType()
        {
            if (filePath.ToLower().Contains("arachnos"))
            {
                if (filePath.ToLower().Contains("male") &&
                   filePath.ToLower().Contains("soldier"))
                {
                    charType = "Arachnos_Male_Soldier";
                }
                else if (filePath.ToLower().Contains("male") &&
                         filePath.ToLower().Contains("widow"))
                {
                    charType = "Arachnos_Male_Widow";
                }
                else if (filePath.ToLower().Contains("female") &&
                         filePath.ToLower().Contains("soldier"))
                {
                    charType = "Arachnos_Female_Soldier";
                }
                else if (filePath.ToLower().Contains("female") &&
                         filePath.ToLower().Contains("widow"))
                {
                    charType = "Arachnos_Female_Widow";
                }
                else if (filePath.ToLower().Contains("huge") &&
                         filePath.ToLower().Contains("soldier"))
                {
                    charType = "Arachnos_Huge_Soldier";
                }
                else
                {
                    charType = "Arachnos_Huge_Widow";
                }
            }
            else if (filePath.ToLower().Contains("huge"))
            {
                charType = "Huge";
            }
            else if (filePath.ToLower().Contains("female"))
            {
                charType = "Female";
            }
            else
            {
                charType = "Male";
            }     
        }

        private void setBodyPart()
        {
            if (filePath.ToLower().Contains(@"\capes\"))
                bodyPart = "Capes";

            else if (filePath.ToLower().Contains(@"\head\"))
                bodyPart = "Head";

            else if (filePath.ToLower().Contains(@"\lower\"))
                bodyPart = "Lower";

            else if (filePath.ToLower().Contains(@"\shields\"))
                bodyPart = "Shields";

            else if (filePath.ToLower().Contains(@"\tails\"))
                bodyPart = "Tails";

            else if (filePath.ToLower().Contains(@"\upper\"))
                bodyPart = "Upper";

            else if (filePath.ToLower().Contains(@"\weapons\"))
                bodyPart = "Weapons";
        }
    }
}
