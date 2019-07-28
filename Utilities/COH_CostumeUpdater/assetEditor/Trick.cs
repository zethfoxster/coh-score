using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.assetEditor
{
    public class Trick
    {
        public int startIndex;
        public int endIndex;
        public ArrayList data;
        public string fileName;
        public string name;
        public bool deleted;

        public Trick(int sIdx, int eIdx, ArrayList trickData, string file_name, string matName)
        {
            startIndex = sIdx;
            endIndex = eIdx;
            data = trickData;
            fileName = file_name;
            name = matName;
            deleted = false;
        }
    }
}
