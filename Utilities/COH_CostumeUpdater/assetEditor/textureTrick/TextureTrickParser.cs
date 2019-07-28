using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.assetEditor.textureTrick
{
    class TextureTrickParser
    {
        public static ArrayList parse(string fileName, ref ArrayList fileContent)
        {
            ArrayList textureTricks = new ArrayList();

            TextureTrick tt;
            ArrayList textureTrickData = null;

            int textureTrickStartIndex = -1, textureTrickEndIndex = -1;
            string textureTrickName = "";
         
            bool isTextureTrick = false;
            bool createTextureTrick = false;
            int endCount = 0;
            int maxEndCount = 0;
            int i = 0;

            foreach (object obj in fileContent)
            {
                System.Windows.Forms.Application.DoEvents();

                string line = (string)obj;

                line = common.COH_IO.removeExtraSpaceBetweenWords(((string)obj).Replace("\t", " ")).Trim();

                if (line.ToLower().StartsWith("Texture ".ToLower()) && !MatParser.isMatTrick(fileContent, i))
                    //!line.ToLower().StartsWith("Texture X_".ToLower()))
                {
                    textureTrickName = line.Replace("//", "#").Replace("Texture ", "").Replace("texture ", "").Split('#')[0].Replace(".tga", "").Replace(".TGA", "").Replace(".Txt", "").Replace(".txt", "");
                    textureTrickStartIndex = i;
                    isTextureTrick = true;
                    createTextureTrick = false;
                    textureTrickData = new ArrayList();
                }
                if (isTextureTrick)
                {
                    string dataObj = (string)obj;

                    if (dataObj.ToLower().Contains("Texture_Name".ToLower()))
                    {
                        dataObj = common.COH_IO.fixInnerCamelCase((string)obj, "Texture_Name").Replace("Texture_Name.tga", "none").Replace("Texture_Name", "none");
                    }

                    textureTrickData.Add(dataObj);

                    if (line.ToLower().StartsWith("End".ToLower()))
                    {
                        endCount++;
                    }

                    if (endCount > maxEndCount || i == (fileContent.Count-1))
                    {
                        isTextureTrick = false;
                        textureTrickEndIndex = i;
                        createTextureTrick = true;
                    }
                    if (createTextureTrick)
                    {
                        tt = new TextureTrick(textureTrickStartIndex,
                            textureTrickEndIndex, textureTrickData, fileName, textureTrickName);

                        textureTricks.Add(tt);
                        endCount = 0;
                        maxEndCount = 0;
                    }
                }
                i++;
            }

            return textureTricks;
        }
    }

}
