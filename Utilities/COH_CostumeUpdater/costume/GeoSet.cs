using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.costume
{
    /*
    \tGeoSet
		\tDisplayName "Back Packs"
		\tBodyPart "Back"
     */ 
    class GeoSet
    {
        private int startIndex;
        private int lastIndex;
        private string pathRoot;
        private string fileName;

        public string header = "\tGeoSet";
        public string displayName;
        public string bodyPart;
        public string defaultView;
        public string zoomView;

        public ArrayList geoPiecesArrayList;
        public ArrayList members;

        public GeoSet()
        {
        }

        public GeoSet(ArrayList fileContent, int index, string filename, System.Windows.Forms.TreeView tv)
        {
            initialize( fileContent, index, filename, tv, null);
        }

        public GeoSet( ArrayList fileContent, int index, string filename, System.Windows.Forms.TreeView tv, System.Windows.Forms.TreeNode tn)
        {
            initialize( fileContent, index, filename, tv, tn);
        }

        public void initialize(ArrayList fileContent, int index, string filename, System.Windows.Forms.TreeView tv, System.Windows.Forms.TreeNode bonSetTn)
        {
            System.Windows.Forms.TreeNode pTn = new System.Windows.Forms.TreeNode();

            System.Windows.Forms.TreeNode tnChild = null;

            pathRoot = common.COH_IO.getRootPath(filename);

            this.fileName = filename;

            geoPiecesArrayList = new ArrayList();

            members = new ArrayList();

            startIndex = index;

            int i = prep(fileContent, filename, ref pTn);

            for ( ; i < fileContent.Count; i++)
            {
                string str = (string)fileContent[i];
                string strNoTab = common.COH_IO.stripWS_Tabs_newLine_Qts(str,false);

                if (strNoTab.ToLower().StartsWith("info"))
                {
                    i = buildGeoPiece(fileContent, i, tv, pTn);
                }

                else if (strNoTab.ToLower().StartsWith("end"))
                {
                    this.lastIndex = i;
                    break;
                }
                else
                {
                    members.Add(str);
                    if (strNoTab.Length > 2 && !strNoTab.StartsWith("//"))
                    {
                        tnChild = createTreeNode(strNoTab, str, "tNGSChild_" + i);
                        common.COH_IO.setImageIndex(ref tnChild, str);
                        pTn.Nodes.Add(tnChild);
                    }
                }
            }

            if (bonSetTn != null)
                bonSetTn.Nodes.Add(pTn);
            else
                tv.Nodes.Add(pTn);
        }

        public int prep(ArrayList fileContent,string filename,ref System.Windows.Forms.TreeNode pTn)
        {
            int i = startIndex;
            
            System.Windows.Forms.TreeNode tnChild = null;

            for (; i < fileContent.Count; i++)
            {
                string str = (string)fileContent[i];
                
                string strNoTab = common.COH_IO.stripWS_Tabs_newLine_Qts(str, false);

                if(strNoTab.ToLower().StartsWith("geoset"))
                {
                    members.Add(str);
                }
		        else if (strNoTab.ToLower().StartsWith("displayname"))
                {
                    string tName = "tN_GeoSet" + startIndex;
                    members.Add(str);
                    displayName = getFiledStr(str);
                    pTn.Text = displayName + "_GeoSet";
                    pTn.Tag = this;
                    pTn.Name = tName;
                    pTn.ImageIndex = 0;
                    pTn.SelectedImageIndex = pTn.ImageIndex;

                    
                    tnChild = createTreeNode(strNoTab, str, "tNGSChild_" + i);
                    common.COH_IO.setImageIndex(ref tnChild, str);
                    //tnChild.ImageIndex = 2;
                    pTn.Nodes.Add(tnChild); 

                }
		        else if (strNoTab.ToLower().StartsWith("bodypart") )
                {
                    members.Add(str);
                    this.bodyPart = getFiledStr(str);
                    tnChild = createTreeNode(strNoTab, str, "tNGSChild_" + i);
                    common.COH_IO.setImageIndex(ref tnChild, str);
                    //tnChild.ImageIndex = 2;
                    pTn.Nodes.Add(tnChild);
                }
                else if (strNoTab.ToLower().StartsWith("info"))
                {
                    return i;
                }
                else if (strNoTab.ToLower().StartsWith("end"))
                {
                    this.lastIndex = i;
                    return i;
                }
                else
                {
                    members.Add(str);
                    if (strNoTab.Length > 2 && !strNoTab.StartsWith("//"))
                    {
                        tnChild = createTreeNode(strNoTab, str, "tNGSChild_" + i);
                        common.COH_IO.setImageIndex(ref tnChild, str);
                        pTn.Nodes.Add(tnChild);
                    }
                }
            }

            return i;
        }

        public int getLastIndex()
        {
            return this.lastIndex;
        }

        private int buildGeoPiece(ArrayList fileContent, int index, System.Windows.Forms.TreeView tv, System.Windows.Forms.TreeNode pTn)
        {
            GeoPiece gPiece = new GeoPiece(fileContent, index, fileName, tv, pTn);

            geoPiecesArrayList.Add(gPiece);

            return gPiece.getLastIndex();
        }

        private System.Windows.Forms.TreeNode createTreeNode(string tNodeText, object tag, string tNodeName)
        {
            System.Windows.Forms.TreeNode tn = new System.Windows.Forms.TreeNode(tNodeText);
            tn.Tag = tag;
            tn.Name = tNodeName;
            return tn;
        }

        private string getFiledStr(string str)
        {
            return common.COH_IO.getFiledStr(str);
        }
    }
}
