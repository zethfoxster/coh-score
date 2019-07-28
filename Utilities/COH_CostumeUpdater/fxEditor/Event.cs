using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    [Serializable]
    public class Event:ICloneable
    {
        public TreeNode treeViewNode;

        public int mEventWinDGIndx;

        public ArrayList eBhvrs;
        
        public ArrayList eBhvrsObject;
        
        public ArrayList eBhvrOverrides;
        
        public ArrayList eGeoms;
        
        public ArrayList eSounds;
        
        public ArrayList eparts;
        
        public ArrayList epartsObject;
        
        public int conditonIndex;
        
        public bool isDirty;
        
        public Condition parent;

        public string eName;
        
        public Dictionary<string, object> eParameters;
        
        public Dictionary<string, object> eBhvrOverrideDic;

        public bool isCommented;       

        public Event(int condition_index, Condition condition, bool isComment)
        {
            mEventWinDGIndx = -1;

            parent = condition;
            isDirty = false;
            eBhvrs = new ArrayList();
            eBhvrsObject = new ArrayList();
            eBhvrOverrides = new ArrayList();
            eGeoms = new ArrayList();
            eSounds = new ArrayList();
            eparts = new ArrayList();
            epartsObject = new ArrayList();
            initialize_eParameters();
            eBhvrOverrideDic = new Dictionary<string, object>();
            this.conditonIndex = condition_index;
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

                else if (!parent.parent.isP4)
                    imgInd = 3;

                else if (parent.parent.isReadOnly)
                    imgInd = 2;

                return imgInd;
            }
        }

        public bool isInBhvrObject(string bhvrFileName)
        {
            foreach (Behavior b in this.eBhvrsObject)
            {
                string fName = System.IO.Path.GetFileName(b.fileName).ToLower();
                string bFName = System.IO.Path.GetFileName(bhvrFileName).ToLower();

                if(fName.Equals(bFName)) 
                    return true;
            }

            return false;
        }

        public bool isInPartObject(string partFileName)
        {
            foreach (Partical p in this.epartsObject)
            {
                string fName = System.IO.Path.GetFileName(p.fileName).ToLower();
                string pFName = System.IO.Path.GetFileName(partFileName).ToLower();

                if (fName.Equals(pFName))
                    return true;
            }

            return false;
        }

        //rather than refactoring the fx parser build the dictionary here
        public void buildBhvrOvrDic()
        {
            eBhvrOverrideDic.Clear();

            foreach (string str in eBhvrOverrides)
            {
                bool isComment = str.StartsWith("#");
                string tmpStr = str;

                //haldle comment by stripping the first # char
                if (isComment)
                    tmpStr = tmpStr.Substring(1).Trim();

                if (tmpStr.Trim().Length > 0 && tmpStr.IndexOf(' ') != -1)
                {
                    string tagStr = BehaviorParser.fixBhvrKeys(tmpStr.Substring(0, tmpStr.IndexOf(' ')).Trim());

                    string tagVal = tmpStr.Substring(tagStr.Length).Trim();

                    eBhvrOverrideDic[tagStr] = isComment ? ("#" + tagVal) : tagVal;
                }
            }
        }
        
        private void initialize_eParameters()
        {
            eParameters = new Dictionary<string, object>();
            eParameters["AltPiv"] = null;		
            eParameters["Anim"] = null;	
            eParameters["AnimPiv"] = null;		
            eParameters["At"   ] = null;	
            eParameters["AtRayFx"] = null;
            eParameters["Type"] = null;	
            eParameters["CameraSpace"] = null;
            eParameters["Cape"] = null;	
            eParameters["CDestroy"] = null;  		
            eParameters["CEvent" ] = null;   			
            eParameters["ChildFx"] = null;	
            eParameters["Create"] = null;//type
            eParameters["CRotation"] = null; 
            eParameters["CThresh" ] = null;                 
            eParameters["Destroy"] = null;//type
            eParameters["EName"] = null;
            eParameters["Flags"] = null;//Flags	OneAtATime AdoptParentEntity NoReticle PowerLoopingSound
            eParameters["HardwareOnly"] = null;
            eParameters["IncludeFx"] = null;
            eParameters["Inherit"] = null;	
            eParameters["JDestroy"] = null;  
            eParameters["JEvent" ] = null;   	
            eParameters["LifeSpan"] = null;	
            eParameters["LifeSpanJitter"] = null;
            eParameters["Local"] =  null;//type
            eParameters["LookAt"] = null;		
            eParameters["Magnet"] = null;		
            eParameters["ParentVelocityFraction"] = null;
            eParameters["PhysicsOnly"] = null;
            eParameters["PMagnet"] = null;	
            eParameters["Posit"] = null;//type
            eParameters["PositOnly"] = null;//type
            eParameters["POther"] = null;		
            eParameters["Power"	] = null;	
            eParameters["RayLength"] = null;
            eParameters["SetLight"] = null;//type
            eParameters["SetState"] = null;
            eParameters["SoftwareOnly"] = null;
            eParameters["SoundNoRepeat"] = null;
            eParameters["Splat"	] = null;	
            eParameters["Start"] = null;//type
            eParameters["StartPositOnly"] = null;//type
            eParameters["Until"	] = null;	
            eParameters["Update" ] = null;            
            eParameters["While"	] = null;	
            eParameters["WorldGroup"] = null;

        }
    }
}
