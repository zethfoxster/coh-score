using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.assetEditor
{
    class MatParser
    {
        public static bool isMatTrick(ArrayList fileContent, int startInd)
        {
            int i = startInd;
            string line = common.COH_IO.removeExtraSpaceBetweenWords(((string)fileContent[i]).Replace("\t", " ")).Trim();

            if(line.ToLower().StartsWith("Texture X_".ToLower()))
                return true;

            while (!line.ToLower().Equals("end") && i < fileContent.Count)
            {
                i++;

                if (line.ToLower().StartsWith("base1"))
                    return true;
                line = common.COH_IO.removeExtraSpaceBetweenWords(((string)fileContent[i]).Replace("\t", " ")).Trim();
            }

            return false;
        }

        public static ArrayList parse(string fileName, ref ArrayList fileContent)
        {
            ArrayList matTricks = new ArrayList();

            materialTrick.MatTrick mt;
            ArrayList matData = null;

            int matStartIndex = -1, matEndIndex = -1;
            string matName = "";

            bool isMatTrick = false;
            bool createMatTrick = false;
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
                    isMatTrick = MatParser.isMatTrick(fileContent, i);
                    //commented below as 6/28/11
                    //isMatTrick = true;
                    if (isMatTrick)
                    {
                        matName = line.Replace("//", "#").Replace("Texture ", "").Replace("texture ", "").Split('#')[0];
                        
                        matStartIndex = i;
                        createMatTrick = false;
                        matData = new ArrayList();
                    }
                }

                if (isMatTrick)
                {
                    string dataObj = (string)obj;

                    if (dataObj.ToLower().Contains("Texture_Name".ToLower()))
                    {
                        dataObj = common.COH_IO.fixInnerCamelCase((string)obj, "Texture_Name").Replace("Texture_Name.tga", "none").Replace("Texture_Name", "none");
                    }

                    matData.Add(dataObj);

                    if (line.ToLower().StartsWith("Fallback".ToLower()))
                    {
                        maxEndCount = 1;
                    }

                    if (line.ToLower().StartsWith("End".ToLower()))
                    {
                        endCount++;
                    }

                    if (endCount > maxEndCount || i == (fileContent.Count-1))
                    {
                        isMatTrick = false;
                        matEndIndex = i;
                        createMatTrick = true;
                    }
                    if (createMatTrick)
                    {
                        mt = new COH_CostumeUpdater.assetEditor.materialTrick.MatTrick(matStartIndex,
                            matEndIndex, matData, fileName, matName);

                        matTricks.Add(mt);
                        endCount = 0;
                        maxEndCount = 0;
                    }
                }
                i++;
            }

            return matTricks;
        }
    }

}
