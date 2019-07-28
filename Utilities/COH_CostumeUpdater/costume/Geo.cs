using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.costume
{
    [Serializable]
    class Geo
    {
        private string[] geoSearchPaths = { };
        public string geo;
        public string gTreeDisplayName;
        public string geoStr;
        public string geoHeader;
        public string path;

        public Geo(string gStr)
        {
            path = "";
            gTreeDisplayName = common.COH_IO.stripWS_Tabs_newLine_Qts(gStr, false);
            int qtIndex = gStr.IndexOf("\"");
            geoStr = gStr;
            geoHeader = gStr.Substring(0, qtIndex);
            geo = gStr.Substring(qtIndex);
        }
    }
}
