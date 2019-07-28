using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.fxEditor
{
    class ParticalParser
    {
        public static string fixPartKeys(string partKey)
        {
            string[] partKeyList = {"Burst", "NewPerFrame", "BurbleAmplitude", "BurbleType", "BurbleFrequency", "BurbleThreshold", 
                                    "EmissionLifeSpan", "EmissionLifeSpanJitter", "EmissionType", "EmissionRadius", "EmissionHeight", 
                                    "EmissionStartJitter", "InitialVelocity", "InitialVelocityJitter", "VelocityJitter", "Magnetism", 
                                    "Gravity", "Drag", "WorldOrLocalPosition", "FrontOrLocalFacing", "Stickiness", "MoveScale", "TightenUp", 
                                    "StreakType", "StreakScale", "StreakOrient", "StreakDirection", "KickStart", "TimeToFull", "AlwaysDraw", 
                                    "SortBias", "DieLikeThis", "DeathAgeToZero", "StartSize", "StartSizeJitter", "ExpandRate", "ExpandType", 
                                    "EndSize", "OrientationJitter", "Spin", "SpinJitter",  "Flags", "IgnoreFxTint", "Blend_mode", "TextureName", 
                                    "TexScroll1", "TexScrollJitter1", "AnimFrames1", "AnimPace1", "AnimType1", "TextureName2", "TexScroll2", 
                                    "TexScrollJitter2", "AnimFrames2", "AnimPace2", "AnimType2", "Alpha", "FadeInBy", "FadeOutStart", "FadeOutBy", 
                                    "StartColor", "ColorChangeType", "PrimaryTint", "SecondaryTint", "ByTime1", "BeColor1", "PrimaryTint1", 
                                    "SecondaryTint1", "ByTime2", "BeColor2", "PrimaryTint2", "SecondaryTint2", "ByTime3", "BeColor3", "PrimaryTint3", 
                                    "SecondaryTint3", "ByTime4", "BeColor4", "PrimaryTint4", "SecondaryTint4", "ColorOffset", "ColorOffsetJitter"
                                    };


            foreach (string s in partKeyList)
            {
                if (s.ToLower().Equals(partKey.ToLower()))
                    return s;
            }

            return partKey;
        }

        public static Partical parse(string fileName, ref ArrayList fileContent)
        {
            Partical part = null;

            int partStartIndex = -1, partEndIndex = -1;

            string partName = System.IO.Path.GetFileNameWithoutExtension(fileName);

            bool isPart = false;
            bool createPart = false;
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


                if (isPart != true && line.ToLower().StartsWith("System".ToLower()))
                {
                    partStartIndex = j;
                    isPart = true;
                    createPart = false;
                    part = new Partical(fileName, false);
                }

                else if (isPart)
                {
                    if (line.IndexOf(' ') > -1)
                    {
                        string tagStr = fixPartKeys(line.Substring(0, line.IndexOf(' ')).Trim());

                        if (tagStr.Equals("Flags"))
                        {
                            string flagsStr = line.Substring("Flags ".Length).Split('#')[0];

                            string[] flags = flagsStr.Split(' ');

                            foreach (string flagStr in flags)
                            {
                                string flagKey = fixPartKeys(flagStr.Trim());

                                if (part.pParameters.ContainsKey(flagKey))
                                {
                                    if (!isComment)
                                        part.pParameters[flagKey] = "1";

                                    //to avoid overwriting a value line by a commented line
                                    else if (part.pParameters[flagKey] == null)
                                        part.pParameters[flagKey] = "#1";
                                }
                            }
                        }
                        else
                        {
                            string tagVal = line.Substring(tagStr.Length).Trim();
                            tagVal = isComment ? string.Format("#{0}", tagVal) : tagVal;

                            if (part.pParameters.ContainsKey(tagStr))
                            {
                                if (!isComment)
                                    part.pParameters[tagStr] = tagVal;

                                //to avoid overwriting a value line by a commented line
                                else if (part.pParameters[tagStr] == null)
                                    part.pParameters[tagStr] = tagVal;
                            }
                        }
                    }

                    if (line.ToLower().Equals("End".ToLower()))
                    {
                        endCount++;
                    }

                    if (endCount > maxEndCount || j == (fileContent.Count - 1))
                    {
                        isPart = false;
                        partEndIndex = j;
                        createPart = true;
                    }

                    if (createPart)
                    {
                        endCount = 0;
                        maxEndCount = 0;
                        return part;
                    }
                }
            }
            return null;
        }
    }
}
