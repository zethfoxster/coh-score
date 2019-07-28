using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.fxEditor
{
    class FX_Parser
    {
        public static ArrayList mBones;

        public static string fixFXtag(string fxTag)
        {
            string[] fxTagList = {"Name","LifeSpan","FileAge","Lighting",           
                                  "PerformanceRadius","OnForceRadius","AnimScale",      
                                  "ClampMinScale","ClampMaxScale"};
            foreach (string s in fxTagList)
            {
                if(s.ToLower().Equals(fxTag.ToLower()))
                    return s;
            }

            return fxTag;
        }

        public static string fixFXflags(string fxflag)
        {
            string[] fxflagList = {"InheritAlpha", "IsArmor","InheritAnimScale","DontInheritBits","DontSuppress",               
                                  "DontInheritTexFromCostume","UseSkinTint","IsShield","IsWeapon","NotCompatibleWithAnimalHead",
                                  "InheritGeoScale","SoundOnly"};
            foreach (string s in fxflagList)
            {
                if (s.ToLower().Equals(fxflag.ToLower()))
                    return s;
            }

            return fxflag;
        }

        public static string fixConditionKeys(string condKey)
        {
            string[] condKeyList = {"On","Repeat","RepeatJitter","Random","Chance","DoMany",
                                   "Time","TimeOfDay","PrimeHit","Prime1Hit","Prime2Hit","Prime3Hit",
                                   "Prime4Hit","Prime5Hit","PrimeBeamHit","Cycle","Force","ForceThreshold",
                                   "Death","TriggerBits", "DayStart", "DayEnd"};

            foreach (string s in condKeyList)
            {
                if (s.ToLower().Equals(condKey.ToLower()))
                    return s;
            }

            return condKey;
        }
        
        public static string fixEventKeys(string eventKey)
        {
            string[] eventKeyList = {"Inherit","Update","JEvent","CEvent","CDestroy","JDestroy","CThresh","ParentVelocityFraction","EName",
                                    "Type","At","CRotation","HardwareOnly","SoftwareOnly","PhysicsOnly","CameraSpace","RayLength","AtRayFx",
                                    "Cape","AltPiv","AnimPiv","Anim","SetState","ChildFx","Magnet","LookAt","PMagnet","POther","Splat",
                                    "SoundNoRepeat","LifeSpan","LifeSpanJitter","Power","While","Until","WorldGroup","Flags","Create", "Destroy",
                                    "Local", "Start", "Posit", "StartPositOnly","PositOnly","SetState", "IncludeFx","SetLight", };

            foreach (string s in eventKeyList)
            {
                if (s.ToLower().Equals(eventKey.ToLower()))
                    return s;
            }

            return eventKey;
        }

        public static string fixEventAtKeys(string eventKey)
        {

            foreach (string s in FX_Parser.mBones)
            {
                if (s.Trim().ToLower().Equals(eventKey.ToLower()))
                    return s;
            }

            return eventKey;
        }

        public static ArrayList parse(string fileName, ref ArrayList fileContent)
        {
            ArrayList fxList = new ArrayList();

            ArrayList conditions = new ArrayList();

            ArrayList inputs = new ArrayList();

            if (FX_Parser.mBones == null)
            {
                FX_Parser.mBones = new ArrayList();
            }
            else
            {
                FX_Parser.mBones.Clear();

                common.COH_IO.fillList(FX_Parser.mBones, @"assetEditor/objectTrick/bones.txt", true);
            }

            FX fx = null;

            ArrayList fxData = null;

            int fxStartIndex = -1, fxEndIndex = -1;

            string fxName = System.IO.Path.GetFileNameWithoutExtension(fileName);

            bool isFx = false;

            bool createFx = false;

            int endCount = 0;

            int maxEndCount = 0;

            for (int j = 0; j < fileContent.Count; j++)
            {
                System.Windows.Forms.Application.DoEvents();

                string line = (string)fileContent[j];

                line = common.COH_IO.removeExtraSpaceBetweenWords(line.Replace("\t", " ")).Trim();

                line = line.Replace("//", "#");

                if (isFx != true && line.ToLower().StartsWith("FxInfo".ToLower()))
                {
                    fxStartIndex = j;

                    isFx = true;

                    createFx = false;

                    fx = new FX(fileName);

                    fxData = new ArrayList();
                }

                else if (isFx)
                {
                    fxData.Add(line);

                    bool isComment = line.StartsWith("#");
                     
                    //skip title lines that has multiple # Char
                    if (isComment && line.Substring(1).Trim().StartsWith("#") && j != fileContent.Count - 1)
                        continue;

                    if (line.ToLower().StartsWith("Flags".ToLower()))
                    {
                        string flagsStr = line.Substring("Flags ".Length).Split('#')[0];

                        string[] flags = flagsStr.Split(' ');

                        foreach (string flag in flags)
                        {
                            if (fx.flags.ContainsKey(fixFXflags(flag)))
                            {
                                fx.flags[flag] = true;
                            }
                        }

                    }

                    else if (line.ToLower().StartsWith("Input".ToLower()))
                    {
                        maxEndCount++;

                        string input = FX_Parser.getInput(fileContent, ref j);

                        if (input != null)
                        {
                            inputs.Add(input);
                        }
                        else
                            maxEndCount--;
                    }

                    //haldle comment for condition and fx tags
                    if(isComment)
                        line = line.Substring(1).Trim();

                    if (line.ToLower().StartsWith("Condition".ToLower()))
                    {
                        int conditionIndex = fxList.Count;

                        conditions.Add(new Condition(conditionIndex, fx, isComment));

                        fillCondition(fileContent, ref j, ref conditions);
                    }

                    else if (line.IndexOf(' ') > -1)
                    {
                        string tagStr = fixFXtag(line.Substring(0, line.IndexOf(' ')).Trim());

                        string tagVal = line.Substring(tagStr.Length).Trim();

                        if (fx.fParameters.ContainsKey(tagStr))
                        {
                            if (isComment)
                                tagVal = "#" + tagVal;

                            //if tgaVal is not commented overwrite param value
                            if (!isComment)
                                fx.fParameters[tagStr] = tagVal;

                            //to avoid overwriting a value line by a commented line
                            else if (fx.fParameters[tagStr] == null)
                                fx.fParameters[tagStr] = tagVal;
                        }
                    }
                    //&& !isComment added to skip commented end
                    if (line.ToLower().Equals("End".ToLower())&& !isComment)
                    {
                        endCount++;
                    }

                    if (endCount > maxEndCount || j == fileContent.Count - 1)
                    {
                        isFx = false;

                        fxEndIndex = j;

                        createFx = true;
                    }

                    if (createFx)
                    {
                        fx.conditions.AddRange(conditions);

                        fx.inputs.AddRange(inputs);

                        fxList.Add(fx);

                        endCount = 0;

                        maxEndCount = 0;
                    }
                }
            }

            return fxList;
        }

        public static string getInput(ArrayList fileContent, ref int startIndex)
        {
            for (startIndex++; startIndex < fileContent.Count; startIndex++)
            {
                System.Windows.Forms.Application.DoEvents();

                string line = (string)fileContent[startIndex];
                line = common.COH_IO.removeExtraSpaceBetweenWords(line.Replace("\t", " ")).Trim();
                line = line.Replace("//", "#");

                if (line.StartsWith("#") || line.Trim().Length < 1)
                    continue;

                if (line.ToLower().Equals("End".ToLower()))
                    break;

                return line;
            }

            return null;
        }

        public static void fillBhvrOverride(ArrayList fileContent, ref int startIndex, ref ArrayList bhvrOverride)
        {
            for (startIndex++; startIndex < fileContent.Count; startIndex++)
            {
                System.Windows.Forms.Application.DoEvents();

                string line = (string)fileContent[startIndex];

                line = common.COH_IO.removeExtraSpaceBetweenWords(line.Replace("\t", " ")).Trim();
                
                line = line.Replace("//", "#");

                if (line.Trim().Length < 1)
                    continue;

                bool isComment = line.StartsWith("#");

                //skip title lines that has multiple # Char
                if (isComment && line.Substring(1).Trim().StartsWith("#"))
                    continue;

                //haldle comment by stripping the first # char
                if (isComment)
                    line = line.Substring(1).Trim();

                //skip accidental commented BhvrOverride statement
                if (line.ToLower().StartsWith("BhvrOverride".ToLower()))
                    continue;

                //trim the end comment
                if (line.Contains("#"))
                    line = line.Substring(0, line.IndexOf('#')).Trim();

                if (line.ToLower().Equals("End".ToLower()))
                    break;
                //add the comment # char back to line to be processed later in bhvroverride processer
                if (isComment)
                    line = "#" + line;

                bhvrOverride.Add(line);
            }
        }

        public static void fillCondition(ArrayList fileContent, ref int startIndex, ref ArrayList conditions)
        {

            ArrayList events = new ArrayList();

            int endCount = 0;

            int maxEndCount = 0;
            
            for (startIndex++; startIndex < fileContent.Count; startIndex++)
            {
                System.Windows.Forms.Application.DoEvents();

                string line = (string)fileContent[startIndex];

                line = common.COH_IO.removeExtraSpaceBetweenWords(line.Replace("\t", " ")).Trim();

                line = line.Replace("//", "#");

                if (line.Trim().Length < 1)
                    continue;

                bool isComment = line.StartsWith("#");

                //skip title lines that has multiple # Char
                if (isComment && line.Substring(1).Trim().StartsWith("#"))
                    continue;

                //haldle comment by stripping the first # char
                if (isComment)
                    line = line.Substring(1).Trim();

                //skip accidental commented condition statement
                if (line.ToLower().StartsWith("Condition".ToLower()))
                    continue;
                
                else if (line.ToLower().StartsWith("Event".ToLower()))
                {
                    int conditionIndex = conditions.Count;

                    ((Condition)conditions[conditions.Count - 1]).events.Add(new Event(conditionIndex,
                        (Condition)conditions[conditions.Count - 1], isComment));

                    fillEvent(fileContent, ref  startIndex, ref  conditions);
                }

                else if (line.IndexOf(' ') > -1)
                {
                    string tagStr = fixConditionKeys(line.Substring(0, line.IndexOf(' ')).Trim());

                    string tagVal = line.Substring(tagStr.Length).Trim();

                    if (tagVal.Contains("#"))
                        tagVal = tagVal.Substring(0, tagVal.IndexOf('#')).Trim();

                    if (((Condition)conditions[conditions.Count - 1]).cParameters.ContainsKey(tagStr))
                    {
                        if (tagStr.Equals("On"))
                            tagVal = fixConditionKeys(tagVal);

                        if (tagStr.Equals("TriggerBits") &&
                            ((Condition)conditions[conditions.Count - 1]).cParameters[tagStr]!= null)
                        {                           
                            string tVal = (string)((Condition)conditions[conditions.Count - 1]).cParameters[tagStr];

                            tagVal = tVal + "; " + tagVal;
                        }

                        if (isComment)
                            tagVal = "#" + tagVal;

                        //if tgaVal is not commented overwrite param value
                        if (!isComment)
                            ((Condition)conditions[conditions.Count - 1]).cParameters[tagStr] = tagVal;

                        //to avoid overwriting a value line by a commented line
                        else if (((Condition)conditions[conditions.Count - 1]).cParameters[tagStr] == null)
                            ((Condition)conditions[conditions.Count - 1]).cParameters[tagStr] = tagVal;
                    }
                }

                if (line.ToLower().Equals("End".ToLower()))
                {
                    endCount++;
                }

                if (endCount > maxEndCount)
                {
                    break;
                }
            }
        }
        
        public static void fillEvent(ArrayList fileContent, ref int startIndex, ref ArrayList conditions)
        {

            ArrayList events = new ArrayList();

            int endCount = 0;

            int maxEndCount = 0;

            int eCount = ((Condition)conditions[conditions.Count - 1]).events.Count;

            for (startIndex++; startIndex < fileContent.Count; startIndex++)
            {
                System.Windows.Forms.Application.DoEvents();

                string line = (string)fileContent[startIndex];

                line = common.COH_IO.removeExtraSpaceBetweenWords(line.Replace("\t", " ")).Trim();

                line = line.Replace("//", "#");

                if (line.Trim().Length < 1)
                    continue;

                bool isComment = line.StartsWith("#");

                //skip title lines that has multiple # Char
                if (isComment && line.Substring(1).Trim().StartsWith("#"))
                    continue;

                //haldle comment by stripping the first # char
                if (isComment)
                    line = line.Substring(1).Trim();

                //skip accidental commented event statement
                if (line.ToLower().StartsWith("Event".ToLower()))
                    continue;         
                 
                //trim the end comment
                if (line.Contains("#"))
                    line = line.Substring(0, line.IndexOf('#')).Trim();

                if (line.ToLower().StartsWith("Geom ".ToLower()))
                {
                    string geom = line.Substring("Geom ".Length).Trim();

                    if (isComment)
                        geom = "#" + geom;
                    
                    ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eGeoms.Add(geom);
                }

                else if (line.ToLower().StartsWith("Part".ToLower()))
                {
                    string part = line.Substring("Part".Length);

                    part = part.Substring(part.IndexOf(" ") + 1);

                    if (isComment)
                        part = "#" + part;

                    ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eparts.Add(part);
                }

                else if (line.ToLower().StartsWith("Bhvr ".ToLower()))
                {
                    string bhvr = line.Substring("Bhvr ".Length).Trim();

                    if (isComment)
                        bhvr = "#" + bhvr;
                    
                    //if tgaVal is not commented overwrite param value
                    if (!isComment)
                    {
                        //overwrite the first bhvr with the uncommented bhvr since events could only have one bhvr.
                        if(((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eBhvrs.Count > 0)
                            ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eBhvrs[0] = bhvr;
                        else
                            ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eBhvrs.Add(bhvr);
                    }
                    //to avoid overwriting a value line by a commented line
                    else if (((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eBhvrs.Count == 0)
                        ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eBhvrs.Add(bhvr);
                }

                else if (line.ToLower().StartsWith("Sound ".ToLower()))
                {
                    string sound = line.Substring("Sound ".Length).Trim();

                    if (isComment)
                        sound = "#" + sound;

                    ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eSounds.Add(sound);
                }

                else if (line.ToLower().StartsWith("BhvrOverride".ToLower()))
                {
                    FX_Parser.fillBhvrOverride(fileContent, ref startIndex, ref ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eBhvrOverrides);
                }

                else if (line.IndexOf(' ') > -1)
                {
                    string tagStr = fixEventKeys(line.Substring(0, line.IndexOf(' ')).Trim());

                    string tagVal = line.Substring(tagStr.Length).Trim();

                    if (((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eParameters.ContainsKey(tagStr))
                    {
                        if (tagStr.Equals("Type"))
                            tagVal = fixEventKeys(tagVal);

                        else if (tagStr.Equals("At"))
                            tagVal = fixEventAtKeys(tagVal);

                        if (isComment)
                            tagVal = "#" + tagVal;

                        //if tgaVal is not commented overwrite param value
                        if (!isComment)
                            ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eParameters[tagStr] = tagVal;

                        //to avoid overwriting a value line by a commented line
                        else if (((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eParameters[tagStr] == null)
                            ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eParameters[tagStr] = tagVal;
                    }
                }
                else
                {
                    string tagStr = fixEventKeys(line.Trim());

                    if (((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eParameters.ContainsKey(tagStr))
                    {
                        string tagVal = "1";

                        switch (tagStr)
                        {
                            //flag like either is or not
                            case "HardwareOnly":
                            case "PhysicsOnly":
                            case "SoftwareOnly":
                            case "CameraSpace":
                                if (isComment)
                                    tagVal = "#" + tagVal;

                                //if tgaVal is not commented overwrite param value
                                if (!isComment)
                                    ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eParameters[tagStr] = tagVal;

                                //to avoid overwriting a value line by a commented line
                                else if (((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eParameters[tagStr] == null)
                                    ((Event)((Condition)conditions[conditions.Count - 1]).events[eCount - 1]).eParameters[tagStr] = tagVal;

                                break;
                        }
        
                    }
                }

                if (line.ToLower().Equals("End".ToLower()))
                {
                    endCount++;
                }

                if (endCount > maxEndCount)
                {
                    break;
                }
            }
        }
    }
}
