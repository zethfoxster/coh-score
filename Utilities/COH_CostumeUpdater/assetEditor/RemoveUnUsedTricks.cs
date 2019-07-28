using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.assetEditor
{
    public class RemoveUnUsedTricks
    {
        private string unusedTricksList;
        private ArrayList lockedFiles;
        private ArrayList data;
        private ArrayList tricks;
        private ArrayList zeroFiles;
        private Dictionary<string, string> trickFiles;


        public RemoveUnUsedTricks(string fileName)
        {
            unusedTricksList = fileName;
            
            lockedFiles = new ArrayList();
            
            trickFiles = new Dictionary<string, string>();
            
            data = new ArrayList();
            
            tricks = new ArrayList();
            
            zeroFiles = new ArrayList();
            
            common.COH_IO.fillList(data, unusedTricksList);

            buildFilesDic();
        }


        public void compareMissingTextures()
        {
            ArrayList missTex = new ArrayList();
            ArrayList trFiles = new ArrayList();

            common.COH_IO.fillList(missTex, @"c:\test\missing_tex2.txt");

            foreach (KeyValuePair<string, string> kvp in trickFiles)
            {
                string line = kvp.Value;

                string[] split = line.Trim().Split(',');
                foreach (string s in split)
                {
                    string trick = System.IO.Path.GetFileNameWithoutExtension(s);

                    foreach (string str in missTex)
                    {
                        if (str.ToLower().Equals(trick.ToLower()))
                        {
                            trFiles.Add(str + ", " + kvp.Key);
                        }
                    }
                }                
            }
            common.COH_IO.writeDistFile(trFiles, @"c:/test/trickOFmissingTexture.txt");
        }
        public void removeUnusedTricks()
        {
            foreach (KeyValuePair<string, string> kvp in trickFiles)
            {
                string file = @"c:\game\data\" + kvp.Key;

                checkoutTrickFile(file);

                System.IO.FileInfo fi = new System.IO.FileInfo(file);

                bool ro = fi.Exists && fi.IsReadOnly;

                if (!ro)
                {
                    string line = kvp.Value;

                    string[] split = line.Trim().Split(',');

                    ArrayList fData = new ArrayList();

                    common.COH_IO.fillList(fData, file);

                    ArrayList mats = MatParser.parse(file, ref fData);
                    ArrayList objTricks = objectTrick.ObjectTrickParser.parse(file, ref fData);
                    ArrayList tTricks = textureTrick.TextureTrickParser.parse(file, ref fData);

                    foreach (string s in split)
                    {

                        int cnt = 0;
                        cnt += removeMat(s, ref mats);
                        //cnt += removeOtrick(s, ref objTricks);
                        cnt += removeTTrick(s, ref tTricks);
                        if (cnt == 0 || cnt > 1)
                            tricks.Add(string.Format("{0} was found {1} times in {2}", s, cnt, file));

                    }

                    ArrayList trickData = getTrickData(mats, objTricks, tTricks);

                    common.COH_IO.writeDistFile(trickData, file);

                    fi = new System.IO.FileInfo(file);

                    if (fi.Length == 0)
                    {
                        zeroFiles.Add(file);
                        assetsMangement.CohAssetMang.undoCheckout(file);
                        assetsMangement.CohAssetMang.delete(file);
                        fi.Delete();
                    }


                    fData.Clear();
                }
            }
            common.COH_IO.writeDistFile(lockedFiles, @"c:\test\lockedTrickFile.txt");

            common.COH_IO.writeDistFile(tricks, @"c:\test\failedTricks.txt");

            common.COH_IO.writeDistFile(zeroFiles, @"c:\test\zeroSizeFiles.txt");
        }

        private ArrayList getTrickData(ArrayList mat, ArrayList obj, ArrayList textureTricks)
        {
            ArrayList trickData = new ArrayList();
            foreach (materialTrick.MatTrick mt in mat)
            {
                trickData.AddRange(mt.data);
                trickData.Add("");
            } 

            foreach (objectTrick.ObjectTrick ot in obj)
            {
                trickData.AddRange(ot.data); 
                trickData.Add("");
            } 

            foreach (textureTrick.TextureTrick tt in textureTricks)
            {
                trickData.AddRange(tt.data);
                trickData.Add("");
            }

            return trickData;
        }
        
        private int removeMat(string s, ref ArrayList mats)
        {
            int i = 0;
            while (i < mats.Count)
            {
                materialTrick.MatTrick mt = (materialTrick.MatTrick)mats[i];

                string nameCompare = mt.name.Trim().ToLower();

                if(!mt.name.ToLower().StartsWith("x_"))
                    nameCompare = System.IO.Path.GetFileNameWithoutExtension(mt.name.Trim().ToLower());

                if (nameCompare.Equals(s.Trim().ToLower()))
                {
                    mats.Remove(mt);
                    return 1;
                }
                i++;
            }
            return 0;
        }
        
        private int removeOtrick(string s, ref ArrayList oTrick)
        {
            int i = 0;
            while (i < oTrick.Count)
            {
                objectTrick.ObjectTrick ot = (objectTrick.ObjectTrick)oTrick[i];

                if (ot.name.Trim().ToLower().Equals(s.Trim().ToLower()))
                {
                    oTrick.Remove(ot);
                    return 1;
                }
                i++;
            }
            return 0;
        }
        
        private int removeTTrick(string s, ref ArrayList tTrick)
        {
            int i = 0;
            while (i < tTrick.Count)
            {
                textureTrick.TextureTrick tt = (textureTrick.TextureTrick)tTrick[i];

                if (tt.name.Trim().ToLower().Equals(s.Trim().ToLower()))
                {
                    tTrick.Remove(tt);
                    return 1;
                }
                i++;
            }
            return 0;
        }

        private void buildFilesDic()
        {           
            foreach (string line in data)
            {
                string []split = line.Trim().Split(',');
                string fileName = split[0];
                string trick = split[1];

                if (trickFiles.ContainsKey(fileName))
                    trickFiles[fileName] = trickFiles[fileName] + "," + trick;
                else
                    trickFiles[fileName] = trick;
            }
        }
        
        private void deleteTrick(string trickFile, string trickName)
        {
        }
        
        private bool checkoutTrickFile(string trickFile)
        {
            string results = assetsMangement.CohAssetMang.checkout(trickFile);
            string lockedFileStr = "can't edit exclusive file already opened";
            if (results.Contains(lockedFileStr))
            {
                string resultStr = trickFile + results.Substring(results.IndexOf(" - also opened by "));
                lockedFiles.Add(resultStr);
                return false;
            }
            return true;
        }

    }
}
