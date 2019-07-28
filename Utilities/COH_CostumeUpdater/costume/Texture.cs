using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.costume
{
    [Serializable]
    class Texture
    {
        private string texWOexMark;
        private string[] texSearchPaths = {@"texture_library", @"tricks"};
        public string root;
        public string texStr;
        public string tTreeDisplayName;
        public string tType;
        public string texName;
        public string path;
        public bool isTrick;

        public Texture(string pathRoot, string tStr)
        {
            tTreeDisplayName = common.COH_IO.stripWS_Tabs_newLine_Qts(tStr, false);
            path = "";
            root = pathRoot;
            texStr = tStr;
            tType = tStr.Substring(tStr.IndexOf("Tex"), 4);
            setTxWOexcl(texStr);
            setTexName();
            isTrick = isShader();
        }

        public string findTexture()
        {
            bool found = false;

            if (path.Length == 0 && 
                !texName.ToLower().StartsWith("none") &&
                !texName.ToLower().StartsWith("mask"))
            {
                if (isShader())
                    setShaderPath(root + texSearchPaths[1], ref found);
                else
                    setTexturePath(root + texSearchPaths[0], ref found);
            }

            return path;
        }
        private void setTxWOexcl(string str)
        {
            if (str.Contains("\""))
            {
                int start = str.IndexOf("\"") + 1;
                int len = str.LastIndexOf("\"") - start;
                texWOexMark = texStr.Substring(start,len);
                texWOexMark = texWOexMark.Replace("!X_", "").Replace("!x","").Replace("!", "");
            }
            else
                texWOexMark = texStr.Replace("!X_", "").Replace("!x", "").Replace("!", "");
        }
        private void setTexName()
        {
            
            if (texWOexMark.Contains("/"))
                texName = texWOexMark.Substring(texWOexMark.LastIndexOf("/"));
            else if (texWOexMark.Contains(@"\"))
                texName = texWOexMark.Substring(texWOexMark.LastIndexOf(@"\"));
            else
                texName = texWOexMark;
        }

       
        private void setTexturePath(string dPath, ref bool found)
        {
            int i = 0;
            string mPath = dPath + @"\" + texWOexMark.Replace("/", @"\") + ".texture";

            if (File.Exists(mPath))
            {
                path = mPath;
                found = true;
                return;
            }
            else
            {
                try
                {
                    DirectoryInfo dInfo = new DirectoryInfo(dPath);
                    DirectoryInfo[] dInfos = dInfo.GetDirectories();

                    while (!found && i < dInfos.Length)
                    {
                        DirectoryInfo di = dInfos[i];
                        mPath = di.FullName + @"\" + texWOexMark.Replace("/", @"\") + ".texture";

                        if (File.Exists(mPath))
                        {
                            path = mPath;
                            found = true;
                            return;
                        }
                        setTexturePath(di.FullName, ref found);
                        i++;
                    }

                }
                catch (Exception ex)
                {
                    System.Windows.Forms.MessageBox.Show(ex.Message);

                }
            }

        }

        public bool isShader()
        {
            if (texStr.Contains("!"))
                return texStr.ToUpper().StartsWith("!X_");
            else
                return texStr.ToUpper().StartsWith("X_");
        }

        private bool hasShader(string shaderPath)
        {
            string searchTX = "Texture X_" + texWOexMark;
            StreamReader sr = new StreamReader(shaderPath);
            string line;
            while ((line = sr.ReadLine()) != null)
            {
                if(line.Contains(searchTX))
                {
                    sr.Close();
                    return true;
                }
            }
            sr.Close();
            return false;
        }

        private void setShaderPath(string dPath,ref bool found)
        {
            int i = 0;
            int ii = 0;
            try
            {
                DirectoryInfo dInfo = new DirectoryInfo(dPath);
                FileInfo[] fis = dInfo.GetFiles();
                while (!found && ii < fis.Length)
                {
                    found = hasShader(fis[ii].FullName);
                    if (found)
                        path = fis[ii].FullName;
                    ii++;
                }
                
                DirectoryInfo[] dInfos = dInfo.GetDirectories();
                while (!found && i < dInfos.Length)
                {
                    DirectoryInfo di = dInfos[i];
                    setShaderPath(di.FullName, ref found);
                    i++;
                }             
            }
            catch (Exception ex)
            {
                System.Windows.Forms.MessageBox.Show(ex.Message);
                
            }           
        }
    }
}
