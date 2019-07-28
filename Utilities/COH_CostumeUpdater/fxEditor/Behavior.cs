using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;namespace COH_CostumeUpdater.fxEditor
{
    [Serializable]
    public class Behavior:ICloneable
    {
        public Dictionary<string, object> bParameters;
        
        public bool isCommented;
        
        public bool isDirty;
        
        public bool isP4;
        
        public bool isReadOnly;

        public string fileName;
        
        private object tag;

        public Behavior(string file_name)
        {
            isDirty = false;
            this.fileName = file_name;
            refreshP4ROAttributes();
            initialize_bParameters();
            this.tag = null;
            this.isCommented = false;        
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
        
        private void initialize_bParameters()
        {
            bParameters = new Dictionary<string, object>();

            bParameters["EndScale"] = null;

            bParameters["Gravity"] = null;

            bParameters["InitialVelocity"] = null;

            bParameters["InitialVelocityJitter"] = null;

            bParameters["PositionOffset"] = null;

            bParameters["PyrRotate"] = null;

            bParameters["PyrRotateJitter"] = null;

            bParameters["Scale"] = null;

            bParameters["ScaleRate"] = null;

            bParameters["Spin"] = null;

            bParameters["SpinJitter"] = null;

            bParameters["StartJitter"] = null;

            bParameters["TrackMethod"] = null;

            bParameters["TrackRate"] = null;

            bParameters["Alpha"] = null;

            bParameters["TintGeom"] = null;

            bParameters["FadeInLength"] = null;

            bParameters["FadeOutLength"] = null;

            bParameters["inheritGroupTint"] = null;

            bParameters["BeColor1"] = null;

            bParameters["ByTime1"] = null;

            bParameters["PrimaryTint"] = null;

            bParameters["SecondaryTint"] = null;

            bParameters["PrimaryTint1"] = null;

            bParameters["SecondaryTint1"] = null;

            bParameters["BeColor2"] = null;

            bParameters["ByTime2"] = null;

            bParameters["PrimaryTint2"] = null;

            bParameters["SecondaryTint2"] = null;

            bParameters["BeColor3"] = null;

            bParameters["ByTime3"] = null;

            bParameters["PrimaryTint3"] = null;

            bParameters["SecondaryTint3"] = null;

            bParameters["BeColor4"] = null;

            bParameters["ByTime4"] = null;

            bParameters["PrimaryTint4"] = null;

            bParameters["SecondaryTint4"] = null;

            bParameters["BeColor5"] = null;

            bParameters["ByTime5"] = null;

            bParameters["PrimaryTint5"] = null;

            bParameters["SecondaryTint5"] = null;

            bParameters["StartColor"] = null;

            bParameters["HueShift"] = null;

            bParameters["HueShiftJitter"] = null;

            bParameters["Rgb0"] = null;

            bParameters["Rgb0Time"] = null;

            bParameters["Rgb0Next"] = null;

            bParameters["Rgb1"] = null;

            bParameters["Rgb1Time"] = null;

            bParameters["Rgb1Next"] = null;

            bParameters["Rgb2"] = null;

            bParameters["Rgb2Time"] = null;

            bParameters["Rgb2Next"] = null;

            bParameters["Rgb3"] = null;

            bParameters["Rgb3Time"] = null;
            
            bParameters["Rgb3Next"] = null;

            bParameters["Rgb4"] = null;

            bParameters["Rgb4Time"] = null;

            bParameters["Rgb4Next"] = null;

            bParameters["physics"] = null;

            bParameters["physDensity"] = null;

            bParameters["physDebris"] = null;

            bParameters["physDFriction"] = null;

            bParameters["physGravity"] = null;

            bParameters["PhysKillBelowSpeed"] = null;

            bParameters["physRadius"] = null;

            bParameters["physRestitution"] = null;

            bParameters["physScale"] = null;

            bParameters["physSFriction"] = null;

            bParameters["physForceCentripetal"] = null;

            bParameters["physForcePower"] = null;

            bParameters["physForcePowerJitter"] = null;

            bParameters["physForceRadius"] = null;

            bParameters["physForceType"] = null;

            bParameters["physJointAnchor"] = null;

            bParameters["physJointAngLimit"] = null;

            bParameters["physJointAngSpring"] = null;

            bParameters["physJointAngSpringDamp"] = null;

            bParameters["physJointBreakForce"] = null;

            bParameters["physJointBreakTorque"] = null;

            bParameters["physJointCollidesWorld"] = null;

            bParameters["physJointDOFs"] = null;

            bParameters["physJointDrag"] = null;

            bParameters["physJointLinLimit"] = null;

            bParameters["physJointLinSpring"] = null;

            bParameters["physJointLinSpringDamp"] = null;

            bParameters["Shake"] = null;

            bParameters["ShakeFallOff"] = null;

            bParameters["ShakeRadius"] = null;

            bParameters["Blur"] = null;

            bParameters["BlurFalloff"] = null;

            bParameters["BlurRadius"] = null;

            bParameters["PulseBrightness"] = null;

            bParameters["PulsePeakTime"] = null;

            bParameters["PulseClamp"] = null;

            bParameters["AnimScale"] = null;

            bParameters["StAnim"] = null;

            bParameters["Stretch"] = null;

            bParameters["SplatFadeCenter"] = null;

            bParameters["SplatFalloffType"] = null;

            bParameters["SplatFlags"] = null;

            bParameters["SplatNormalFade"] = null;

            bParameters["SplatSetBack"] = null;
        }
    }
}
