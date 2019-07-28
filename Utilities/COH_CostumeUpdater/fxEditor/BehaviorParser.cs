using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.fxEditor
{
    class BehaviorParser
    {
        public static string fixBhvrKeys(string bhvrKey)
        {
            string[] bhvrKeyList = {"EndScale","Gravity","InitialVelocity","InitialVelocityJitter","PositionOffset","PyrRotate","PyrRotateJitter","Scale",
                                    "ScaleRate","Spin","SpinJitter","StartJitter","TrackMethod","TrackRate","Alpha","TintGeom","FadeInLength","FadeOutLength",
                                    "inheritGroupTint","BeColor1","ByTime1","PrimaryTint","SecondaryTint","PrimaryTint1","SecondaryTint1","BeColor2","ByTime2",
                                    "PrimaryTint2","SecondaryTint2","BeColor3","ByTime3","PrimaryTint3","SecondaryTint3","BeColor4","ByTime4","PrimaryTint4",
                                    "SecondaryTint4","BeColor5","ByTime5","PrimaryTint5","SecondaryTint5","StartColor","HueShift","HueShiftJitter",
                                    "Rgb0","Rgb0Time","Rgb0Next","Rgb1","Rgb1Time","Rgb1Next","Rgb2","Rgb2Time","Rgb2Next","Rgb3","Rgb3Time","Rgb3Next",
                                    "Rgb4","Rgb4Time","Rgb4Next","physics","physDensity",
                                    "physDebris","physDFriction","physGravity","PhysKillBelowSpeed","physRadius","physRestitution","physScale","physSFriction",
                                    "physForceCentripetal","physForcePower","physForcePowerJitter","physForceRadius","physForceType","physJointAnchor",
                                    "physJointAngLimit","physJointAngSpring","physJointAngSpringDamp","physJointBreakForce","physJointBreakTorque",
                                    "physJointCollidesWorld","physJointDOFs","physJointDrag","physJointLinLimit","physJointLinSpring","physJointLinSpringDamp",
                                    "Shake","ShakeFallOff","ShakeRadius","Blur","BlurFalloff","BlurRadius","PulseBrightness","PulsePeakTime","PulseClamp","AnimScale",
                                    "StAnim","Stretch","SplatFadeCenter","SplatFalloffType","SplatFlags","SplatNormalFade","SplatSetBack"};


            foreach (string s in bhvrKeyList)
            {
                if (s.ToLower().Equals(bhvrKey.ToLower()))
                    return s;
            }

            return bhvrKey;
        }

        public static Behavior parse(string fileName, ref ArrayList fileContent)
        {
            Behavior bhvr = null;

            int bhvrStartIndex = -1, bhvrEndIndex = -1;

            string bhvrName = System.IO.Path.GetFileNameWithoutExtension(fileName);

            bool isBhvr = false;
            bool createBhvr = false;
            bool isComment = false;

            int endCount = 0;
            int maxEndCount = 0;

            for (int j = 0; j < fileContent.Count; j++)
            {
                System.Windows.Forms.Application.DoEvents();

                string line = (string)fileContent[j];
                line = common.COH_IO.removeExtraSpaceBetweenWords(line.Replace("\t", " ")).Trim();
                line = line.Replace("//", "#");

                //don't skip last empty line
                if (line.Trim().Length < 1 && j != fileContent.Count - 1)
                    continue;

                if (line.StartsWith("#"))
                {
                    isComment = true;
                    line = line.Substring(1);
                }
                else
                {
                    isComment = false;
                }


                if (isBhvr != true && line.ToLower().StartsWith("Behavior".ToLower()))
                {
                    bhvrStartIndex = j;
                    isBhvr = true;
                    createBhvr = false;
                    bhvr = new Behavior(fileName);
                }

                else if (isBhvr)
                {
                    if (line.IndexOf(' ') > -1)
                    {
                        string tagStr = fixBhvrKeys(line.Substring(0, line.IndexOf(' ')).Trim());
                        string tagVal = line.Substring(tagStr.Length).Trim();
                        tagVal = isComment ? string.Format("#{0}", tagVal) : tagVal;

                        if (bhvr.bParameters.ContainsKey(tagStr))
                        {
                            if(!isComment)
                                bhvr.bParameters[tagStr] = tagVal;

                            //to avoid overwriting a value line by a commented line
                            else if(bhvr.bParameters[tagStr] == null)
                                bhvr.bParameters[tagStr] = tagVal;
                        }
                    }

                    if (line.ToLower().Equals("End".ToLower()))
                    {
                        endCount++;
                    }

                    if (endCount > maxEndCount || j == fileContent.Count - 1)
                    {
                        isBhvr = false;
                        bhvrEndIndex = j;
                        createBhvr = true;
                    }

                    if (createBhvr)
                    {
                        endCount = 0;
                        maxEndCount = 0;
                        return bhvr;
                    }
                }
            }
            return null;
        }
    }
}
