using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.assetEditor.materialTrick
{
    class EndlessTricks
    {
        public static void findEndlessTricks()
        {
            string endlessTricksFile = @"c:\test\endlessTricks.txt";

            ArrayList endlessTrks = new ArrayList();
            
            ArrayList data = new ArrayList();

            string[] files = Directory.GetFiles(@"C:\game\data\tricks", "*.txt", System.IO.SearchOption.AllDirectories);

            foreach (string f in files)
            {
                data.Clear();

                common.COH_IO.fillList(data, f);

                ArrayList et = EndlessTricks.parse(f, ref data);

                if (et.Count > 0)
                    endlessTrks.AddRange(et);
            }

            if (endlessTrks.Count > 0)
                common.COH_IO.writeDistFile(endlessTrks, endlessTricksFile);
        }

        public static bool isMat(ArrayList fileContent, int startInd)
        {
            int i = startInd;

            string line = common.COH_IO.removeExtraSpaceBetweenWords(((string)fileContent[i]).Replace("\t", " ")).Trim();

            if (line.ToLower().StartsWith("Texture X_".ToLower()))
                return true;

            while (!line.ToLower().Equals("end") && i < fileContent.Count)
            {
                if (line.ToLower().StartsWith("base1"))
                    return true;

                line = common.COH_IO.removeExtraSpaceBetweenWords(((string)fileContent[i]).Replace("\t", " ")).Trim();

                i++;
            }

            return false;
        }

        public static ArrayList parse(string fileName, ref ArrayList fileContent)
        {
            ArrayList endlessTricks = new ArrayList();

            bool isMatT = false;

            bool isTextureTrick = false;

            bool isObjTrick = false;

            string trickName = "";

            int endCount = 0;

            int maxEndCount = 0;

            int i = 0;

            foreach (object obj in fileContent)
            {
                System.Windows.Forms.Application.DoEvents();

                string line = (string)obj;

                line = common.COH_IO.removeExtraSpaceBetweenWords(((string)obj).Replace("\t", " ")).Trim();

                if (line.ToLower().StartsWith("Texture".ToLower()))
                {
                    if (maxEndCount != 0 && endCount <= maxEndCount) 
                        endlessTricks.Add(string.Format("File: {0},\tTrick: {1}, line#: {2}", fileName, trickName, i));

                    isMatT = EndlessTricks.isMat(fileContent, i);

                    endCount = 0;

                    maxEndCount = 0;

                    if (isMatT)
                    {
                        trickName = line.ToLower().Replace("//", "#").Replace("texture ", "").Split('#')[0];
                    }
                    else
                    {
                        trickName = line.ToLower().Replace("//", "#").Replace("texture ", "").Split('#')[0].Replace(".tga", "").Replace(".txt", "");
                        isTextureTrick = true;
                    }
                }
                else if (line.ToLower().StartsWith("Trick ".ToLower()))
                {
                    if (maxEndCount != 0 && endCount <= maxEndCount)
                        endlessTricks.Add(string.Format("File: {0},\tTrick: {1}, line#: {2}", fileName, trickName, i));

                    trickName = line.ToLower().Replace("//", "#").Replace("trick ", "").Split('#')[0];
                    
                    isObjTrick = true;

                    endCount = 0;

                    maxEndCount = 0;
                }

                if (isObjTrick)
                {
                    if (line.ToLower().StartsWith("Autolod".ToLower()))
                    {
                        maxEndCount += 1;
                    }

                    if (line.ToLower().StartsWith("End".ToLower()))
                    {
                        endCount++;
                    }

                    if (endCount > maxEndCount || i == (fileContent.Count - 1))
                    {
                        isObjTrick = false;

                        if (endCount <= maxEndCount && i == (fileContent.Count - 1))
                            endlessTricks.Add(string.Format("File: {0},\tObjTrick: {1}, line#: {2}", fileName, trickName, i));
                    }
                }
                else if (isTextureTrick)
                {
                    if (line.ToLower().StartsWith("End".ToLower()))
                    {
                        endCount++;
                    }

                    if (endCount > maxEndCount || i == (fileContent.Count - 1))
                    {
                        isTextureTrick = false;

                        if (endCount <= maxEndCount && i == (fileContent.Count - 1))
                            endlessTricks.Add(string.Format("File: {0},\tTextureTrick: {1}, line#: {2}", fileName, trickName, i));
                    }
                }

                else if (isMatT)
                {
                    if (line.ToLower().StartsWith("Fallback".ToLower()))
                    {
                        maxEndCount = 1;
                    }
                    if (line.ToLower().StartsWith("End".ToLower()))
                    {
                        endCount++;
                    }

                    if (endCount > maxEndCount || i == (fileContent.Count - 1))
                    {
                        isMatT = false;

                        if(endCount <= maxEndCount && i == (fileContent.Count - 1))
                            endlessTricks.Add(string.Format("File: {0},\tMatTrick: {1}, line#: {2}", fileName, trickName, i));
                    }
                }

                i++;
            }

            return endlessTricks;
        }
    }
}
