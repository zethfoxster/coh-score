using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    [Serializable]
    public class Partical:ICloneable
    {
        public Dictionary<string, object> pParameters;

        public bool isCommented;
        
        public bool isDirty;
        
        public bool isP4;
        
        public bool isReadOnly;       
        
        public string fileName;
        
        private object tag;

        public Partical(string file_name, bool isComment)
        {
            isDirty = false;
            this.fileName = file_name;
            this.refreshP4ROAttributes();
            initialize_pParameters();
            this.tag = null;
            this.isCommented = isComment;        
        }

        public bool renameFile(string new_name)
        {
            string oldName = this.fileName;

            string newName = Path.GetDirectoryName(oldName).Replace("/","\\") + "\\" + new_name + (new_name.ToLower().EndsWith(".part")?"":".part");

            bool isNewNameInP4 = assetsMangement.CohAssetMang.isP4file(newName);

            if (isNewNameInP4)
            {
                MessageBox.Show(string.Format("New File Name exists in P4!\r\nCan not rename {0} to {1}",Path.GetFileName(oldName), new_name));
                return false;
            }


            if (isP4)
            {
                if (isReadOnly)
                {
                    if (assetsMangement.CohAssetMang.isFileCheckedOutByAnyone(oldName))
                    {
                        string results = assetsMangement.CohAssetMang.whoCheckedOutFile(oldName);
                        MessageBox.Show("Can not rename File!\r\n" + results);
                        return false;
                    }

                    string renameStr = assetsMangement.CohAssetMang.rename(oldName, newName.Trim());

                    MessageBox.Show(renameStr);

                    this.fileName = newName.Trim();

                    return true;
                }
                else
                {
                    string msg = string.Format("File is checked out by you!\r\nCan not rename {0} while it is open for edit\r\nPlease Check file in first then try to use rename again.", oldName);
                    MessageBox.Show(msg);
                    return false;
                }
            }
            else
            {
                try
                {
                    File.Move(oldName, newName.Trim());

                    if (File.Exists(newName.Trim()))
                    {
                        this.fileName = newName.Trim();

                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message);
                    return false;
                }
            }

            return false;
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
            isP4 = assetsMangement.CohAssetMang.isP4file(fileName);
            System.IO.FileInfo fi = new System.IO.FileInfo(fileName);
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
       
        public Object Tag
        {
            get
            {
                return tag;
            }
            set
            {
                tag = value;
            }
        }

        private void initialize_pParameters()
        {
            pParameters = new Dictionary<string, object>();
            pParameters["Alpha"]  = null;
            pParameters["AlwaysDraw"]  = null;
            pParameters["AnimFrames1"]  = null;
            pParameters["AnimFrames2"]  = null;
            pParameters["AnimPace1"]  = null;
            pParameters["AnimPace2"]  = null;
            pParameters["AnimType1"]  = null;
            pParameters["AnimType2"]  = null;
            pParameters["BeColor1"]  = null;
            pParameters["BeColor2"]  = null;
            pParameters["BeColor3"]  = null;
            pParameters["BeColor4"]  = null;
            pParameters["Blend_mode"]  = null;
            pParameters["BurbleAmplitude"]  = null;
            pParameters["BurbleFrequency"]  = null;
            pParameters["BurbleThreshold"]  = null;
            pParameters["BurbleType"]  = null;
            pParameters["Burst"]  = null;
            pParameters["ByTime1"]  = null;
            pParameters["ByTime2"]  = null;
            pParameters["ByTime3"]  = null;
            pParameters["ByTime4"]  = null;
            pParameters["ColorChangeType"]  = null;
            pParameters["ColorOffset"]  = null;
            pParameters["ColorOffsetJitter"]  = null;
            pParameters["DeathAgeToZero"]  = null;
            pParameters["DieLikeThis"]  = null;
            pParameters["Drag"]  = null;
            pParameters["EmissionHeight"]  = null;
            pParameters["EmissionLifeSpan"]  = null;
            pParameters["EmissionLifeSpanJitter"]  = null;
            pParameters["EmissionRadius"]  = null;
            pParameters["EmissionStartJitter"]  = null;
            pParameters["EmissionType"]  = null;
            pParameters["EndSize"]  = null;
            pParameters["ExpandRate"]  = null;
            pParameters["ExpandType"]  = null;
            pParameters["FadeInBy"]  = null;
            pParameters["FadeOutBy"]  = null;
            pParameters["FadeOutStart"]  = null;
            pParameters["FrontOrLocalFacing"]  = null;
            pParameters["Gravity"] = null;
            pParameters["IgnoreFxTint"] = null;
            pParameters["InitialVelocity"]  = null;
            pParameters["InitialVelocityJitter"]  = null;
            pParameters["KickStart"]  = null;
            pParameters["Magnetism"]  = null;
            pParameters["MoveScale"]  = null;
            pParameters["NewPerFrame"]  = null;
            pParameters["OrientationJitter"]  = null;
            pParameters["PrimaryTint"]  = null;
            pParameters["PrimaryTint1"]  = null;
            pParameters["PrimaryTint2"]  = null;
            pParameters["PrimaryTint3"]  = null;
            pParameters["PrimaryTint4"]  = null;
            pParameters["SecondaryTint"]  = null;
            pParameters["SecondaryTint1"]  = null;
            pParameters["SecondaryTint2"]  = null;
            pParameters["SecondaryTint3"]  = null;
            pParameters["SecondaryTint4"]  = null;
            pParameters["SortBias"]  = null;
            pParameters["Spin"]  = null;
            pParameters["SpinJitter"]  = null;
            pParameters["StartColor"]  = null;
            pParameters["StartSize"]  = null;
            pParameters["StartSizeJitter"]  = null;
            pParameters["Stickiness"]  = null;
            pParameters["StreakDirection"]  = null;
            pParameters["StreakOrient"]  = null;
            pParameters["StreakScale"]  = null;
            pParameters["StreakType"]  = null;
            pParameters["TexScroll1"]  = null;
            pParameters["TexScroll2"]  = null;
            pParameters["TexScrollJitter1"]  = null;
            pParameters["TexScrollJitter2"]  = null;
            pParameters["TextureName"]  = null;
            pParameters["TextureName2"]  = null;
            pParameters["TightenUp"]  = null;
            pParameters["TimeToFull"]  = null;
            pParameters["VelocityJitter"]  = null;
            pParameters["WorldOrLocalPosition"]  = null;
        }
    }
}
