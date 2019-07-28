using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.assetEditor.objectTrick

{
    public class ObjectTrick:assetEditor.Trick
    {
        public ObjectTrick(int sIdx, int eIdx, ArrayList trickData, string file_name, string matName)
            : base(sIdx, eIdx, trickData, file_name, matName) { }
        //public int startIndex;
        //public int endIndex;
        //public ArrayList data;
        //public string fileName;
        //public string name;
        //public bool deleted;

        //public ObjectTrick(int sIdx, int eIdx, ArrayList trickData, string file_name, string matName)
        //{
        //    startIndex = sIdx;
        //    endIndex = eIdx;
        //    data = trickData;
        //    fileName = file_name;
        //    name = matName;
        //    deleted = false;
        //}
    }
}
