using System;
using System.IO;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.fxEditor
{
    class ParticalKeys
    {
        public ArrayList keysList;

        private string gameBranch;

        public ParticalKeys(string game_branch)
        {
            gameBranch = game_branch;

            keysList = new ArrayList();

            populateKeys();
        }

        private void populateKeys()
        {
            
            string[] partList = System.IO.Directory.GetFiles(@"C:\" + gameBranch + @"\data\fx", "*.part", SearchOption.TopDirectoryOnly);
            
            string[] partListWp = new string[partList.Length];

            int a = 0;
            //build part list for dieLikeThis combobox
            foreach (string str in partList)
            {
                partListWp[a++] = System.IO.Path.GetFileName(str);
            }

            /************************* Emitter Keys*******************************************/
            
            keysList.Add(new ParticalKey("Burst", "Emitter", null, "particle", 1, 0, 10000, new decimal[] { 0.0m }, 0, false, null, false));

            keysList.Add(new ParticalKey("NewPerFrame", "Emitter", null, "partical/frame", 1, 0, 1000000, new decimal[] { 0.0m }, 6, false, null, false));

            keysList.Add(new ParticalKey("BurbleAmplitude", "Emitter", null, "rand part/frame", 1, 0, 1000, new decimal[] { 0.0m }, 6, false, null, false));

            //0 = Sine Wave, 1 = Random Curve
            keysList.Add(new ParticalKey("BurbleType", "Emitter", null, "function", 0, 0, 1, new decimal[] { 0.0m }, 0, false, 
                new string[]{"0 (Sine Wave)", "1 (Random Curve)"}, false));

            keysList.Add(new ParticalKey("BurbleFrequency", "Emitter", null, "times", 1, 0, 10000, new decimal[] { 1.0m }, 6, false, null, false));

            keysList.Add(new ParticalKey("BurbleThreshold", "Emitter", null, "percent", 1, 0, 1, new decimal[] { 0.0m }, 6, false, null, false));

            keysList.Add(new ParticalKey("EmissionLifeSpan", "Emitter", null, "age", 1, 0, 100000, new decimal[] { 1.0m }, 6, false, null, false));

            keysList.Add(new ParticalKey("EmissionLifeSpanJitter", "Emitter", null, "rand/age", 1, 0, 100000, new decimal[] { 0.0m }, 6, false, null, false));


            //#Emitter Shape
            //line & line Even must use POther in fxinfo
            //0 = Point, 1 = Line, 2 = Cylinder, 3 = Even Trail, 4 = Sphere, 5 = Line Even
            keysList.Add(new ParticalKey("EmissionType", "Emitter", null, "region", 0, 0, 5, new decimal[] { 0.0m }, 0, false,
                new string [] {"0 (Point)", "1 (Line)", "2 (Cylinder)", "3 (Even Trail)", "4 (Sphere)", "5 (Line Even)"}, false));

            keysList.Add(new ParticalKey("EmissionRadius", "Emitter", null, "feet", 1, 0, 10000, new decimal[] { 0.0m }, 6, false, null, false));

            keysList.Add(new ParticalKey("EmissionHeight", "Emitter", null, "feet", 1, 0, 10000, new decimal[] { 0.0m }, 6, false, null, false));

            //x, y, z. 
            keysList.Add(new ParticalKey("EmissionStartJitter", "Emitter", null, "XYZ rand/unit", 3, 0, 10000, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, false, null, false));

            //Keetsie request this on 6/21/11 “MoveScale” particle attribute should be under the Emitter tab, not the Particle tab.
            keysList.Add(new ParticalKey("MoveScale", "Emitter", null, "", 1, -1000, 1000, new decimal[] { 1.0m },
                6, false, null, false));


            //#Motion Keys
            /************************* Motion Keys*******************************************/
            keysList.Add(new ParticalKey("InitialVelocity", "Motion", null, "feet/sec^2", 3, -1000000, 1000000, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, false, null, false));

            keysList.Add(new ParticalKey("InitialVelocityJitter", "Motion", null, "rand +/- iniVelocity", 3, -1000000, 1000000, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, false, null, false));

            keysList.Add(new ParticalKey("VelocityJitter", "Motion", null, "rand +/- Velocity", 3, -1, 1, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, false, null, false));

            keysList.Add(new ParticalKey("Magnetism", "Motion", null, "unit", 1, -1000000, 1000000, new decimal[] { 0.0m },
                6, false, null, false));

            //if WorldOrLocalPosition = local means local gravity too
            keysList.Add(new ParticalKey("Gravity", "Motion", null, "unit/sec^2", 1, -1000000, 1000000, new decimal[] { 1.0m },
                6, false, null, false));

            keysList.Add(new ParticalKey("Drag", "Motion", null, "-Speed/frame", 1, 0, 1, new decimal[] { 0.0m },
                6, false, null, false));


            //range (-127 to 127), Positive spins cw, negative, ccw. Disabled if streakorient is used
            keysList.Add(new ParticalKey("Spin", "Motion", null, "degrees", 1, -127, 127, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("SpinJitter", "Motion", null, "", 1, 0, 1000, new decimal[] { 0.0m },
                0, false, null, false));


            /************************* Partical Keys*******************************************/
            keysList.Add(new ParticalKey("IgnoreFxTint", "Particle", null, "flag", 1, 0, 1, new decimal[] { 1.0m },
                0, false, null, false));
            //Flags IgnoreFxTint
            keysList.Add(new ParticalKey("AlwaysDraw", "Particle", null, "flag", 1, 0, 1, new decimal[] { 1.0m },
                0, false, null, false));
            //0 = World, 1 = Local
            keysList.Add(new ParticalKey("WorldOrLocalPosition", "Particle", null, "system", 0, 0, 1, new decimal[] { 0.0m },
                0, false, new string[] { "0 (World)", "1 (Local)" }, false));

            //0 = Front, 1 = Local
            keysList.Add(new ParticalKey("FrontOrLocalFacing", "Particle", null, "", 0, 0, 1, new decimal[] { 0.0m },
                0, false, new string[] { "0 (Front)", "1 (Local)" }, false));

            //0.0 ignore emitter movement, 1.0 copy 100% emitter movement
            keysList.Add(new ParticalKey("Stickiness", "Particle", null, "", 1, 0, 1, new decimal[] { 0.0m },
                6, false, null, false));

            keysList.Add(new ParticalKey("TightenUp", "Particle", null, "<-> Camera", 1, -1000, 1000, new decimal[] { 0.0m },
                6, false, null, false));

            //#Particle Streaking

            //0 = NOSTREAK, 1 = VELOCITY direction, 2 = GRAVITY upwards streak, 3 = ORIGIN, 4 = MAGNET, 5 = OTHER, 7 = CHAIN, 8 = VPARENT
            keysList.Add(new ParticalKey("StreakType", "Particle", null, "", 0, 0, 7, new decimal[] { 0.0m },
                0, false, new string[] { "0 (NOSTREAK)", "1 (VELOCITY)", "2 (GRAVITY)", "3 (ORIGIN)", "4 (MAGNET)", "5 (OTHER)", "6 (NOSTREAK)", "7 (CHAIN)", "8 (VPARENT)" }, false));


            keysList.Add(new ParticalKey("StreakScale", "Particle", null, "unit", 1, 0, 1000, new decimal[] { 1.0m },
                0, false, null, false));

            //1 = Orient (front facing), 0 = No Orient
            keysList.Add(new ParticalKey("StreakOrient", "Particle", null, "", 0, 0, 1, new decimal[] { 1.0m },
                0, false, new string [] {"0 (No Orient)","1 (Orient)"}, false));

            //0 = source direction,   1 = destination direction
            keysList.Add(new ParticalKey("StreakDirection", "Particle", null, "", 0, 0, 1, new decimal[] { 1.0m },
                0, false, new string[]{"0 (source)", "1 (destination)"}, false));

            //need to uset TimeToFull
            keysList.Add(new ParticalKey("KickStart", "Particle", null, "need TimeToFull", 1, 0, 1000, new decimal[] { 1.0m },
                0, false, null, false));

            //Default is 10
            keysList.Add(new ParticalKey("TimeToFull", "Particle", null, "", 1, 0, 10000, new decimal[] { 10.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("SortBias", "Particle", null, "", 1, 0, 100, new decimal[] { 0.0m },
                6, false, null, false));

            keysList.Add(new ParticalKey("DieLikeThis", "Particle", null, "", 0, 0, 1, new decimal[] { 0.0m },
                0, false, partListWp, false));

            //0 = keep partical age, 1 = reset all particles age to zero
            keysList.Add(new ParticalKey("DeathAgeToZero", "Particle", null, "", 0, 0, 1, new decimal[] { 0.0m },
                0, false, new string [] {"0 (keep age)","1 (reset age)"}, false));


            //#Particle Size
            keysList.Add(new ParticalKey("StartSize", "Particle", null, "", 1, -10000, 10000, new decimal[] { 5.0m },
                6, false, null, false));

            keysList.Add(new ParticalKey("StartSizeJitter", "Particle", null, "", 1, 0, 10000, new decimal[] { 0.0m },
                6, false, null, false));

            keysList.Add(new ParticalKey("ExpandRate", "Particle", null, "", 1, -1000, 1000, new decimal[] { 1.0m },
                6, false, null, false));

            //0 = Expand Forever. Ignore EndSize, 1 = Expand and Stop at EndSize, 2 = Ping Pong between startSize and EndSize
            keysList.Add(new ParticalKey("ExpandType", "Particle", null, "", 0, 0, 2, new decimal[] { 0.0m },
                0, false, new string[] { "0 (Expand Forever)", "1 (Expand to EndSize)", "2 (Ping Pong)" }, false));


            //Not used if ExpandRate or ExpandType is 0. 
            keysList.Add(new ParticalKey("EndSize", "Particle", null, "pixle", 1, -10000, 10000, new decimal[] { 1.0m },
                6, false, null, false));

            //#Particle Rotation

            //Range is 0 to 128.
            //0 = straight up and down, 128 = random orientation
            keysList.Add(new ParticalKey("OrientationJitter", "Particle", null, "", 1, 0, 128, new decimal[] { 0.0m },
                0, false, null, false));

            /************************* Texture/Color Keys*******************************************/
            //(string, or integer 0)
            //Normal ,Additive, Subtractive, PreMultipliedAlpha, Multiply, SubtractiveInverse, 
            keysList.Add(new ParticalKey("Blend_mode", "Texture/Color", null, "", 0, 0, 6, new decimal[] { 0.0m },
                0, false, new string[] { "Normal", "Additive", "Subtractive", "PreMultipliedAlpha", "Multiply", "SubtractiveInverse" }, false));

            //#Texture 1 name
            keysList.Add(new ParticalKey("TextureName", "Texture/Color", "TextureName", "", 0, 0, 1, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("TexScroll1", "Texture/Color", "TextureName", "XYZ/frame", 3, -10000, 10000, new decimal[] { 0.0m, 0.0m, 0.0m },
               6, false, null, false));

            keysList.Add(new ParticalKey("TexScrollJitter1", "Texture/Color", "TextureName", "XYZ/frame", 3, -10000, 10000, new decimal[] { 0.0m },
                6, false, null, false));

            //if  <=2 frames: 2x1 <= 4 frames: 2x2 <= 8 frames 4x2 <=16 frames 4x4 <=32 frames: 8x4
            keysList.Add(new ParticalKey("AnimFrames1", "Texture/Color", "TextureName", "framesCount", 1, 0, 64, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("AnimPace1", "Texture/Color", "TextureName", "num.frames/frame", 1, 0, 1000, new decimal[] { 0.0m },
                6, false, null, false));

            //0 = loop, 1 = stop at end, 2 = ping pong 
            keysList.Add(new ParticalKey("AnimType1", "Texture/Color", "TextureName", "", 0, 0, 255, new decimal[] { 0.0m },
                0, false, new string[]{"0 (loop)", "1 (playOnce)", "2 (ping pong)"}, false));

            //Name of the second texture (default white)
            keysList.Add(new ParticalKey("TextureName2", "Texture/Color", "TextureName2", "", 0, 0, 255, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("TexScroll2", "Texture/Color", "TextureName2", "XYZ/frame", 3, -10000, 10000, new decimal[] { 0.0m, 0.0m, 0.0m },
               6, false, null, false));

            keysList.Add(new ParticalKey("TexScrollJitter2", "Texture/Color", "TextureName2", "XYZ/frame", 3, -10000, 10000, new decimal[] { 0.0m },
                6, false, null, false));

            //if  <=2 frames: 2x1 <= 4 frames: 2x2 <= 8 frames 4x2 <=16 frames 4x4 <=32 frames: 8x4
            keysList.Add(new ParticalKey("AnimFrames2", "Texture/Color", "TextureName2", "framesCount", 1, 0, 64, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("AnimPace2", "Texture/Color", "TextureName2", "num.frames/frame", 1, 0, 1000, new decimal[] { 0.0m },
                6, false, null, false));

            //0 = loop, 1 = stop at end, 2 = ping pong 
            keysList.Add(new ParticalKey("AnimType2", "Texture/Color", "TextureName2", "", 0, 0, 255, new decimal[] { 0.0m },
                0, false, new string[] { "0 (loop)", "1 (playOnce)", "2 (ping pong)" }, false));

            //Particle Alpha
            keysList.Add(new ParticalKey("Alpha", "Texture/Color", "Alpha", "", 1, 0, 255, new decimal[] { 255.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("FadeInBy", "Texture/Color", "Alpha", "frame", 1, 0, 255, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("FadeOutStart", "Texture/Color", "Alpha", "frame", 1, 0, 10000, new decimal[] { 30.0m },
                0, false, null, false));

            //warrning if FadeInBy <= FadeOutStart <= FadeOutBy.
            keysList.Add(new ParticalKey("FadeOutBy", "Texture/Color", "Alpha", "frame", 1, 0, 10000, new decimal[] { 30.0m },
                0, false, null, false));

            //Particle Color
            //If ByTime1 is 0, this will always be the particle's color.
            keysList.Add(new ParticalKey("StartColor", "Texture/Color", "StartColor", "RGB", 3, 0, 255, new decimal[] { 0.0m, 0.0m, 0.0m },
                0, true, null, false));

            //0 = Stop at the last color,  1 = Loop back to the beginning
            keysList.Add(new ParticalKey("ColorChangeType", "Texture/Color", "StartColor", "", 0, 0, 1, new decimal[] { 0.0m },
                0, false, new string[]{"0 (Stop@LastColor)","1 (LoopBackToBeginning)"}, false));

            //(Range 0-100)
            keysList.Add(new ParticalKey("PrimaryTint", "Texture/Color", "StartColor", "", 1, 0, 100, new decimal[] { 0.0m },
                0, false, null, false));

            //(Range 0-100)
            keysList.Add(new ParticalKey("SecondaryTint", "Texture/Color", "StartColor", "", 1, 0, 100, new decimal[] { 0.0m },
                0, false, null, false));

            //ByTime/BeColor make sure that you have a ByTime defined that matches or exceeds the FadeOutTime.
            keysList.Add(new ParticalKey("ByTime1", "Texture/Color", "ByTime1", "frames", 1, 0, 1000000, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("BeColor1", "Texture/Color", "ByTime1", "RGB", 3, 0, 255, new decimal[] { 0.0m, 0.0m, 0.0m },
                0, true, null, false));

            keysList.Add(new ParticalKey("PrimaryTint1", "Texture/Color", "ByTime1", "", 1, 0, 255, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("SecondaryTint1", "Texture/Color", "ByTime1", "", 1, 0, 255, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("ByTime2", "Texture/Color", "ByTime2", "frames", 1, 0, 1000000, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("BeColor2", "Texture/Color", "ByTime2", "RGB", 3, 0, 255, new decimal[] { 0.0m, 0.0m, 0.0m },
                0, true, null, false));

            keysList.Add(new ParticalKey("PrimaryTint2", "Texture/Color", "ByTime2", "", 1, 0, 255, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("SecondaryTint2", "Texture/Color", "ByTime2", "", 1, 0, 255, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("ByTime3", "Texture/Color", "ByTime3", "frames", 1, 0, 1000000, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("BeColor3", "Texture/Color", "ByTime3", "RGB", 3, 0, 255, new decimal[] { 0.0m, 0.0m, 0.0m },
                0, true, null, false));

            keysList.Add(new ParticalKey("PrimaryTint3", "Texture/Color", "ByTime3", "", 1, 0, 255, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("SecondaryTint3", "Texture/Color", "ByTime3", "", 1, 0, 255, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("ByTime4", "Texture/Color", "ByTime4", "frames", 1, 0, 1000000, new decimal[] { 0.0m },
                 0, false, null, false));

            keysList.Add(new ParticalKey("BeColor4", "Texture/Color", "ByTime4", "RGB", 3, 0, 255, new decimal[] { 0.0m, 0.0m, 0.0m },
                0, true, null, false));

            keysList.Add(new ParticalKey("PrimaryTint4", "Texture/Color", "ByTime4", "", 1, 0, 255, new decimal[] { 0.0m },
                0, false, null, false));

            keysList.Add(new ParticalKey("SecondaryTint4", "Texture/Color", "ByTime4", "", 1, 0, 255, new decimal[] { 0.0m },
                0, false, null, false));

            //positive or negative
            keysList.Add(new ParticalKey("ColorOffset", "Texture/Color", "ColorOffset", "+/-RGB", 3, -255, 255, new decimal[] { 0.0m, 0.0m, 0.0m },
                0, false, null, false));

            //Randomly calculated once per particle system, but not per-particle.
            keysList.Add(new ParticalKey("ColorOffsetJitter", "Texture/Color", "ColorOffset", "rand/partSystem", 3, 0, 255, new decimal[] { 0.0m, 0.0m, 0.0m },
                0, false, null, false));

        }
    }
}
