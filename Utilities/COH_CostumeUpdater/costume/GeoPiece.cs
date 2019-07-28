using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.costume
{
    /*
       \tInfo
			\tDevOnly 1
			\tGeoName "Destroyers Single Lvl 1"
			\tDisplayName "Destroyers Single Lvl 1"
			\tGeo  "V_FEM_BACK.GEO/GEO_Back_Destroyers_01"
            \tFx   "ENEMYFX\SkyRaiders\SkyRaiderJets.fx"
			\tTex1 "!Emblem_DestroyerFem_01"
			\tTex2 "!Emblem_DestroyerFem_01_Mask"
            \tKey  "Warp_Backpack"
		\tEnd
      */

    [Serializable]
    class GeoPiece:ICloneable
    {
        private string infoStr;
        private string endStr;
        private int startIndex;
        private int lastIndex;
        private int geoMemInd;
        private string pathRoot;

        public int fxCount;
        public int keyCount;
        public bool isMask;
        public string id;
        public string geoName;
        public string displayName;
        public ArrayList members;

        //"Id", "GeoName", "DisplayName",  "Geo", "Fx", "Tex1", "Tex2", "Tag", "Key"


        public GeoPiece(ArrayList fileContent, int index, string fileName, System.Windows.Forms.TreeView tv)
        {
            this.initialize(fileContent, index, fileName, tv, null);
        }

        public GeoPiece( ArrayList fileContent, int index, string fileName, System.Windows.Forms.TreeView tv, System.Windows.Forms.TreeNode pTn)
        {
            this.initialize(fileContent, index, fileName, tv, pTn);
        }

        private void initialize(ArrayList fileContent, int index, string fileName, System.Windows.Forms.TreeView tv, System.Windows.Forms.TreeNode pTn)
        {
            isMask = false;
            
            id = "";
            
            setPathRoot(fileName);
            
            fxCount = 0;
            
            keyCount = 0;

            members = new ArrayList();

            startIndex = index;

            for (int i = index; i < fileContent.Count; i++)
            {
                string str = (string)fileContent[i];

                if (str.ToLower().Contains("info"))
                {
                    infoStr = str;
                }
                else if (str.ToLower().Contains("geoname"))
                {
                    geoName = getFiledStr(str);
                    members.Add(str);
                }
                else if (str.ToLower().Contains("displayname"))
                {
                    displayName = getFiledStr(str);
                    members.Add(str);
                }
                else if (str.ToLower().Contains("geo"))
                {
                    geoMemInd = members.Count;
                    Geo geo = new Geo(str);
                    members.Add(geo);
                }
                else if (str.ToLower().Contains("fx"))
                {
                    fxCount++;
                    Fx fx = new Fx(str, pathRoot);
                    members.Add(fx);
                }
                else if (str.ToLower().Contains("tex1"))
                {
                    costume.Texture tx = new costume.Texture(pathRoot, str);
                    members.Add(tx);
                }
                else if (str.Contains("Tex2"))
                {
                    costume.Texture tx = new costume.Texture(pathRoot, str);
                    members.Add(tx);
                }
                else if (str.ToLower().Contains("tag"))
                {
                    members.Add(str);
                }
                else if (str.ToLower().Contains("key"))
                {
                    members.Add(str);
                }
                else if (str.ToLower().Contains("end"))
                {
                    endStr = str;
                    lastIndex = i;
                    if (members.Count > 0)
                    {
                        string lastMember = members[members.Count - 1].ToString();
                        if (isMask)
                            isMask = true;
                    }
                    break;
                }
                else
                {
                    if (members.Count == 0)
                        id = stripWS_Tabs_newLine_Qts(str, true);

                    members.Add(str);

                }
            }

            buildTree(tv, pTn);          
        }

        public object Clone()
        {
            System.IO.MemoryStream ms = new System.IO.MemoryStream();

            System.Runtime.Serialization.Formatters.Binary.BinaryFormatter bf = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();

            bf.Serialize(ms, this);

            ms.Position = 0;

            object obj = bf.Deserialize(ms);

            ms.Close();

            return obj;

        }

        public object Clone(int startInd, int lastInd)
        {
            GeoPiece clone = (GeoPiece) this.Clone();
            
            clone.startIndex = startInd;

            clone.lastIndex = lastInd;

            return clone;
        }

        private string getFiledStr(string str)
        {
            return common.COH_IO.getFiledStr(str);
        }

        private string stripWS_Tabs_newLine_Qts(string str, bool removeQuotes)
        {
            return common.COH_IO.stripWS_Tabs_newLine_Qts(str, false);
        }

        private void buildTree(System.Windows.Forms.TreeView tv)
        {
            buildTree(tv, null);
        }

        private void buildTree(System.Windows.Forms.TreeView tv, System.Windows.Forms.TreeNode pTn)
        {
            string geo = ((Geo)members[geoMemInd]).geo;
            string tName = "tN_" + startIndex;

            System.Windows.Forms.TreeNode tn = createTreeNode(displayName, this, tName);
            tn.ImageIndex = 1;
            tn.SelectedImageIndex = tn.ImageIndex;

            int k = 0;
            foreach (object obj in members)
            {
                System.Windows.Forms.TreeNode tnChild = null;
                string txt = "";

                Type objType = obj.GetType();
                string typeName = objType.Name.ToLower();
                if(typeName.ToLower().Contains("string"))
                {
                    string str = (string)obj;
                    if (str.ToLower().Contains("geoname"))
                    {
                        txt = stripWS_Tabs_newLine_Qts((string)obj, false);
                        tnChild = createTreeNode(txt, obj, "tNChild_" + k++);
                        tnChild.ImageIndex = 2;

                    }
                    else if (str.ToLower().Contains("displayname"))
                    {
                        txt = stripWS_Tabs_newLine_Qts((string)obj, false);
                        tnChild = createTreeNode(txt, obj, "tNChild_" + k++);
                        tnChild.ImageIndex = 2;
                    }
                    else if (str.ToLower().Contains("tag"))
                    {
                        txt = stripWS_Tabs_newLine_Qts((string)obj, false);
                        tnChild = createTreeNode(txt, obj, "tNChild_" + k++);
                        tnChild.ImageIndex = 7;
                    }
                    else if (str.ToLower().Contains("key"))
                    {
                        txt = stripWS_Tabs_newLine_Qts((string)obj, false);
                        tnChild = createTreeNode(txt, obj, "tNChild_" + k++);
                        tnChild.ImageIndex = 6;
                    }
                    else if (str.ToLower().Contains("end"))
                    {
                        if (isMask)
                        {
                            txt = stripWS_Tabs_newLine_Qts((string)obj, false);
                            tnChild = createTreeNode(txt, obj, "tNChild_" + k++);
                            tnChild.ImageIndex = 8;
                        }
                    }
                    else
                    {
                        txt = stripWS_Tabs_newLine_Qts((string)obj, false);
                        tnChild = createTreeNode(txt, obj, "tNChild_" + k++);
                        common.COH_IO.setImageIndex(ref tnChild, (string)obj);
                        //tnChild.ImageIndex = 100;
                    }
                }
  
                else if (typeName.Contains("geo"))
                {
                    txt = ((Geo)obj).gTreeDisplayName;
                    tnChild = createTreeNode(txt, obj, "tNChild_" + geoMemInd);
                    tnChild.ImageIndex = 3;
                }
                else if (typeName.Contains("fx"))
                {
                    Fx tFx = (Fx)obj;
                    txt = tFx.fxTreeNodeText;
                    tnChild = createTreeNode(txt, obj, "tNChild_" + k++);
                    tnChild.ImageIndex = 4;
                    if (!tFx.fileExists)
                        tnChild.ForeColor = System.Drawing.Color.Red;
                }
                else if (typeName.Contains("texture"))
                {
                    txt =  ((Texture)obj).tTreeDisplayName;                    
                    tnChild = createTreeNode(txt, obj, "tNChild_" + k++);
                    tnChild.ImageIndex = 5;
                }

                if (tnChild != null)
                {
                    tnChild.SelectedImageIndex = tnChild.ImageIndex;
                    if (tnChild.Text.ToLower().Contains("devonly 1"))
                    {
                        tn.ImageIndex = 10;
                        tn.SelectedImageIndex = tn.ImageIndex;
                    }
                    tn.Nodes.Add(tnChild);
                }
            }

            if (pTn != null)
                pTn.Nodes.Add(tn);
            else
                tv.Nodes.Add(tn);
        }

        private System.Windows.Forms.TreeNode createTreeNode(string tNodeText, object tag, string tNodeName)
        {
            System.Windows.Forms.TreeNode tn = new System.Windows.Forms.TreeNode(tNodeText);
            tn.Tag = tag;
            tn.Name = tNodeName;
                           
            return tn;
        }

        public int getStartIndex()
        {
            return this.startIndex;
        }

        public int getLastIndex()
        {
            return this.lastIndex;
        }

        private void setPathRoot(string filePath)
        {
            pathRoot = common.COH_IO.getRootPath(filePath);

        }

    }
}
