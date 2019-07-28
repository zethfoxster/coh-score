using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    public partial class FilterActive : Form
    {
        public Dictionary<string, bool> eventDic;
        
        public Dictionary<string, bool> bhvrDic;
        
        public Dictionary<string, bool> partDic;

        private ArrayList eventsSubgroups;

        private ArrayList partsSubgroups;

        private ArrayList bhvrSubgroups;

        public FilterActive(Dictionary<string,bool> eventd, Dictionary<string,bool> bhvrd, Dictionary<string,bool> partd)
        {
            //Lamdba expressions "=>" can be expressed as "goes to," "k => k.key", "k => k.Value" style lines. 
            //meaning each item in the dictionary goes to itself.
            eventDic = eventd.ToDictionary(k=>k.Key, k=>k.Value);
            
            bhvrDic = bhvrd.ToDictionary(k => k.Key, k => k.Value);
            
            partDic = partd.ToDictionary(k => k.Key, k => k.Value);
            
            InitializeComponent();
            
            this.Event_tabPage.SuspendLayout();
            orgEvent();
            buildSubgroupsCkBx("Event", eventDic, true, eventsSubgroups, this.Event_tabPage);

            orgPart();
            buildSubgroupsCkBx("Part", partDic, true, partsSubgroups, this.part_tabPage);

            orgBhvr();
            buildSubgroupsCkBx("Bhvr", bhvrDic, true, bhvrSubgroups, this.bhvr_tabPage);
            
            this.Event_tabPage.ResumeLayout(false);
        }

        public void buildBhvrPartCkBx(string tabPageTxt, Dictionary<string, bool> tags, bool isColumnOrder)
        {
            int i = 1;
            
            //sort dic by key ascending
            foreach (KeyValuePair<string, bool> kvp in tags.OrderBy(a => a.Key, new CompareStrings()))
            {
                string tag = kvp.Key.Substring(0,kvp.Key.IndexOf("__"));

                string grp = kvp.Key.Substring(kvp.Key.IndexOf("__")).Replace("__","");

                CheckBox checkBox = new System.Windows.Forms.CheckBox();

                checkBox.AutoSize = true;

                checkBox.Tag = kvp.Key;

                checkBox.Name = tag + "_CkBx";

                checkBox.TabIndex = i;

                checkBox.Text = tag;

                checkBox.Checked = kvp.Value;

                checkBox.CheckedChanged += new EventHandler(checkBox_CheckedChanged);

                checkBox.UseVisualStyleBackColor = true;

                checkBox.BackColor = checkBox.Checked ? Color.Yellow : this.part_tabPage.BackColor;

                switch (tabPageTxt)
                {
                    case "Bhvr":
                        addToBhvrFilter(grp, checkBox);
                        break;

                    case "Part":
                        addToPartFilter(grp, checkBox);
                        break;
                }
                i++;
            }
            foreach (TabPage tpage in filter_tabControl.TabPages)
            {
                if (tpage == bhvr_tabPage)
                {
                    TabControl tbctl = (TabControl)tpage.Controls[0];
                    foreach (TabPage tp in tbctl.TabPages)
                    {
                        fixCkbxOrder(tp);
                    }
                }
            }
        }
        
        private void orgBhvr()
        {
            bhvrSubgroups = new ArrayList();
            SubGroups alpha = new SubGroups("ALPHA", new string[]{"Alpha", "FadeInLength","FadeOutLength"}); 
            SubGroups motion = new SubGroups("MOTION", new string[]{"LifeSpan", "InitialVelocity","InitialVelocityJitter","Drag","Gravity","Spin","SpinJitter","TrackMethod","TrackRate"});
            SubGroups pos = new SubGroups("POSITION", new string[]{"PositionOffset","StartJitter","PyrRotate","PyrRotateJitter"});
            SubGroups scale = new SubGroups("SCALE",  new string[]{"Scale","ScaleRate","EndScale"});
            SubGroups anim = new SubGroups("ANIMATION", new string[]{"AnimScale","StAnim","Stretch"});
            SubGroups cam = new SubGroups("CAMERA EFFECTS", new string[]{"Blur","BlurFalloff","BlurRadius","","Shake","ShakeFallOff","ShakeRadius"});  
            SubGroups color = new SubGroups("COLOR", new string[]{"TintGeom","", "HueShift","HueShiftJitter","inheritGroupTint","","StartColor","PrimaryTint","SecondaryTint","",
            "BeColor1", "ByTime1","PrimaryTint1","SecondaryTint1", "","BeColor2", "ByTime2","PrimaryTint2","SecondaryTint2"});

            SubGroups colorCont = new SubGroups("COLOR CONT.",  new string[]{"BeColor3", "ByTime3","PrimaryTint3","SecondaryTint3","",
                "BeColor4",  "ByTime4","PrimaryTint4","SecondaryTint4","","BeColor5", "ByTime5","PrimaryTint5","SecondaryTint5"});

            SubGroups colorOver = new SubGroups("COLOR-OVERRIDE", new string[]{"Rgb0","Rgb0Next","Rgb0Time","","Rgb1","Rgb1Next","Rgb1Time","","Rgb2", "Rgb2Next","Rgb2Time"});
            SubGroups colorOverCont = new SubGroups("COLOR-OVERRIDE CONT.", new string[]{"Rgb3","Rgb3Next","Rgb3Time","","Rgb4","Rgb4Next","Rgb4Time","",});
            SubGroups light = new SubGroups("LIGHT PULSE", new string[]{"PulseBrightness","PulsePeakTime","PulseClamp"});
            SubGroups phys = new SubGroups("PHYSICS",new string[]{"physics","physGravity","physRadius","physRestitution","physSFriction","physDFriction","PhysKillBelowSpeed"});
            SubGroups physDebris = new SubGroups("PHYSICS-DEBRIS",new string[]{ "physDebris","physDensity","physScale"});
            SubGroups physForce = new SubGroups("PHYSICS-FORCE" ,new string[]{"physForceType","physForceRadius","physForcePower","physForcePowerJitter","physForceCentripetal"});                                               
            SubGroups physJoint = new SubGroups("PHYSICS-JOINTED OBJECTS",new string[]{"physJointAnchor","physJointAngLimit","physJointAngSpring","physJointAngSpringDamp",
            "physJointBreakForce","physJointBreakTorque","physJointCollidesWorld","physJointDOFs","physJointDrag","physJointLinLimit","physJointLinSpring","physJointLinSpringDamp"});

            SubGroups splat = new SubGroups("SPLAT",  new string[]{"SplatFadeCenter","SplatFalloffType","SplatFlags","SplatNormalFade","SplatSetBack"});

            bhvrSubgroups.Add(alpha);
            bhvrSubgroups.Add(motion);
            bhvrSubgroups.Add(pos);
            bhvrSubgroups.Add(scale);
            bhvrSubgroups.Add(anim);
            bhvrSubgroups.Add(cam);
            bhvrSubgroups.Add(color);
            bhvrSubgroups.Add(colorCont);
            bhvrSubgroups.Add(colorOver);
            bhvrSubgroups.Add(colorOverCont);
            bhvrSubgroups.Add(light);
            bhvrSubgroups.Add(phys);
            bhvrSubgroups.Add(physDebris);
            bhvrSubgroups.Add(physForce);
            bhvrSubgroups.Add(physJoint);
            bhvrSubgroups.Add(splat);
        }

        private void orgPart()
        {
            partsSubgroups = new ArrayList();
            SubGroups emitter = new SubGroups("EMITTER", new string[] {"KickStart", "TimeToFull", "", "Burst", "NewPerFrame", "MoveScale", "", "BurbleAmplitude"
            , "BurbleFrequency", "BurbleThreshold", "", "EmissionType", "EmissionRadius", "EmissionHeight", "EmissionLifeSpan"
            , "", "TightenUp", "", "WorldOrLocalPosition","DieLikeThis","DeathAgeToZero"});

            SubGroups motion = new SubGroups("MOTION", new string[] { "InitialVelocity", "InitialVelocityJitter", "VelocityJitter"
            , "", "Gravity", "Drag", "Magnetism", "Stickiness", "", "OrientationJitter", "Spin", "SpinJitter"});

            SubGroups particle = new SubGroups("PARTICLE", new string[] { "FrontOrLocalFacing", "SortBias", "", "StartSize", "ExpandRate"
            , "ExpandType", "EndSize", "", "StreakType", "StreakScale", "StreakOrient", "StreakDirection"});

            SubGroups textures = new SubGroups("TEXTURES", new string[] { "Blend_mode", "", "TextureName", "TexScroll1", "TexScrollJitter1"
            , "AnimFrames1", "AnimPace1", "AnimType1", "", "TextureName2", "TexScroll2", "TexScrollJitter2", "AnimFrames2", "AnimPace2"
            , "AnimType2"});

            SubGroups alpha = new SubGroups("ALPHA", new string[] { "Alpha", "FadeInBy", "FadeOutStart", "FadeOutBy" });

            SubGroups flags = new SubGroups("FLAGS", new string[] { "IgnoreFxTint", "AlwaysDraw"});

            SubGroups color = new SubGroups("COLOR", new string[] { "ColorOffset", "ColorOffsetJitter", "",
            "StartColor", "ColorChangeType", "PrimaryTint", "SecondaryTint", "",
            "ByTime1", "BeColor1", "PrimaryTint1", "SecondaryTint1", "",
            "ByTime2", "BeColor2", "PrimaryTint2", "SecondaryTint2", "",
            "ByTime3", "BeColor3", "PrimaryTint3", "SecondaryTint3", "",
            "ByTime4", "BeColor4", "PrimaryTint4", "SecondaryTint4"});

            partsSubgroups.Add(emitter);
            partsSubgroups.Add(flags);
            partsSubgroups.Add(motion);
            partsSubgroups.Add(particle);
            partsSubgroups.Add(textures);
            partsSubgroups.Add(alpha);
            partsSubgroups.Add(color);
        }

        private void orgEvent()
        {
            eventsSubgroups = new ArrayList();
            SubGroups standard = new SubGroups("STANDARD",new string []{ "EName", "Type", "At", "Flags", "ChildFx", "LifeSpan", "LifeSpanJitter"});
            SubGroups animated = new SubGroups("ANIMATED ENTITIES", new string[] { "Anim", "AnimPiv", "Cape", "NewState", "SetState"});
            SubGroups animBits = new SubGroups("ANIM BITS",new string[]{"Until","While"});
            SubGroups camera = new SubGroups("CAMERA", new string[] {"CameraSpace" });
            SubGroups createNode = new SubGroups("CREATE NODE", new string[] { "Inherit_Position", "Inherit_Rotation", "Inherit_Scale", "Inherit_SuperRotation", 
                                                                               "Update_Position","Update_Rotation","Update_Scale","Update_SuperRotation"});
            SubGroups geometry = new SubGroups("GEOMETRY", new string[] { "Geom", "AltPiv"});
            SubGroups goalsMag = new SubGroups("GOALS & MAGNETS", new string[] { "LookAt", "Magnet", "PMagnet", "POther"});
            SubGroups physics = new SubGroups("PHYSICS", new string[] { "CDestroy", "CEvent", "CThresh", "PhysicsOnly", "JDestroy", "JEvent"});
            SubGroups sysReq = new SubGroups("SYSTEM REQUIREMENTS", new string[] { "HardwareOnly", "SoftwareOnly"});
            SubGroups legacy = new SubGroups("LEGACY", new string[] { "AtRayFx", "CRotation", "Lighting", "PowerMax", "PowerMin", "RayLength", "SoundNoRepeat", "WorldGroup"});

            eventsSubgroups.Add(standard);
            eventsSubgroups.Add(animated);
            eventsSubgroups.Add(animBits);
            eventsSubgroups.Add(camera);
            eventsSubgroups.Add(createNode);
            eventsSubgroups.Add(geometry);
            eventsSubgroups.Add(goalsMag);
            eventsSubgroups.Add(physics);
            eventsSubgroups.Add(sysReq);
            eventsSubgroups.Add(legacy);
        }

        private void getEventBreak(string sgName, bool isColumnOrder,ref bool newColumn, ref int y, ref int x, int w,int h)
        {
            switch (sgName)
            {
                case "STANDARD":
                case "CAMERA":
                case "PHYSICS":
                case "LEGACY":
                    if (newColumn)
                    {
                        newColumn = false;
                        if (isColumnOrder)
                        {
                            y = 10;
                            x += w + 25;
                        }
                        else
                        {
                            x = 10;
                            y += h + 10;
                        }
                    }
                    break;

            }
        }

        private void getBhvrBreak(string sgName, bool isColumnOrder, ref bool newColumn, ref int y, ref int x, int w, int h)
        {
            switch (sgName)
            {
                case "ALPHA":
                case "CAMERA EFFECTS":
                case "COLOR CONT.":
                case "COLOR-OVERRIDE CONT.":
                case "PHYSICS-FORCE":
                    if (newColumn)
                    {
                        newColumn = false;
                        if (isColumnOrder)
                        {
                            y = 10;
                            x += w + 25;
                        }
                        else
                        {
                            x = 10;
                            y += h + 10;
                        }
                    }
                    break;

            }
        }

        private void getPartBreak(string sgName, bool isColumnOrder, ref bool newColumn, ref int y, ref int x, int w,int h)
        {
            switch (sgName)
            {
                case "EMITTER":
                case "FLAGS":
                case "MOTION":
                case "PARTICLE":
                case "TEXTURES":
                case "ALPHA":
                case "COLOR":
                    if (newColumn)
                    {
                        newColumn = false;
                        if (isColumnOrder)
                        {
                            y = 10;
                            x += w + 25;
                        }
                        else
                        {
                            x = 10;
                            y += h + 10;
                        }
                    }
                    break;

            }
        }
        
        public void buildSubgroupsCkBx(string tabPageTxt, Dictionary<string, bool> tags, bool isColumnOrder, ArrayList subgroups, TabPage tp)
        {
            int i = 1;
            int x = 10;
            int y = 10;
            int w = 0;

            foreach (SubGroups sg in subgroups)
            {
                w = Math.Max((int)this.CreateGraphics().MeasureString(sg.name, this.Font).Width, w);
                foreach(string str in sg.content)
                    w = Math.Max((int)this.CreateGraphics().MeasureString(str, this.Font).Width, w);
            }

            int itemCount = (Event_tabPage.Width - w) / w;

            if (isColumnOrder)
                itemCount = (Event_tabPage.Height - 17) / 17;

            int h = 15;

            string oldSubGrp = ((SubGroups)eventsSubgroups[0]).name;

            bool newColumn = false;

            foreach (SubGroups sg in subgroups)
            {
                switch (tp.Text)
                {
                    case "Event":
                        getEventBreak(sg.name, isColumnOrder, ref newColumn, ref y, ref x, w, h);
                        break;

                    case "Part":
                        getPartBreak(sg.name, isColumnOrder, ref newColumn, ref y, ref x, w, h);
                        break;

                    case "Bhvr":
                        getBhvrBreak(sg.name, isColumnOrder, ref newColumn, ref y, ref x, w, h);
                        break;
                }

                Label header = new Label();
                header.Location = new System.Drawing.Point(x, y);
                header.Text = sg.name;
                header.Name = sg.name.ToLower() + "_lable";
                header.Font = new Font(this.Font, FontStyle.Bold);
                header.AutoSize = true;

                tp.Controls.Add(header);

                foreach (string key in sg.content)
                {
                    string tag = key;

                    if (key.Length == 0)
                    {
                        if (isColumnOrder)
                            y += h;
                        else
                            x += w + 25;

                        continue;
                    }

                    CheckBox checkBox = new System.Windows.Forms.CheckBox();

                    checkBox.AutoSize = true;

                    if (isColumnOrder)
                        y += checkBox.Height;
                    else
                        x += w + 25;

                    checkBox.Location = new System.Drawing.Point(x, y);

                    checkBox.Name = tag + "_CkBx";

                    checkBox.TabIndex = i;

                    checkBox.Text = tag;

                    checkBox.Checked = tags[key];

                    checkBox.CheckedChanged += new EventHandler(checkBox_CheckedChanged);

                    checkBox.UseVisualStyleBackColor = true;

                    checkBox.BackColor = checkBox.Checked ? Color.Yellow : this.part_tabPage.BackColor;

                    h = checkBox.Height;

                    tp.Controls.Add(checkBox);
                    i++;
                }
                oldSubGrp = sg.name;
                newColumn = true; 
                if (isColumnOrder)
                    y += h+5;
                else
                    x += w + 25;
            }

        }
        
        private void fixCkbxOrder(TabPage tp)
        {
            int x = 10;
            int y = 10;
            int w = 0;
            int i = 1;
            bool isColumnOrder = false;

            foreach (CheckBox checkBox in tp.Controls)
            {
                string cName = checkBox.Name.Replace("_CkBx","");

                w = Math.Max((int)this.CreateGraphics().MeasureString(cName, this.Font).Width, w);
            }

            int baseD = w == 0 ? 1 : w;

            int itemCount = (tp.Width - w) / baseD;

            if (isColumnOrder)
                itemCount = (tp.Height - 40) / 40;

            foreach (CheckBox checkBox in tp.Controls)
            {
                checkBox.Location = new System.Drawing.Point(x, y);

                if (isColumnOrder)
                    y += checkBox.Height + 10;
                else
                    x += w + 25;

                if (i % itemCount == 0)
                {
                    if (isColumnOrder)
                    {
                        y = 10;
                        x += w + 25;
                    }
                    else
                    {
                        x = 10;
                        y += checkBox.Height + 10;
                    }
                }
                i++;
            }
        }

        private void addToBhvrFilter(string grp, CheckBox checkBox)
        {
            switch (grp)
            {
                case "Basic":
                    basic_tabPage.Controls.Add(checkBox);
                    break;

                case "Color/Alpha":
                    colorAlpha_tabPage.Controls.Add(checkBox);
                    break;

                case "Physics":
                    physics_tabPage.Controls.Add(checkBox);
                    break;

                case "Shake/Blur/Light":
                    shakeBlurLight_tabPage.Controls.Add(checkBox);
                    break;

                case "Animation/Splat":
                    animationSplat_tabPage.Controls.Add(checkBox);
                    break;
            }
        }
        
        private void addToPartFilter(string grp, CheckBox checkBox)
        {
            switch (grp)
            {
                case "Emitter":
                    emitter_tabPage.Controls.Add(checkBox);
                    break;

                case "Motion":
                    motion_tabPage.Controls.Add(checkBox);
                    break;

                case "Particle":
                    particle_tabPage.Controls.Add(checkBox);
                    break;

                case "Texture/Color":
                    textureColor_tabPage.Controls.Add(checkBox);
                    break;
            }
        }

        public void buildCkBx(string tabPageTxt, Dictionary<string, bool> tags, bool isColumnOrder)
        {
            int i = 1;
            int x = 10;
            int y = 10;
            int w = 0;

            foreach (KeyValuePair<string, bool> kvp in tags)
            {
                w = Math.Max((int)this.CreateGraphics().MeasureString(kvp.Key, this.Font).Width, w);
            }

            int itemCount = (Event_tabPage.Width - w) / w;

            if (isColumnOrder)
                itemCount = (Event_tabPage.Height - 40) / 40;

            //sort dic by key ascending
            foreach (KeyValuePair<string, bool> kvp in tags.OrderBy(a => a.Key, new CompareStrings()))
            {
                string tag = kvp.Key;

                CheckBox checkBox = new System.Windows.Forms.CheckBox();

                checkBox.AutoSize = true;

                checkBox.Location = new System.Drawing.Point(x, y);

                checkBox.Name = tag + "_CkBx";

                checkBox.TabIndex = i;

                checkBox.Text = tag;

                checkBox.Checked = kvp.Value;

                checkBox.CheckedChanged += new EventHandler(checkBox_CheckedChanged);

                checkBox.UseVisualStyleBackColor = true;

                checkBox.BackColor = checkBox.Checked ? Color.Yellow : this.part_tabPage.BackColor;

                if (isColumnOrder)
                    y += checkBox.Height + 10;
                else
                    x += w + 25;

                if (i % itemCount == 0)
                {
                    if (isColumnOrder)
                    {
                        y = 10;
                        x += w + 25;
                    }
                    else
                    {
                        x = 10;
                        y += checkBox.Height + 10;
                    }
                }

                switch (tabPageTxt)
                {
                    case "Event":
                        this.Event_tabPage.Controls.Add(checkBox);
                        break;

                    case "Bhvr":
                        this.bhvr_tabPage.Controls.Add(checkBox);
                        break;

                    case "Part":
                        this.part_tabPage.Controls.Add(checkBox);
                        break;
                }
                i++;
            }
        }

        private void checkBox_CheckedChanged(object sender, EventArgs e)
        {
            string tabPage = filter_tabControl.SelectedTab.Text;
            
            CheckBox ck = (CheckBox)sender;
            
            switch (tabPage)
            {
                case "Event":
                    eventDic[ck.Text] = ck.Checked;
                    break;

                case "Bhvr":
                    bhvrDic[ck.Text] = ck.Checked;
                    break;

                case "Part":
                    partDic[ck.Text] = ck.Checked;
                    break;
            }

            ck.BackColor = ck.Checked ? Color.Yellow : filter_tabControl.SelectedTab.BackColor;
        }

        public void updateEnabledLists(out Dictionary<string, bool> eventd, out Dictionary<string, bool> bhvrd, out Dictionary<string, bool> partd)
        {
            eventd = eventDic.ToDictionary(k => k.Key, k => k.Value);
           
            bhvrd = bhvrDic.ToDictionary(k => k.Key, k => k.Value);
            
            partd = partDic.ToDictionary(k => k.Key, k => k.Value);
        }

        private void allOff_btn_Click(object sender, EventArgs e)
        {
            setCheckBoxChecked(false);
        }

        private void allOn_btn_Click(object sender, EventArgs e)
        {
            setCheckBoxChecked(true);
        }
        
        private void setCheckBoxChecked(bool isChecked)
        {
            foreach (Control ctl in filter_tabControl.SelectedTab.Controls)
            {
                if (ctl.GetType() == typeof(CheckBox))
                {
                    CheckBox ck = (CheckBox)ctl;
                    ck.Checked = isChecked;
                }
            }
        }
    }

    public class SubGroups
    {
        public string name;
        public string[] content;

        public SubGroups(string sName, string[] list)
        {
            name = sName;
            content = (string[]) list.Clone();
        }
    }

    public class CompareStrings : IComparer<string>
    {
        public int Compare(string s1, string s2)
        {
            return string.Compare(s1, s2, true);
        }
    }

    public class CompareFileNameStr : IComparer<string>
    {
        public int Compare(string s1, string s2)
        {
            return string.Compare(System.IO.Path.GetFileName(s1), System.IO.Path.GetFileName(s2), true);
        }
    }
}
