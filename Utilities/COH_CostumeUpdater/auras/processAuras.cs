using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.auras
{
    class processAuras
    {
        private string pathRoot;
        private string filePath;

        public processAuras(string path)
        {
            filePath = path;
            setRootPath();
        }

        private void setRootPath()
        {
            pathRoot = common.COH_IO.getRootPath(filePath);
        }

        public string createTreeViewNodes(Regions regionsObject,
                                           System.Windows.Forms.TreeView tv,
                                           System.Windows.Forms.TreeNodeMouseClickEventHandler tNmouseClickEH)
        {
            try
            {
                //Cursor.Current = System.Windows.Forms.Cursors.WaitCursor;
                //this.CostumeTV_lable.Text = regionsObject.filePath;
                tv.BeginUpdate();

                // Clear the TreeView each time the method is called.
                tv.Nodes.Clear();

                tv.NodeMouseDoubleClick += tNmouseClickEH;

                // Add a root TreeNode for each Customer object in the ArrayList.
                foreach (Region r in regionsObject.regionSections)
                {
                    tv.Nodes.Add(new System.Windows.Forms.TreeNode(r.displayName));
                    foreach (auras.auraCostume ctm in r.includeCostumes)
                    {
                        System.Windows.Forms.TreeNode chTNode = new System.Windows.Forms.TreeNode(ctm.fileName);
                        chTNode.ToolTipText = ctm.includeStatement;
                        chTNode.Tag = ctm.fullPath;
                        if (!ctm.exists)
                            chTNode.ForeColor = System.Drawing.Color.Red;
                        tv.Nodes[tv.Nodes.Count - 1].Nodes.Add(chTNode);
                    }
                }
                //Cursor.Current = Cursors.Default;

                // Begin repainting the TreeView.
                tv.EndUpdate();

                return "built treeview\r\n";
            }
            catch (Exception e)
            {
                return (e.Message + "\r\n");
            }
        }

        public string createTreeViewNodes(System.Windows.Forms.TreeView tv, 
                                        System.Windows.Forms.TreeNodeMouseClickEventHandler tNmouseClickEH)
        {
            try
            {
                //Cursor.Current = System.Windows.Forms.Cursors.WaitCursor;
                string dNameHeader = "\tDisplayName";
                string includeHeader = "\tInclude";
                //this.CostumeTV_lable.Text = fileName;
                StreamReader sr = new StreamReader(filePath);
                String line, displayName = "";
                tv.BeginUpdate();

                // Clear the TreeView each time the method is called.
                tv.Nodes.Clear();
                tv.NodeMouseDoubleClick += tNmouseClickEH;
                while ((line = sr.ReadLine()) != null)
                {
                    if (line.StartsWith(dNameHeader))
                    {
                        int startIndex = line.IndexOf('\"');
                        int endIndex = line.IndexOf('\"', startIndex + 1);
                        int nameLen = endIndex - startIndex;
                        displayName = line.Substring(startIndex + 1, nameLen - 1);
                        tv.Nodes.Add(new System.Windows.Forms.TreeNode(displayName));
                    }
                    if (line.StartsWith(includeHeader))
                    {
                        int startIndex = includeHeader.Length;
                        int nameLen = line.Length - startIndex;
                        string includeStatement = line.Substring(1, line.Length - 1);
                        string includeName = line.Substring(startIndex + 1, nameLen - 1);
                        string ctmFileName = Path.GetFileName(includeName);
                        string fullPath = pathRoot + includeName;

                        bool exists = File.Exists(fullPath);
                        System.Windows.Forms.TreeNode chTNode = new System.Windows.Forms.TreeNode(ctmFileName);
                        chTNode.ToolTipText = includeName;
                        chTNode.Tag = fullPath;
                        if (!exists)
                            chTNode.ForeColor = System.Drawing.Color.Red;
                        tv.Nodes[tv.Nodes.Count - 1].Nodes.Add(chTNode);
                    }

                }
                sr.Close();
                //Cursor.Current = Cursors.Default;

                // Begin repainting the TreeView.
                tv.EndUpdate();
                return "built treeview \r\n";
            }
            catch (Exception e)
            {
                return ( e.Message + "\r\n");
            }
        }
        
        public string copyAura(System.Collections.ArrayList distList)
        {
            string results = "";

            if (filePath.ToLower().EndsWith("regions.ctm"))
            {
                string distPathRoot = pathRoot + @"menu\Costume\";

                foreach (object dist in distList)
                {
                    string distPath = distPathRoot;
                    string[] distSplit = ((string)dist).Split(" ".ToCharArray());
                    if (distSplit.Length == 3)
                    {//"Arachnos Female Soldier",
                        distPath += distSplit[0] + "\\" + distSplit[1] + "\\" + distSplit[2] + "\\Regions.ctm";
                    }
                    else
                    {
                        distPath += distSplit[0] + "\\All\\Regions.ctm";
                    }

                    if (File.Exists(distPath))
                    {
                        results += copyAuraTo(distPath, distSplit);
                        results += "Processed: " + distPath + "\r\n";
                    }
                    else
                    {
                        results += "Destination file: \"" + distPath + "\"could not be found\r\n";
                    }
                }
                return results;
            }
            else
            {
                System.Windows.Forms.MessageBox.Show("You need to load \"Regions.ctm\" file before Propagation!", "Warrning");

                return "No Regions.ctm file was loaded to process!";
            }
        }

        private void fixFXDistPath(string distPath, string aurasType)
        {
            System.Collections.ArrayList sourceArrayList = new System.Collections.ArrayList();
            System.Collections.ArrayList fixedDistArrayList = new System.Collections.ArrayList();

            COH_CostumeUpdater.common.COH_IO.fillList(sourceArrayList, distPath);
            foreach (object item in sourceArrayList)
            {
                string newStr = @"Auras\" + aurasType;
                string itemStr = (string) item;
                if (itemStr.ToLower().Contains(@"auras\male"))
                {
                    itemStr = itemStr.Replace(@"Auras\Male", newStr);
                }
                
                fixedDistArrayList.Add(itemStr);
            }

            COH_CostumeUpdater.common.COH_IO.writeDistFile(fixedDistArrayList, distPath);

        }
        private string copyAuraTo(string charTypeFileName, string[] distSplit)
        {
            string results = "";

            System.Collections.ArrayList sourceArrayList = new System.Collections.ArrayList();
            System.Collections.ArrayList distArrayList = new System.Collections.ArrayList();

            COH_CostumeUpdater.common.COH_IO.fillList(sourceArrayList, filePath);
            COH_CostumeUpdater.common.COH_IO.fillList(distArrayList, charTypeFileName);

            int srcAurasStartIndx = sourceArrayList.IndexOf("\tDisplayName \"Auras\"");
            int srcAurasEndIndx = sourceArrayList.IndexOf("End", srcAurasStartIndx);

            int distAurasStartIndx = distArrayList.IndexOf("\tDisplayName \"Auras\"");
            int distAurasEndIndx = distArrayList.IndexOf("End", distAurasStartIndx);

            distArrayList.RemoveRange(distAurasStartIndx + 1, distAurasEndIndx - distAurasStartIndx - 1);

            string aurasType = "";

            if (distSplit.Length == 3)
            {//"Arachnos Female Soldier",
                aurasType += distSplit[1];
            }
            else
            {
                aurasType += distSplit[0];
            }

            for (int i = srcAurasStartIndx + 1, k = 1; i < srcAurasEndIndx; i++, k++)
            {
                string aurasPath = (string)sourceArrayList[i];
                string aurasNewPath = aurasPath.Replace("Male", aurasType);

                if (aurasPath.Contains(@"Male"))
                {
                    //\tInclude Menu\Costume\Common\Auras\Female\AuraNone.ctm

                    string src = pathRoot + aurasPath.Substring("\tInclude ".Length);
                    string dist = pathRoot + aurasNewPath.Substring("\tInclude ".Length);

                    if (!File.Exists(dist) && File.Exists(src))
                    {
                        File.Copy(src, dist);
                        fixFXDistPath(dist, aurasType);
                        results += "Copied \"" + src + "\" to \"" + dist + "\"\r\n";
                    }
                }

                distArrayList.Insert(distAurasStartIndx + k, aurasNewPath);
            }

            FileInfo fi = new FileInfo(charTypeFileName);

            if (fi.IsReadOnly)
            {
                //COH_CostumeUpdater.assetsMangement.CohAssetMang.preFileCheckout(charTypeFileName);
                COH_CostumeUpdater.assetsMangement.CohAssetMang.checkout(charTypeFileName);//.checkoutNoEdit(charTypeFileName);
                //COH_CostumeUpdater.assetsMangement.CohAssetMang.postFileCheckout(charTypeFileName);
            }
            results += COH_CostumeUpdater.common.COH_IO.writeDistFile(distArrayList, charTypeFileName);

            return results;
        }
    }
}
