using System;
using System.Collections;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.fxEditor
{
    class BehaviorKeys
    {
        public ArrayList keysList;
        private string gameBranch;

        public BehaviorKeys(string game_branch)
        {
            gameBranch = game_branch;

            keysList = new ArrayList();

            populateKeys();
        }
        private void populateKeys()
        {

            string[] animList = System.IO.Directory.GetFiles(@"C:\" + gameBranch + @"\data\player_library\animations\tscroll", "*.anim");
            
            string[] animListWxt = new string[animList.Length];

            int a = 0;

            foreach (string str in animList)
            {
                animListWxt[a++] = System.IO.Path.GetFileNameWithoutExtension(str);
            }

            //behaviorKey(name,group,subGroup,label,numFields,min,max,default[],numDecimal,isOverridable, isColor)
            
            //#Basic Behavior Keys
            //suffix m for decimal 0.0m like suffix f for float 0.0f
            //Drag 0.0
            keysList.Add(new BehaviorKey("Drag", "Basic", null, "-Velocity/sec", 1, 0, 1, new decimal[] { 0.0m }, 6, true, false, null, false));
                        
            //Gravity 0.04
            keysList.Add(new BehaviorKey("Gravity", "Basic", null, "unit/time^2", 1, -1000, 1000, new decimal[] { 0.04m },
                6, true, false, null, false));
            //InitialVelocity 0.0 0.0 0.0
            keysList.Add(new BehaviorKey("InitialVelocity", "Basic", null, "feet/sec", 3, -100000, 100000, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, true, false, null, false));
            //InitialVelocityJitter 0.0 0.0 0.0
            keysList.Add(new BehaviorKey("InitialVelocityJitter", "Basic", null, "Rand +/- Velocity", 3, 0, 100000, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, true, false, null, false));
            //LifeSpan -1
            keysList.Add(new BehaviorKey("LifeSpan", "Basic", null, "frames", 1, -1, 100000, new decimal[] { -1.0m }, 6, true, false, null, false));

            //PositionOffset 0.0 0.0 0.0
            keysList.Add(new BehaviorKey("PositionOffset", "Basic", null, "feet", 3, -10000, 10000, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, true, false, null, false));
            //PyrRotate 0.0 0.0 0.0
            keysList.Add(new BehaviorKey("PyrRotate", "Basic", null, "degrees", 3, -180, 180, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, true, false, null, false));
            //PyrRotateJitter 0.0 0.0 0.
            keysList.Add(new BehaviorKey("PyrRotateJitter", "Basic", null, "Rand +/- pyrRotate", 3, -360, 360, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, true, false, null, false));
            //Scale 0.5 0.5 0.5
            keysList.Add(new BehaviorKey("Scale", "Basic", null, "unit", 3, 0, 100000, new decimal[] { 1.0m, 1.0m, 1.0m },
                6, true, false, null, false));
            //ScaleRate 1.0 1.0 1.0 #no bhvrOverride
            keysList.Add(new BehaviorKey("ScaleRate", "Basic", null, "unit/frame", 3, 0, 100000, new decimal[] { 1.0m, 1.0m, 1.0m },
                6, false, false, null, false));

            //EndScale 1.0 1.0 1.0  #no bhvrOverride >> Leo requested bhvrOverride for endscale 06/28/11
            keysList.Add(new BehaviorKey("EndScale", "Basic", null, "unit", 3, 0, 1000, new decimal[] { 1.0m, 1.0m, 1.0m },
                6, true, false, null, false));

            //Spin 0.0 0.0 0.0
            keysList.Add(new BehaviorKey("Spin", "Basic", null, "degrees/frame", 3, -360, 360, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, true, false, null, false));
            //SpinJitter 0.0 0.0 0.00
            keysList.Add(new BehaviorKey("SpinJitter", "Basic", null, "Rand +/- Spin", 3, -360, 360, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, true, false, null, false));
            //StartJitter 0.0 0.0 0.0
            keysList.Add(new BehaviorKey("StartJitter", "Basic", null, "Rand +/- Pos.Offset", 3, 0, 10000, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, true, false, null, false));
            //TrackMethod 0
            keysList.Add(new BehaviorKey("TrackMethod", "Basic", null, "", 1, 0, 1, new decimal[] { 0.0m },
                0, true, false, null, false));

            //TrackRate 1.0
            keysList.Add(new BehaviorKey("TrackRate", "Basic", null, "feet/sec", 1, -10, 10, new decimal[] {2.0m},
                6, true, false, null, false));

            //#Color and Alpha
            //ColorChangeType 1 (This doesn't appear to work anymore)
            keysList.Add(new BehaviorKey("TintGeom", "Color/Alpha", null, "", 1, 0, 1, new decimal[] { 0.0m },
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("Alpha", "Color/Alpha", null, "units", 1, 0, 255, new decimal[] {0.0m},
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("FadeInLength", "Color/Alpha", null, "frames", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("FadeOutLength", "Color/Alpha", null, "frames", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false)); 

            keysList.Add(new BehaviorKey("inheritGroupTint", "Color/Alpha", null, "", 0, 0, 0, new decimal[] { 0.0m },
                0, false, false, null, false));

            keysList.Add(new BehaviorKey("BeColor1", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));
            keysList.Add(new BehaviorKey("ByTime1", "Color/Alpha", null, "frame#", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("PrimaryTint", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("SecondaryTint", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("PrimaryTint1", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("SecondaryTint1", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("BeColor2", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));
            keysList.Add(new BehaviorKey("ByTime2", "Color/Alpha", null, "frame#", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("PrimaryTint2", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("SecondaryTint2", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("BeColor3", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));
            keysList.Add(new BehaviorKey("ByTime3", "Color/Alpha", null, "frame#", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("PrimaryTint3", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("SecondaryTint3", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("BeColor4", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));
            keysList.Add(new BehaviorKey("ByTime4", "Color/Alpha", null, "frame#", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("PrimaryTint4", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("SecondaryTint4", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("BeColor5", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));
            keysList.Add(new BehaviorKey("ByTime5", "Color/Alpha", null, "frame#", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("PrimaryTint5", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("SecondaryTint5", "Color/Alpha", null, "RGB%", 1, 0, 100, new decimal[] { 0.0m },
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("StartColor", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));

            //#BhvrOverride feature of an FX file event uses the old color value system.
            //#     Rgb0 255 255 255
            keysList.Add(new BehaviorKey("Rgb0", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));
            //#     Rgb0Time 15
            keysList.Add(new BehaviorKey("Rgb0Time", "Color/Alpha", null, "frame#", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));
            //#     Rgb0Next 1
            keysList.Add(new BehaviorKey("Rgb0Next", "Color/Alpha", null, "", 1, 0, 4, new decimal[] { 1.0m },
                0, true, false, null, false));

            //#     Rgb1 0 0 0
            keysList.Add(new BehaviorKey("Rgb1", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));
            //#     Rgb1Time 30
            keysList.Add(new BehaviorKey("Rgb1Time", "Color/Alpha", null, "frame#", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));
            //#     Rgb1Next 2
            keysList.Add(new BehaviorKey("Rgb1Next", "Color/Alpha", null, "", 1, 0, 4, new decimal[] { 1.0m },
                0, true, false, null, false));

            //#     Rgb2 0 0 255
            keysList.Add(new BehaviorKey("Rgb2", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));
            //#     Rgb2Time 45
            keysList.Add(new BehaviorKey("Rgb2Time", "Color/Alpha", null, "frame#", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));
            //#     Rgb2Next 0
            keysList.Add(new BehaviorKey("Rgb2Next", "Color/Alpha", null, "", 1, 0, 4, new decimal[] { 1.0m },
                0, true, false, null, false));

            //#     Rgb3 255 255 255
            keysList.Add(new BehaviorKey("Rgb3", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));
            //#     Rgb3Time 15
            keysList.Add(new BehaviorKey("Rgb3Time", "Color/Alpha", null, "frame#", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));
            //#     Rgb3Next 1
            keysList.Add(new BehaviorKey("Rgb3Next", "Color/Alpha", null, "", 1, 0, 4, new decimal[] { 1.0m },
                0, true, false, null, false));

            //#     Rgb4 0 0 0
            keysList.Add(new BehaviorKey("Rgb4", "Color/Alpha", null, "RGB", 3, 0, 255, new decimal[] { 255.0m, 255.0m, 255.0m },
                0, true, true, null, false));
            //#     Rgb4Time 30
            keysList.Add(new BehaviorKey("Rgb4Time", "Color/Alpha", null, "frame#", 1, 0, 10000, new decimal[] { 1.0m },
                0, true, false, null, false));
            //#     Rgb4Next 2
            keysList.Add(new BehaviorKey("Rgb4Next", "Color/Alpha", null, "", 1, 0, 4, new decimal[] { 1.0m },
                0, true, false, null, false));
            
            keysList.Add(new BehaviorKey("HueShift", "Color/Alpha", null, "degrees", 1, -180, 180, new decimal[] { 0.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("HueShiftJitter", "Color/Alpha", null, "degrees", 1, -180, 180, new decimal[] { 0.0m },
                0, true, false, null, false));

            /************************* Physics values *******************************************/
            keysList.Add(new BehaviorKey("physics", "Physics", null, "", 1, 0, 1, new decimal[] { 0.0m },
                0, true, false, null, false));

            keysList.Add(new BehaviorKey("physDensity", "Physics", null, "units", 1, 0, 1000, new decimal[] { 1.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physDebris", "Physics", null, "", 1, 1, 3, new decimal[] { 1.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("physDFriction", "Physics", null, "units", 1, 0, 1, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physGravity", "Physics", null, "unit/time^2", 1, -1000, 1000, new decimal[] { 1.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("PhysKillBelowSpeed", "Physics", null, "unit/time^2", 1, 0, 100000, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physRadius", "Physics", null, "unit", 1, 0, 1, new decimal[] { 0.3m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physRestitution", "Physics", null, "*velocity", 1, 0, 1, new decimal[] { 1.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physScale", "Physics", null, "XYZ units", 3, 0, 10000, new decimal[] { 1.0m , 1.0m, 1.0m},
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physSFriction", "Physics", null, "-speed", 1, 0, 1, new decimal[] { 0.0m },
                6, true, false, null, false));
             
            //#Force Values
            keysList.Add(new BehaviorKey("physForceCentripetal", "Physics", null, "unit", 1, 0, 100, new decimal[] { 2.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physForcePower", "Physics", null, "unit", 1, 0, 10000000, new decimal[] { 100.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physForcePowerJitter", "Physics", null, "rand/object", 1, 0, 1000, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physForceRadius", "Physics", null, "feet", 1, 0, 100000, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physForceType", "Physics", null, "", 0, 0, 1, new decimal[] { 0.0m },
                0, true, false, new string[] {"None","Out","In","CWSwirl","CCWSwirl","Up","Forward","Side","Drag"},false));
            //#Jointed Object Values
            keysList.Add(new BehaviorKey("physJointAnchor", "Physics", null, "XYZ units", 3, -1000, 1000, new decimal[] { 0.0m, 0.0m, 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physJointAngLimit", "Physics", null, "degrees", 1, -180, 180, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physJointAngSpring", "Physics", null, "unit", 1, 0, 1000, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physJointAngSpringDamp", "Physics", null, "unit", 1, 0, 1000, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physJointBreakForce", "Physics", null, "unit", 1, 0, 1000000, new decimal[] { 100000.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physJointBreakTorque", "Physics", null, "unit", 1, 0, 1000, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physJointCollidesWorld", "Physics", null, "", 0, 0, 1, new decimal[] { 0.0m },
                6, false, false, null, false));

            //num field is 4 to turn off numupdwns without getting is flag
            keysList.Add(new BehaviorKey("physJointDOFs", "Physics", null, "", 4, 0, 1, new decimal[] { 0.0m },
                0, true, false, null, true)); 
            keysList.Add(new BehaviorKey("physJointDrag", "Physics", null, "unit", 1, 0, 1000, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physJointLinLimit", "Physics", null, "+/- feet", 1, -1000, 1000, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physJointLinSpring", "Physics", null, "unit", 1, 0, 100000, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("physJointLinSpringDamp", "Physics", null, "unit", 1, 0, 100000, new decimal[] { 0.0m },
                6, true, false, null, false));
            
            //#Camera Shake
            keysList.Add(new BehaviorKey("Shake", "Shake/Blur/Light", null, "unit", 1, 0, 100, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("ShakeFallOff", "Shake/Blur/Light", null, "*shake/frame", 1, 0, 1, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("ShakeRadius", "Shake/Blur/Light", null, "feet", 1, 0, 10000, new decimal[] { 0.0m },
                6, true, false, null, false));
             
            //#Screen Blur
            keysList.Add(new BehaviorKey("Blur", "Shake/Blur/Light", null, "", 1, 0, 10, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("BlurFalloff", "Shake/Blur/Light", null, "*blur/tick", 1, 0, 1, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("BlurRadius", "Shake/Blur/Light", null, "feet", 1, 0, 10000, new decimal[] { 0.0m },
                6, true, false, null, false));
            //#light Puls
            keysList.Add(new BehaviorKey("PulseBrightness", "Shake/Blur/Light", null, "*brightness", 1, 0, 10000, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("PulsePeakTime", "Shake/Blur/Light", null, "ticks", 1, 0, 10000, new decimal[] { 0.0m },
                0, true, false, null, false));
            keysList.Add(new BehaviorKey("PulseClamp", "Shake/Blur/Light", null, "", 1, 0, 1, new decimal[] { 0.0m },
                0, true, false, null, false));
            
            //#Animation
            keysList.Add(new BehaviorKey("AnimScale", "Animation/Splat", null, "*plybk Speed", 1, 0, 1000, new decimal[] { 1.0m },
                0, true, false, null, false));
            //keysList.Add(new behaviorKey("StAnim  0.600000 1.000000 light_pulse2 FRAMESNAP #Means play the light_pulse2 animation (in the animation folder) at 60%% speed, not zoomed at all, and don't interpolate between frames.

            keysList.Add(new BehaviorKey("StAnim", "Animation/Splat", null, "", 5, 0, 1, new decimal[] { 0.0m, 0.0m },
                6, false, false, animListWxt, false));              
            
            keysList.Add(new BehaviorKey("Stretch", "Animation/Splat", null, "", 1, 1, 2, new decimal[] { 1.0m },
                0, true, false, null, false));
            //#Splat
            //"Splat	ImpactCracks.tga" should be at the fx interface
            keysList.Add(new BehaviorKey("SplatFadeCenter", "Animation/Splat", null, "", 1, 0, 1, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("SplatFalloffType", "Animation/Splat", null, "", 0, 0, 1, new decimal[] { 0.0m },
                6, true, false, new string[] { "NONE", "UP", "DOWN", "BOTH" }, false));
            keysList.Add(new BehaviorKey("SplatFlags", "Animation/Splat", null, "", 0, 0, 1, new decimal[] { 0.0m },
                6, true, false, new string[] { "ADDITIVE", "DIFFUSE" }, false));
            keysList.Add(new BehaviorKey("SplatNormalFade", "Animation/Splat", null, "", 1, 0, 1, new decimal[] { 0.0m },
                6, true, false, null, false));
            keysList.Add(new BehaviorKey("SplatSetBack", "Animation/Splat", null, "", 1, 0, 1, new decimal[] { 0.0m },
                6, true, false, null, false));
            
        }
    }
}
