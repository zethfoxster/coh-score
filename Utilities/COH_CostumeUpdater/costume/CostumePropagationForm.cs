using System;
using System.Collections;
using System.IO;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.costume
{
    public partial class CostumePropagationForm : Form
    {
        public CostumePropagationForm(string filePath, ArrayList fContent, int start_index, int last_index)
        {//Female, Huge, Male
            //Capes, Head, Lower, Shields, Tails, Upper, Weapons
            
            InitializeComponent();
            this.geosetName = null;
            this.gSetStartIndex = -1;
            this.gSetEndIndex = -1;

            this.cPropImageList.Images.AddRange(new System.Drawing.Image[]
                                                {
                                                 COH_CostumeUpdater.Properties.Resources.CLSDFOLD,
                                                 COH_CostumeUpdater.Properties.Resources.DOCR
                                                });
            this.fileName = filePath;
            this.startIndex = start_index;
            this.lastIndex = last_index;
            this.blockLength = 1 + this.lastIndex - this.startIndex;
            this.fileContent = fContent;
            this.Text += ": " + fileName;

            this.checkedItems = new ArrayList();
            this.insertionBlock = new ArrayList();
            buildTree();
            
            
        }
        private string getCharType(string path)
        {
            string charType = "";
            string cKey = @"\costume\";
            int typeStartPos = path.ToLower().IndexOf(cKey) + cKey.Length;

            if (path.ToLower().Contains(@"\all\"))
            {
                string allKey = @"\all";
                int typeEndPos = path.ToLower().IndexOf(allKey);
                int len = typeEndPos - typeStartPos;
                charType = path.Substring(typeStartPos, len);
            }
            return charType;
        }
        private void buildTree()
        {
            this.sourceType = getCharType(fileName);
            this.processingFeedBack.Text = sourceType;
            //string[] types = {"Arachnos", "Common", "Enemy", "Female", "FemaleNPC", "Huge", "Male", "MaleNPC"};
            string[] types = {"Female", "Huge", "Male"};
            if (sourceType.Length > 0)
            {
                string rootPath = common.COH_IO.getRootPath(fileName);

                buildTypeTree(fileName, sourceType, false);
                foreach (string t in types)
                {
                    if (!sourceType.ToLower().Equals(t.ToLower()))
                    {
                        string path = fileName.Replace(sourceType, t);
                        //string path = rootPath + @"menu\costume\" + t;
                        buildTypeTree(path, t, false);
                    }
                }
            }
        }
        private void buildTypeTree(string path, string type, bool isPath)
        {
            TreeNode tn = new TreeNode();
            tn.Tag = Path.GetDirectoryName(path);

            if(isPath)
                tn.Tag = path;

            tn.Text = type.Replace("\\", "_");
            tn.Name = tn.Text.Trim();
            tn.ImageIndex = 0;
            tn.SelectedImageIndex = tn.ImageIndex;
            AddDirectories(tn);
            propTV.Nodes.Add(tn);
        }

        void propTV_NodeMouseClick(object sender, System.Windows.Forms.TreeNodeMouseClickEventArgs e)
        {
           if (((TreeNode)e.Node).ImageIndex == 0)
            {
                setCheck(e.Node);
            }
        }

        private void setCheck(TreeNode tn)
        {
            foreach (TreeNode chTn in tn.Nodes)
            {
                chTn.Checked = tn.Checked;

                if (chTn.Nodes.Count > 0)
                    setCheck(chTn);
            }
        }

        private void getChecked(TreeNode tn)
        {
            foreach (TreeNode chTn in tn.Nodes)
            {
                if (chTn.Checked && chTn.ImageIndex != 0)
                    checkedItems.Add((string)chTn.Tag);

                if (chTn.Nodes.Count > 0)
                    getChecked(chTn);
            }
        }

        private void populate(DirectoryInfo dirInfo, TreeNode tn)
        {

            FileInfo[] dirFiles = dirInfo.GetFiles();
            
            string fName = Path.GetFileName(fileName);

            foreach (FileInfo fi in dirFiles)
            {
                if (!fi.FullName.Equals(fileName))
                {
                    TreeNode tnFile = new TreeNode(fi.Name, 1, 2);
                    tnFile.Name = fi.Name;
                    tnFile.Text = fi.Name;
                    tnFile.Tag = fi.FullName;
                    tnFile.ImageIndex = 1;
                    tnFile.SelectedImageIndex = tnFile.ImageIndex;
                    tnFile.ToolTipText = fi.FullName;
                    if (fi.Name.ToLower().Equals(fName.ToLower()))
                    {
                        tnFile.BackColor = Color.Yellow;
                    }
                    tn.Nodes.Add(tnFile);
                }
            }

            DirectoryInfo[] diArr = dirInfo.GetDirectories();

            foreach (DirectoryInfo dri in diArr)
            {
                TreeNode tnDir = new TreeNode(dri.Name, 1, 2);
                tnDir.Name = dri.Name;
                tnDir.Text = dri.Name;
                tnDir.Tag = dri.FullName;
                tnDir.ImageIndex = 0;
                tn.SelectedImageIndex = tn.ImageIndex;
                tn.ToolTipText = dri.FullName;
                tn.Nodes.Add(tnDir);
                populate(dri, tnDir);
            }

        }


        private void AddDirectories(TreeNode tn)
        {
            tn.Nodes.Clear();

            string strPath = (string) tn.Tag;

            DirectoryInfo diDirectory = new DirectoryInfo(strPath);

            try
            {
                populate(diDirectory, tn);
            }

            catch(Exception exp)
            {                
            }

        }

        private void propagateBtn_Click(object sender, EventArgs e)
        {
            checkedItems.Clear();
            foreach (TreeNode tn in this.propTV.Nodes)
            {
                getChecked(tn);
            }
            if(this.checkedItems.Count > 0)
                propagate();
            else
                MessageBox.Show("No Files were CHECKED");
        }

        private void fillInsertionBlock()
        {
            this.insertionBlock.Clear();

            int maxCount = Math.Min(startIndex + blockLength, fileContent.Count);
            for (int i = startIndex; i < maxCount; i++)
            {
                this.insertionBlock.Add(fileContent[i]);
            }
        }

        private string getSourceGeo()
        {
            string srcGeo = "";

            foreach (object obj in insertionBlock)
            {
                string str = (string) obj;

                str = common.COH_IO.stripWS_Tabs_newLine_Qts(str,false);

                if(str.ToLower().StartsWith("geoname"))
                    continue;
                else if (str.ToLower().StartsWith("geoset"))
                    continue;
                else if(str.ToLower().StartsWith("geo"))
                {
                    srcGeo = str.Substring(str.IndexOf("\""));
                    srcGeo.Replace("\"","");
                    break;
                }
            }

            return srcGeo;
        }

        private void collectGeos()
        {
            if (lvItems != null)
                lvItems = null;

            lvItems = new ListViewItem[checkedItems.Count];

            fillInsertionBlock();

            string srcGeo = getSourceGeo();

            int i = 0;

            foreach (object obj in checkedItems)
            {
                ListViewItem lvi = new ListViewItem();

                string path = (string)obj;

                lvi.Text = path;
                lvi.Name = Path.GetFileNameWithoutExtension(path);
                
                string locDestType = getCharType(path);

                string destChType = locDestType.ToLower().Replace("female", "fem") + "_";    
                string srcChType = sourceType.ToLower().Replace("female", "fem") + "_";

                string destGeo = srcGeo.Replace(srcChType,destChType).Replace(srcChType.ToUpper(),destChType.ToUpper());
                
                ArrayList destFileContent = new ArrayList();
                ArrayList geos = new ArrayList();

                destFileContent.Clear();
                geos.Clear();

                COH_CostumeUpdater.common.COH_IO.fillList(destFileContent, path);

                geos.Add(destGeo);

                foreach(object objLine in destFileContent)
                {
                    string str = (string) objLine;
                    str = common.COH_IO.stripWS_Tabs_newLine_Qts(str,false);
                    if(str.ToLower().StartsWith("geoname"))
                        continue;
                    else if (str.ToLower().StartsWith("geoset"))
                        continue;
                    else if(str.ToLower().StartsWith("geo"))
                    {
                        string geo = str.Substring(str.IndexOf("\""));
                        geo.Replace("\"","");

                        if(!geos.Contains(geo))
                            geos.Add(geo);
                    }
                }
                geos.Sort();
                lvi.SubItems.Add(destChType.Replace("_",""));
                lvi.SubItems.Add(destGeo);
                lvi.Tag = geos;

                lvItems[i++] = lvi;
            }
        }

        private void propagate()
        {
            collectGeos();

            if (this.showOptionWin.Checked)
                doPropagation();
            else
            {
                GeoSelectionForm nf = new costume.GeoSelectionForm();
                bool okClicked = nf.showGeoSelectionBox(ref lvItems);
                nf.Dispose();
                if (okClicked)
                    doPropagation();
            }

        }
        private void updateInsertionBlockGeo(string newGeo)
        {
            for(int i = 0; i < insertionBlock.Count; i++)
            {
                string str = (string)insertionBlock[i];

                str = common.COH_IO.stripWS_Tabs_newLine_Qts(str, false);

                if (str.ToLower().StartsWith("geoname"))
                    continue;
                else if (str.ToLower().StartsWith("geoset"))
                    continue;
                else if (str.ToLower().StartsWith("geo"))
                {
                    string geoTag = ((string)insertionBlock[i]).Substring(0, ((string)insertionBlock[i]).IndexOf("\""));
                    insertionBlock[i] = geoTag + newGeo;
                    break;
                }
            }
        }
        private void doPropagation()
        {
            bool hasBlock = false;
            propPBar.Visible = true;
            propPBar.Minimum = 1;
            propPBar.Maximum = checkedItems.Count;
            propPBar.Value = 1;
            propPBar.Step = 1;

            //foreach (object obj in checkedItems)
            foreach (ListViewItem lvi in this.lvItems)
            {
                int simiCount = 0;
                int insertionPos = -1;
                string path = (string) lvi.Text;

                fillInsertionBlock();

                updateInsertionBlockGeo(lvi.SubItems[2].Text);

                destType = getCharType(path);

                ArrayList destFileContent = new ArrayList();

                destFileContent.Clear();

                COH_CostumeUpdater.common.COH_IO.fillList(destFileContent, path);

                //check out file
                checkoutFile(path);

                //does the desination contains the desired block 
                hasBlock = destContainsDesiredBlock(destFileContent, startIndex, lastIndex,ref simiCount, ref insertionPos);

                //identify insertion position
                if (!hasBlock)
                {
                    insertionPos = getInsertionPos(destFileContent, insertionPos, simiCount);

                    if (insertionPos < 0)
                    {
                        if (gSetEndIndex > 0)
                        {
                            insertionPos = gSetEndIndex - 1;
                        }
                        else
                        {
                            insertionPos = destFileContent.Count - 1;
                        }
                    }
                }

                //check if show option window is checked
                if (this.showOptionWin.Checked)
                    showOptionWindow(); 

                //remove old section if exists and replace by new section
                bool blockRemoved = false;
                if (hasBlock)
                {
                    destFileContent.RemoveRange(insertionPos, blockLength);
                    blockRemoved = true;
                }

                string dStr = (string) destFileContent[insertionPos];
                
                bool addNewLineAfter = false;

                if (dStr.Length > 0)
                {
                    addNewLineAfter = true;
                    dStr = common.COH_IO.stripWS_Tabs_newLine_Qts(dStr, false);
                }
                else if (!blockRemoved)
                    destFileContent.Insert(insertionPos++, "");
                

                foreach (object o in insertionBlock)
                {
                    destFileContent.Insert(insertionPos++, (string)o);
                }

                if(addNewLineAfter)
                    destFileContent.Insert(insertionPos++, "");

                //save file and update log with processed files
                saveFile(destFileContent, path);

                //clean up
                updateLog(path);
                destType = null;
                propPBar.PerformStep();
                this.Update();
            }
            propPBar.Visible = false;
        }
       
        private void updateLog(string processedFileName)
        {
            string str = "";
            this.processingFeedBack.Text += "\r\n  Inserted the following into " + processedFileName + "\r\n\r\n";
            foreach (object obj in this.insertionBlock)
            {
                str = (string)obj;
                if (!str.Contains("\r\n"))
                    str += "\r\n";
                this.processingFeedBack.Text += str;
            }
            this.processingFeedBack.Text += "\r\n";
            this.processingFeedBack.Update();
            this.Update();
        }
       
        private void showOptionWindow()
        {
            this.insertionBlock.Clear();
            this.fillInsertionBlock();
            AddCostumePiece acp = new AddCostumePiece();
            showOptionWin.Checked = !acp.showOptionWindow(fileName, ref insertionBlock);
            acp.Dispose();
        }
        
        private void checkoutFile(string filePath)
        {

            FileInfo fi = new FileInfo(filePath);

            if (fi.IsReadOnly)
            {
                COH_CostumeUpdater.assetsMangement.CohAssetMang.preFileCheckout(filePath);
                COH_CostumeUpdater.assetsMangement.CohAssetMang.checkoutNoEdit(filePath);
                COH_CostumeUpdater.assetsMangement.CohAssetMang.postFileCheckout(filePath);
            }
        }

        private void saveFile(ArrayList file_content, string path)
        {
            string results = COH_CostumeUpdater.common.COH_IO.writeDistFile(file_content, path);
            this.processingFeedBack.Text += results + "\r\n";
        }

        private int compareBlockSections(ArrayList destination, int dCurrPos, int srcStartInd, int srcEndInd)
        { 
            int simCount = 0;

            int highestMatch = -1;

            string compDest = "";

            int len = 1 + srcEndInd - srcStartInd;

            string destChType = "_" + destType.ToLower().Replace("female", "fem") + "_";    

            string srcChType = "_" + sourceType.ToLower().Replace("female", "fem") + "_";

            for(int i = 0; i < len && (i+dCurrPos) < destination.Count; i++)
            {

                string des = (string)destination[dCurrPos + i];
                compDest = des.ToLower().Replace(destChType, srcChType);
                string notabStr = common.COH_IO.stripWS_Tabs_newLine_Qts(compDest, false);

                compDest = common.COH_IO.removeExtraSpaceBetweenWords(compDest);

                if (notabStr.Equals("info"))
                    simCount = 0;

                else if (notabStr.ToLower().Equals("end"))
                    break;

                else
                {
                    //add 1 to start so not to compare info to info 
                    for (int c = srcStartInd + 1; c < srcEndInd; c++)
                    {
                        string src = (string)fileContent[c];
                        src = common.COH_IO.removeExtraSpaceBetweenWords(src);
                        if (compDest.Equals(src.ToLower()))
                        {
                            simCount++;
                            break;
                        }
                    }
                }
                if (simCount > highestMatch)
                    highestMatch = simCount;
            }

            return highestMatch;
        }
        private void setGeoSetStartEndIndex(ArrayList destination, int i)
        {
            if (geosetName != null)
            {
                for (int j = i; j < destination.Count; j++)
                {
                    string dStr = (string)destination[i];

                    dStr = common.COH_IO.stripWS_Tabs_newLine_Qts(dStr, false);

                    if (dStr.ToLower().StartsWith("geoset")
                        && gSetStartIndex < 0)
                    {
                        setGeoSetStartIndex(destination, j);
                    }
                    if (gSetStartIndex > 0)
                    {
                        setGeoSetEndIndex(destination, j);
                        break;
                    }
                }
            }
        }
        private void setGeoSetStartIndex(ArrayList destination, int i)
        {
            for (int j = i + 1; j < i + 10; j++)
            {
                string lstr = (string)destination[j];
                lstr = common.COH_IO.stripWS_Tabs_newLine_Qts(lstr, false);
                if (lstr.ToLower().StartsWith("displayname"))
                {
                    string gSetName = lstr.ToLower().Replace("displayname", "").Replace("\"","").Trim();
                    if (geosetName.ToLower().Equals(gSetName))
                    { 
                        gSetStartIndex = i;
                        break;
                    }
                }
            }
        }
        private void setGeoSetEndIndex(ArrayList destination, int i)
        {
            int infoCount = 0;
            int infoEndCount = 0;
            for (int j = i; j < destination.Count; j++)
            {
                string lstr = (string)destination[j];
                lstr = common.COH_IO.stripWS_Tabs_newLine_Qts(lstr, false);
                if (lstr.ToLower().StartsWith("info"))
                {
                    infoCount++;
                }
                else if (lstr.ToLower().StartsWith("end"))
                {
                    infoEndCount++;
                }
                if (infoEndCount > infoCount)
                {
                    gSetEndIndex = j;
                    break;
                }
            }
        }
        private bool destContainsDesiredBlock(ArrayList destination, int srcStartPos, int srcEndPos, ref int simCount, ref int endPos)
        {
            // len = end - start -1 to exclude info and end from the count
            int len = srcEndPos - srcStartPos -1;

            for (int i = 0; i < destination.Count; i++)
            {
                string dStr = (string) destination[i];

                dStr = common.COH_IO.stripWS_Tabs_newLine_Qts(dStr,false);
                //GeoSet
		        //DisplayName "Mantle"
                if (dStr.ToLower().StartsWith("geoset") 
                    && gSetStartIndex < 0)
                {
                    setGeoSetStartEndIndex(destination, i);
                }

                if (dStr.ToLower().StartsWith("info"))
                {
                    endPos = i;

                    simCount = compareBlockSections(destination, i, srcStartPos, srcEndPos);
                    
                    if(simCount > (len - 2))
                        break;
                }
            }

            if (simCount >= len)
                return true;

            return false;
        }

        private int getInsertionPos(ArrayList destination, int possiblePos, int simCount)
        {
            bool hasPrevBlock = false;

            int prevBlockStart = -1;
            
            int prevBlockEnd = -1;

            int len = 2;

            int sCnt = -1;

            int endPos = -1;
            
            int insertionPos = -1;
            
            if (simCount > (this.blockLength - 2) && possiblePos > 0)
            {
                insertionPos = 1 + getBlockPos(destination, possiblePos, "end");
            }
            else
            {
                 getPreviousBlock(ref prevBlockStart, ref prevBlockEnd);
                

                if (prevBlockStart < 0)
                {
                    insertionPos = destination.Count;
                }
                else
                {
                    hasPrevBlock = destContainsDesiredBlock(destination, prevBlockStart, prevBlockEnd, ref sCnt, ref endPos);
                    
                    //end - start -1 to exclued info and end from comparison
                    if(prevBlockEnd > 0)
                        len = prevBlockEnd - prevBlockStart -1;

                    if (hasPrevBlock)
                    {
                        insertionPos = 1 + getBlockPos(destination, endPos, "end");
                    }

                    else if (sCnt > (len - 2) && endPos > 0)
                    {
                        insertionPos = 1 + getBlockPos(destination, endPos, "end");
                    }
                }
            }
            if (gSetEndIndex > 0 && insertionPos > gSetEndIndex)
            {
                insertionPos = gSetEndIndex - 1;
            }
            return insertionPos;
        }

        private int getBlockPos(ArrayList destination, int searchStartLocation,string fieldKey)
        {
            int maxSearchCount = Math.Min(searchStartLocation + 32,destination.Count);
            for (int i = searchStartLocation; i < maxSearchCount ; i++)
            {
                string dStr = (string)destination[i];
                dStr = common.COH_IO.stripWS_Tabs_newLine_Qts(dStr, false);
                if (dStr.ToLower().StartsWith(fieldKey.ToLower()))
                    return i;
            }
            return -1;
        }

        private void getPreviousBlock(ref int prevBlockStart, ref int prevBlockEnd)
        {//BoneSet, GeoSet, Include, End, Info <-- 
            
            for (int i = this.startIndex - 1 , c = 0; i > 0 && c < 16; i--, c++)
            {
                string str = (string)this.fileContent[i];

                str = common.COH_IO.stripWS_Tabs_newLine_Qts(str, true);

                if (str.ToLower().Contains("info"))
                {
                    prevBlockStart = i;

                    bool isEnd = false;

                    while (!isEnd && i < fileContent.Count)
                    {
                        i++;

                        string testStr = (string)fileContent[i];

                        testStr = common.COH_IO.stripWS_Tabs_newLine_Qts(testStr, false);

                        isEnd = testStr.ToLower().Contains("end");

                        if (isEnd)
                            prevBlockEnd = i;
                    }

                    break;
                }

                if (str.ToLower().Contains("include"))
                {
                    prevBlockStart = i;
                    break;
                }

                if (str.ToLower().Contains("geoset"))
                {
                    prevBlockStart = i;
                    break;
                }
                if (str.ToLower().Contains("boneset"))
                {
                    prevBlockStart = i;
                    break;
                }
            }                     
        }

    }
}
