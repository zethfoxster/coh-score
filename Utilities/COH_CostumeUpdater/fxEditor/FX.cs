using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.fxEditor
{
    [Serializable]
    public class FX:ICloneable
    {
        public ArrayList conditions;
        
        public ArrayList inputs;
        
        public Dictionary<string, bool> flags;
        
        public Dictionary<string, string> fParameters;
        
        public string fxFileName;
        
        public bool isCommented;
        
        public bool isDirty;
        
        public bool isP4;
        
        public bool isReadOnly;

        public FX(string file_name)
        {
            isDirty = false;
            fxFileName = file_name;
            refreshP4ROAttributes();

            conditions = new ArrayList();//add new Condition()
            
            inputs = new ArrayList();
            
            initialize_fParameters();
            
            initializeFlags();
            
            isCommented = false;
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

        public void refreshP4ROAttributes()
        {
            isP4 = assetsMangement.CohAssetMang.isP4file(fxFileName);
            System.IO.FileInfo fi = new System.IO.FileInfo(fxFileName);
            isReadOnly = (fi.Attributes & System.IO.FileAttributes.ReadOnly) != 0;
        }

        public int ImageIndex
        {
            get
            {
                int imgInd = 0;

                if (isDirty)
                    imgInd = 1;

                else if (!isP4)
                    imgInd = 3;

                else if (isReadOnly)
                    imgInd = 2;

                return imgInd;
            }
        }
  
        private void initialize_fParameters()
        {
            fParameters = new Dictionary<string, string>();//FxInfo
            fParameters["Name"]                 = fxFileName; //Name med_fire
            fParameters["LifeSpan"]             = null; //LifeSpan 200
            fParameters["FileAge"]              = null; // not in any fx files
            fParameters["Lighting"]             = null; //Lighting	1 or 0	##Generate your own light
            fParameters["PerformanceRadius"]    = null; // not in any fx files
            fParameters["OnForceRadius"]        = null; //OnForceRadius 3
            fParameters["AnimScale"]            = null; //AnimScale	1	# 0.0+
            fParameters["ClampMinScale"]        = null; //ClampMinScale 1.0 1.0 1.0
            fParameters["ClampMaxScale"]        = null; //ClampMaxScale 1.0 1.0 1.0
        }

        private void initializeFlags()
        {
            flags = new Dictionary<string, bool>();
	        flags["InheritAlpha"]                = false;
            flags["IsArmor"]                     = false; /* no longer used */
            flags["InheritAnimScale"]            = false;
            flags["DontInheritBits"]             = false;
            flags["DontSuppress"]                = false;
            flags["DontInheritTexFromCostume"]   = false;
            flags["UseSkinTint"]                 = false; //not in wiki but in fxinfo.c
            flags["IsShield"]                    = false; //not in wiki but in fxinfo.c
            flags["IsWeapon"]                    = false; //not in wiki but in fxinfo.c
            flags["NotCompatibleWithAnimalHead"] = false;
            flags["InheritGeoScale"]             = false; //not in wiki but in fxinfo.c
            flags["SoundOnly"]                   = false; //obsolete
        }
    }
}
