using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.costume
{

    class BoneSet
    {
        private int startIndex;
        private int lastIndex;
        private string pathRoot;
        private string fileName;

        public string name;
        public string displayName;
        public string key;

        public ArrayList geoSetsArrayList;
        public ArrayList members;

        public BoneSet()
        {
        }

        public BoneSet( ArrayList fileContent, int index, string filename, System.Windows.Forms.TreeView tv)
        {
            System.Windows.Forms.TreeNode pTn = new System.Windows.Forms.TreeNode();

            System.Windows.Forms.TreeNode tnChild = null;

            pathRoot = common.COH_IO.getRootPath(filename);

            this.fileName = filename;

            geoSetsArrayList = new ArrayList();

            members = new ArrayList();

            startIndex = index;

            int i = initialize(fileContent, filename, ref pTn);

            for ( ; i < fileContent.Count; i++)
            {
                string str = (string)fileContent[i];
                string strNoTab = common.COH_IO.stripWS_Tabs_newLine_Qts(str,false);

                if (strNoTab.ToLower().StartsWith("geoset"))
                {
                    i = buildGeoSets(fileContent, i, tv, pTn);
                    this.lastIndex = i;
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
                        tnChild = createTreeNode(strNoTab, str, "tNBSChild_" + i);
                        common.COH_IO.setImageIndex(ref tnChild, str);
                        pTn.Nodes.Add(tnChild);
                    }
                }
            }

            if(pTn != null)
                tv.Nodes.Add(pTn);
        }

        public int initialize(ArrayList fileContent,string filename,ref System.Windows.Forms.TreeNode pTn)
        {  
            /*
            BoneSet
	            \tName "Back Packs" // dont' change
	            \tDisplayName "Back Packs"
	            \tKey "Back_BackPacks"

	            \t// Wing effect
	            \t//------------------------------------------------------------
             */

            int i = startIndex;
            
            System.Windows.Forms.TreeNode tnChild = null;

            for (; i < fileContent.Count; i++)
            {
                string str = (string)fileContent[i];
                
                string strNoTab = common.COH_IO.stripWS_Tabs_newLine_Qts(str, false);

                if (strNoTab.ToLower().StartsWith("boneset"))
                {
                    members.Add(str);
                }

		        else if (strNoTab.ToLower().StartsWith("displayname"))
                {
                    string tName = "tN_BoneSet" + startIndex;
                    members.Add(str);
                    displayName = getFiledStr(str);
                    pTn.Text = displayName + "_BoneSet";
                    pTn.Tag = this;
                    pTn.Name = tName;
                    pTn.ImageIndex = 11;
                    pTn.SelectedImageIndex = pTn.ImageIndex;

                    
                    tnChild = createTreeNode(strNoTab, str, "tNBSChild_" + i);
                    common.COH_IO.setImageIndex(ref tnChild, str);
                    pTn.Nodes.Add(tnChild); 

                }
		        else if (strNoTab.ToLower().StartsWith("name") )
                {
                    members.Add(str);
                    this.name = getFiledStr(str);
                    tnChild = createTreeNode(strNoTab, str, "tNGSChild_" + i);
                    common.COH_IO.setImageIndex(ref tnChild, str);
                    pTn.Nodes.Add(tnChild);
                }
                else if (strNoTab.ToLower().StartsWith("key"))
                {
                    members.Add(str);
                    this.key = getFiledStr(str);
                    tnChild = createTreeNode(strNoTab, str, "tNGSChild_" + i);
                    common.COH_IO.setImageIndex(ref tnChild, str);
                    pTn.Nodes.Add(tnChild);
                }
                else if (strNoTab.ToLower().StartsWith("geoset"))
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

        private int buildGeoSets(ArrayList fileContent, int index, System.Windows.Forms.TreeView tv, System.Windows.Forms.TreeNode pTn)
        {
            GeoSet gSet = new GeoSet(fileContent, index, fileName, tv, pTn);

            geoSetsArrayList.Add(gSet);

            return gSet.getLastIndex();
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
