using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.assetEditor.objectTrick
{
    class ObjectTrickParser
    {
        public static ArrayList parse(string fileName, ref ArrayList fileContent)
        {
            ArrayList objectTricks = new ArrayList();

            ObjectTrick ot;
            ArrayList objTrickData = null;

            int objTrickStartIndex = -1, objTrickEndIndex = -1;
            string objTrickName = "";

            bool isObjTrick = false;
            bool createObjTrick = false;
            int endCount = 0;
            int maxEndCount = 0;
            int i = 0;

            foreach (object obj in fileContent)
            {
                System.Windows.Forms.Application.DoEvents();

                string line = (string)obj;

                line = common.COH_IO.removeExtraSpaceBetweenWords(((string)obj).Replace("\t", " ")).Trim();

                if (line.ToLower().StartsWith("Trick ".ToLower()))
                {
                    objTrickName = line.Replace("//", "#").Replace("Trick ", "").Replace("trick ", "").Split('#')[0];
                    objTrickStartIndex = i;
                    isObjTrick = true;
                    createObjTrick = false;
                    objTrickData = new ArrayList();
                }
                if (isObjTrick)
                {
                    string dataObj = (string)obj;

                    if (dataObj.ToLower().Contains("Texture_Name".ToLower()))
                    {
                        dataObj = common.COH_IO.fixInnerCamelCase((string)obj, "Texture_Name").Replace("Texture_Name.tga", "none").Replace("Texture_Name", "none");
                    }

                    objTrickData.Add(dataObj);

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
                        objTrickEndIndex = i;
                        createObjTrick = true;
                    }
                    if (createObjTrick)
                    {
                        ot = new ObjectTrick(objTrickStartIndex,
                            objTrickEndIndex, objTrickData, fileName, objTrickName);

                        objectTricks.Add(ot);
                        endCount = 0;
                        maxEndCount = 0;
                    }
                }
                i++;
            }

            return objectTricks;
        }
    }

}
