using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.costume
{
    [Serializable]
    class Fx
    {
        private string[] fxSearchPaths = { };
        public string fx;
        public string fxStr;
        public string path;
        public string subPath;
        public string fxTreeNodeText;
        public string pathRoot;
        public bool fileExists;

        public Fx(string fStr, string pRoot)
        {
            fxStr = fStr;
            pathRoot = pRoot;
            fxTreeNodeText = common.COH_IO.stripWS_Tabs_newLine_Qts(fStr, false);
            subPath = common.COH_IO.getFiledStr(fStr);
            fx = System.IO.Path.GetFileName(path);
            if (System.IO.File.Exists(subPath))
            {
                path = subPath;
                fileExists = true;
            }
            else
                setPath();
            path = path.Replace(@"\", "/");
        }
        private void setPath()
        {
            path = pathRoot + @"fx\" + subPath;
            path = path.ToLower();

            if (System.IO.File.Exists(path))
            {
                fileExists = true;
                return;
            }
            else
            {
                fileExists = false;
            }
        }
    }
}
