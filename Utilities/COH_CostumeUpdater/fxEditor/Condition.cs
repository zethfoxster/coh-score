using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    [Serializable]
    public class Condition:ICloneable
    {
        public TreeNode treeViewNode;

        public int mCondWinDGIndx;

        public Dictionary<string, object> cParameters;
        
        public ArrayList events;
        
        public int fxIndex;
        
        public FX parent;

        public bool isCommented;
        
        public bool isDirty;

        public Condition(int fx_index, FX fx, bool isComment)
        {
            isDirty = false;
            parent = fx;
            events = new ArrayList();
            initialize_cParameters();
            this.fxIndex = fx_index;
            isCommented = isComment;
        }
        
        public object Clone()
        {
            System.IO.MemoryStream ms = new System.IO.MemoryStream();

            System.Runtime.Serialization.Formatters.Binary.BinaryFormatter bf = new System.Runtime.Serialization.Formatters.Binary.BinaryFormatter();

            bf.Serialize(ms, this);

            ms.Position = 0;

            object obj = bf.Deserialize(ms);

            ms.Close();

            return obj;

        }
        
        public int ImageIndex
        {
            get
            {
                int imgInd = 0;

                if (isDirty)
                    imgInd = 1;

                else if (!parent.isP4)
                    imgInd = 3;

                else if (parent.isReadOnly)
                    imgInd = 2;

                return imgInd;
            }
        }
       
        private void initialize_cParameters()
        {
            cParameters = new Dictionary<string, object>();
            cParameters["On"            ] = null;
            cParameters["Repeat"        ] = null;
            cParameters["RepeatJitter"  ] = null;
            cParameters["Random"        ] = null;
            cParameters["Chance"        ] = null;
            cParameters["DoMany"        ] = null;
            cParameters["Time"          ] = null;
            cParameters["TimeOfDay"     ] = null;
            cParameters["PrimeHit"      ] = null;
            cParameters["Prime1Hit"     ] = null;
            cParameters["Prime2Hit"     ] = null;
            cParameters["Prime3Hit"     ] = null;
            cParameters["Prime4Hit"     ] = null;
            cParameters["Prime5Hit"     ] = null;
            cParameters["PrimeBeamHit"  ] = null;
            cParameters["Cycle"         ] = null;
            cParameters["Force"         ] = null;
            cParameters["ForceThreshold"] = null;
            cParameters["Death"         ] = null;
            cParameters["TriggerBits"   ] = null;
            cParameters["DayStart"      ] = null;
            cParameters["DayEnd"        ] = null;
        }
    }
}
