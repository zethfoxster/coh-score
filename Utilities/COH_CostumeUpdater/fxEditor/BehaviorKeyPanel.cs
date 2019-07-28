using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    public class BehaviorKeyPanel:Panel
    {
        private int fieldsCount;

        private int decimal_places;
        
        private bool isColor;
        
        private bool isFlag;
        
        private bool isCkbx;
        
        private bool isOverridable;
        
        private string bhvrOriginalValue;
        
        private string bhvrOverrideValue;
        
        private decimal[] defaultValues;
        
        private System.Windows.Forms.CheckBox enablePanel_ckbx;

        protected System.Windows.Forms.Label units_label;
        
        protected System.Windows.Forms.NumericUpDown keyVal1_numericUpDown;
        
        protected System.Windows.Forms.NumericUpDown keyVal2_numericUpDown;
        
        protected System.Windows.Forms.NumericUpDown keyVal3_numericUpDown;
        
        protected System.Windows.Forms.CheckBox flag_ckbx;
        
        protected System.Windows.Forms.CheckBox overKeyName_ckbx;
        
        protected System.Windows.Forms.CheckBox rotateX_ckbx;
        
        protected System.Windows.Forms.CheckBox rotateY_ckbx;
        
        protected System.Windows.Forms.CheckBox rotateZ_ckbx;
        
        protected System.Windows.Forms.CheckBox translateX_ckbx;
        
        protected System.Windows.Forms.CheckBox translateY_ckbx;
        
        protected System.Windows.Forms.CheckBox translateZ_ckbx;
        
        protected System.Windows.Forms.Button keyColor_btn;

        protected System.Windows.Forms.ContextMenu  mnuContextMenu;

        protected System.Windows.Forms.MenuItem mnuItemNew;
        
        protected System.Windows.Forms.ComboBox bhvr_combobx;
        
        protected Object ownerPanel;
        
        protected int width;      
        
        protected bool isUsed;
        
        protected bool resetPanel;

        protected bool isCombo;

        public string group;

        public string subGroup;

        public Color mainBackColor;
        
        public bool isDirty;
        
        public bool isLocked;

        public event EventHandler CheckChanged;
        
        public event EventHandler IsDirtyChanged;


        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        public BehaviorKeyPanel(Object ownerPnl, string name, string key_label, int numFields, int min, int max, decimal [] defaultVals, int decimalPlaces,
            bool isOverridableField, bool isColorField, bool isFlagField, object[] comboItems, int pWidth, bool isCkbxFields, string grp, string subGrp)
        {
            isCombo = false;

            group = grp;

            subGroup = subGrp;

            decimal_places = decimalPlaces;

            if (ownerPnl.GetType() == typeof(BehaviorWin))
                ownerPanel = (BehaviorWin)ownerPnl;
            else
                ownerPanel = (ParticalsWin)ownerPnl;

            resetPanel = false;

            isLocked = true;
            
            fieldsCount = numFields;
           
            isColor = isColorField;
            
            isFlag = isFlagField;
            
            isCkbx = isCkbxFields;
            
            isOverridable = isOverridableField;
            
            defaultValues = null; 

            if (defaultVals != null)
            {
                defaultValues = (decimal[])defaultVals.Clone();
            }

            width = pWidth;
            
            InitializeComponent();
            
            this.Name = name + "_BehaviorKeyPanel";
            
            overKeyName_ckbx.Text = "(Ovr)";
            
            enablePanel_ckbx.Text = name;
            
            flag_ckbx.Text = name;
            
            units_label.Text = key_label;

            switch (numFields)
            {
                case 1:
                    keyVal1_numericUpDown.Visible = true;
                    
                    keyVal2_numericUpDown.Visible = false;
                    
                    keyVal3_numericUpDown.Visible = false;
                    
                    break;

                case 2:
                    keyVal1_numericUpDown.Visible = true;
                    
                    keyVal2_numericUpDown.Visible = true;
                    
                    keyVal3_numericUpDown.Visible = false;
                    
                    break;

                case 3:
                    keyVal1_numericUpDown.Visible = true;
                    
                    keyVal2_numericUpDown.Visible = true;
                    
                    keyVal3_numericUpDown.Visible = true;
                    
                    break;

                case 5:
                    keyVal1_numericUpDown.Visible = true;
                    
                    keyVal2_numericUpDown.Visible = true;
                    
                    keyVal3_numericUpDown.Visible = false;
                    
                    units_label.Visible = false;

                    bhvr_combobx.Location = keyVal3_numericUpDown.Location;

                    bhvr_combobx.Size = new System.Drawing.Size(144, 21);

                    rotateX_ckbx = new System.Windows.Forms.CheckBox();

                    rotateX_ckbx.Anchor = ((System.Windows.Forms.AnchorStyles)(
                                            (System.Windows.Forms.AnchorStyles.Top | 
                                            System.Windows.Forms.AnchorStyles.Right)));

                    rotateX_ckbx.AutoSize = true;
                    
                    rotateX_ckbx.Location = new System.Drawing.Point(bhvr_combobx.Right + 30 , 5);
                    
                    rotateX_ckbx.Name = "rotateX_ckbx";
                    
                    rotateX_ckbx.Size = new System.Drawing.Size(65, 17);
                    
                    rotateX_ckbx.Text = "FRAMESNAP";
                    
                    rotateX_ckbx.UseVisualStyleBackColor = true;
                    
                    Controls.Add(rotateX_ckbx);
                    
                    break;

                default:                    
                    keyVal1_numericUpDown.Visible = false;
                    
                    keyVal2_numericUpDown.Visible = false;
                    
                    keyVal3_numericUpDown.Visible = false;
                    
                    break;
            }


            if (isColorField)
            {
                enableColor();
            }
            else
            {
                keyColor_btn.Visible = false;
            }

            enableOverRide(isOverridableField);

            setMinMax(min, max, decimalPlaces);

            setDefaultVal(defaultVals);

            if (isFlagField && comboItems == null)
            {
                this.overKeyName_ckbx.Visible = false;

                this.flag_ckbx.Text = name;
            }
            if (comboItems != null && comboItems.Length > 0)
            {
                this.bhvr_combobx.Visible = true;

                this.isCombo = true;

                this.bhvr_combobx.Items.AddRange(comboItems);

                this.bhvr_combobx.Text = (string)comboItems[0];
            }

            if (isCkbxFields)
            {
                int ckWidth = 65;
                
                int ckStart = 205;
                
                this.units_label.Visible = false;
                
                rotateX_ckbx= new System.Windows.Forms.CheckBox();
                
                rotateY_ckbx= new System.Windows.Forms.CheckBox();
                
                rotateZ_ckbx= new System.Windows.Forms.CheckBox();
                
                translateX_ckbx= new System.Windows.Forms.CheckBox();
                
                translateY_ckbx= new System.Windows.Forms.CheckBox();
                
                translateZ_ckbx= new System.Windows.Forms.CheckBox();

                rotateX_ckbx.AutoSize = true;
                
                rotateX_ckbx.Location = new System.Drawing.Point(ckStart, 5);
                
                rotateX_ckbx.Name = "rotateX_ckbx";
                
                rotateX_ckbx.Size = new System.Drawing.Size(ckWidth, 17);
                
                rotateX_ckbx.Text = "RotateX";
                
                rotateX_ckbx.UseVisualStyleBackColor = true;

                rotateY_ckbx.AutoSize = true;
                
                rotateY_ckbx.Location = new System.Drawing.Point(ckStart + (ckWidth), 5);
                
                rotateY_ckbx.Name = "rotateY_ckbx";
                
                rotateY_ckbx.Size = new System.Drawing.Size(ckWidth, 17);
                
                rotateY_ckbx.Text = "RotateY";
                
                rotateY_ckbx.UseVisualStyleBackColor = true;

                rotateZ_ckbx.AutoSize = true;
                
                rotateZ_ckbx.Location = new System.Drawing.Point(ckStart + (ckWidth * 2), 5);
                
                rotateZ_ckbx.Name = "rotateZ_ckbx";
                
                rotateZ_ckbx.Size = new System.Drawing.Size(ckWidth, 17);
                
                rotateZ_ckbx.Text = "RotateZ";
                
                rotateZ_ckbx.UseVisualStyleBackColor = true;

                translateX_ckbx.AutoSize = true;
                
                translateX_ckbx.Location = new System.Drawing.Point(ckStart + (ckWidth * 3), 5);
                
                translateX_ckbx.Name = "translateX_ckbx";
                
                translateX_ckbx.Size = new System.Drawing.Size(ckWidth, 17);
                
                translateX_ckbx.Text = "TranslateX";
                
                translateX_ckbx.UseVisualStyleBackColor = true;

                translateY_ckbx.AutoSize = true;
                
                translateY_ckbx.Location = new System.Drawing.Point(ckStart + 15 +(ckWidth * 4), 5);
                
                translateY_ckbx.Name = "translateY_ckbx";
                
                translateY_ckbx.Size = new System.Drawing.Size(ckWidth, 17);
                
                translateY_ckbx.Text = "TranslateY";
                
                translateY_ckbx.UseVisualStyleBackColor = true;

                translateZ_ckbx.AutoSize = true;
                
                translateZ_ckbx.Location = new System.Drawing.Point(ckStart + 30 +(ckWidth * 5), 5);
                
                translateZ_ckbx.Name = "translateZ_ckbx"; 
                
                translateZ_ckbx.Size = new System.Drawing.Size(ckWidth, 17);
                
                translateZ_ckbx.Text = "TranslateZ";
                
                translateZ_ckbx.UseVisualStyleBackColor = true;

                Controls.Add(rotateX_ckbx);
                
                Controls.Add(rotateY_ckbx);
                
                Controls.Add(rotateZ_ckbx);
                
                Controls.Add(translateX_ckbx);
                
                Controls.Add(translateY_ckbx);
                
                Controls.Add(translateZ_ckbx);
            }

            this.BackColor = mainBackColor;
        }

        public bool OverrideBhvr
        {
            get
            {
                bool hasOverrideVal = overKeyName_ckbx.Checked || (overKeyName_ckbx.ForeColor == Color.Red);
                return  hasOverrideVal;
            }
        }

        private void reset()
        {
            resetPanel = true;

            bhvrOriginalValue = null;

            bhvrOverrideValue = null;

            isUsed = false;

            overKeyName_ckbx.Checked = false;

            this.BackColor = Color.FromArgb(this.mainBackColor.R - 50, this.mainBackColor.G, this.mainBackColor.B - 50);

            enableOverRide(isOverridable);

            setDefaultVal(defaultValues);

            if(flag_ckbx != null)
                flag_ckbx.Checked = false;

            if(rotateX_ckbx!= null)
                rotateX_ckbx.Checked = false;

            if(rotateY_ckbx!= null)
                rotateY_ckbx.Checked = false;

            if(rotateZ_ckbx!= null)
                rotateZ_ckbx.Checked = false;

            if(translateX_ckbx!= null)
                translateX_ckbx.Checked = false;

            if(translateY_ckbx!= null)
                translateY_ckbx.Checked = false;

            if(translateZ_ckbx!= null)
                translateZ_ckbx.Checked = false;

            if (bhvr_combobx != null && bhvr_combobx.Items.Count > 0)
                bhvr_combobx.Text = (string)bhvr_combobx.Items[0];

            resetPanel = false;
        }
        
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.units_label = new System.Windows.Forms.Label();
            this.bhvr_combobx = new System.Windows.Forms.ComboBox();
            this.keyVal3_numericUpDown = new System.Windows.Forms.NumericUpDown();
            this.keyVal1_numericUpDown = new System.Windows.Forms.NumericUpDown();
            this.keyVal2_numericUpDown = new System.Windows.Forms.NumericUpDown();
            this.flag_ckbx = new System.Windows.Forms.CheckBox();
            this.overKeyName_ckbx = new System.Windows.Forms.CheckBox();
            this.enablePanel_ckbx = new System.Windows.Forms.CheckBox();
            this.keyColor_btn = new System.Windows.Forms.Button();
            this.mnuContextMenu = new ContextMenu();
            this.mnuItemNew = new MenuItem();


            ((System.ComponentModel.ISupportInitialize)(this.keyVal3_numericUpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.keyVal1_numericUpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.keyVal2_numericUpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // flag_ckbx
            // 
            this.flag_ckbx.AutoSize = true;
            this.flag_ckbx.Location = new System.Drawing.Point(3, 5);
            this.flag_ckbx.Name = "flag_ckbx";
            this.flag_ckbx.Size = new System.Drawing.Size(101, 17);
            this.flag_ckbx.TabIndex = 501;
            this.flag_ckbx.Text = "inheritGroupTint";
            this.flag_ckbx.UseVisualStyleBackColor = true;
            this.flag_ckbx.CheckedChanged += new EventHandler(flag_ckbx_CheckedChanged);
            this.flag_ckbx.Visible = false;
            // 
            // overKeyName_ckbx
            // 
            this.overKeyName_ckbx.AutoSize = true;
            this.overKeyName_ckbx.Location = new System.Drawing.Point(150, 5);
            this.overKeyName_ckbx.Name = "overKeyName_ckbx";
            this.overKeyName_ckbx.Size = new System.Drawing.Size(101, 17);
            this.overKeyName_ckbx.TabStop = false;
            this.overKeyName_ckbx.TabIndex = 501;
            this.overKeyName_ckbx.Text = "(Ovr) Key Name";
            this.overKeyName_ckbx.UseVisualStyleBackColor = true;
            this.overKeyName_ckbx.CheckedChanged += new System.EventHandler(this.overKeyName_ckbx_CheckedChanged);
            // 
            // enablePanel_ckbx
            // 
            this.enablePanel_ckbx.AutoSize = true;
            this.enablePanel_ckbx.Location = new System.Drawing.Point(3, 5);
            this.enablePanel_ckbx.Name = "enablePanel_ckbx";
            this.enablePanel_ckbx.Size = new System.Drawing.Size(101, 17);
            this.enablePanel_ckbx.TabStop = false;
            this.enablePanel_ckbx.TabIndex = 501;
            this.enablePanel_ckbx.Text = "";
            this.enablePanel_ckbx.UseVisualStyleBackColor = true;
            this.enablePanel_ckbx.CheckedChanged += new System.EventHandler(this.enablePanel_ckbx_CheckedChanged);
            // 
            // keyColor_btn
            // 
            this.keyColor_btn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.keyColor_btn.BackColor = System.Drawing.Color.Red;
            this.keyColor_btn.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.keyColor_btn.Location = new System.Drawing.Point(156, 2);
            this.keyColor_btn.Name = "keyColor_btn";
            this.keyColor_btn.Size = new System.Drawing.Size(22, 22);
            this.keyColor_btn.TabIndex = 502;
            this.keyColor_btn.UseVisualStyleBackColor = false;
            this.keyColor_btn.ContextMenu = this.mnuContextMenu;
            this.keyColor_btn.Click += new System.EventHandler(this.keyColor_btn_Click);
            //
            // mnuContextMenu
            //
            this.mnuContextMenu.MenuItems.Add(mnuItemNew);

            this.mnuItemNew.Text = "Subtractive";
            this.mnuItemNew.Click += new EventHandler(mnuItemNew_Click);

            // 
            // bhvr_combobx
            //
            this.bhvr_combobx.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.bhvr_combobx.FormattingEnabled = true;
            this.bhvr_combobx.Location = new System.Drawing.Point(182, 2);
            this.bhvr_combobx.Name = "bhvr_combobx";
            this.bhvr_combobx.Size = new System.Drawing.Size(121, 21);
            this.bhvr_combobx.TabIndex = 1;
            this.bhvr_combobx.Visible = false;
            this.bhvr_combobx.SelectedIndexChanged += new System.EventHandler(this.keyVal_numericUpDown_ValueChanged);
            // 
            // keyVal1_numericUpDown
            // 
            this.keyVal1_numericUpDown.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.keyVal1_numericUpDown.DecimalPlaces = 4;
            this.keyVal1_numericUpDown.Location = new System.Drawing.Point(182, 3);
            this.keyVal1_numericUpDown.Name = "keyVal1_numericUpDown";
            this.keyVal1_numericUpDown.Size = new System.Drawing.Size(75, 20);
            this.keyVal1_numericUpDown.TabIndex = 1;
            this.keyVal1_numericUpDown.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.keyVal1_numericUpDown.KeyDown += new KeyEventHandler(numericUpDown_KeyDown);
            this.keyVal1_numericUpDown.KeyUp += new KeyEventHandler(numericUpDown_KeyDown);
            this.keyVal1_numericUpDown.ValueChanged += new System.EventHandler(this.keyVal_numericUpDown_ValueChanged);
            this.keyVal1_numericUpDown.Enter += new EventHandler(numericUpDown_Enter);
            // 
            // keyVal2_numericUpDown
            // 
            this.keyVal2_numericUpDown.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.keyVal2_numericUpDown.DecimalPlaces = 4;
            this.keyVal2_numericUpDown.Location = new System.Drawing.Point(262, 3);
            this.keyVal2_numericUpDown.Name = "keyVal2_numericUpDown";
            this.keyVal2_numericUpDown.Size = new System.Drawing.Size(75, 20);
            this.keyVal2_numericUpDown.TabIndex = 2;
            this.keyVal2_numericUpDown.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.keyVal2_numericUpDown.KeyDown += new KeyEventHandler(numericUpDown_KeyDown);
            this.keyVal2_numericUpDown.KeyUp += new KeyEventHandler(numericUpDown_KeyDown);
            this.keyVal2_numericUpDown.ValueChanged += new System.EventHandler(this.keyVal_numericUpDown_ValueChanged);
            this.keyVal2_numericUpDown.Enter += new EventHandler(numericUpDown_Enter);
            // 
            // keyVal3_numericUpDown
            // 
            this.keyVal3_numericUpDown.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.keyVal3_numericUpDown.DecimalPlaces = 4;
            this.keyVal3_numericUpDown.Location = new System.Drawing.Point(342, 3);
            this.keyVal3_numericUpDown.Name = "keyVal3_numericUpDown";
            this.keyVal3_numericUpDown.Size = new System.Drawing.Size(75, 20);
            this.keyVal3_numericUpDown.TabIndex = 3;
            this.keyVal3_numericUpDown.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.keyVal3_numericUpDown.KeyDown += new KeyEventHandler(numericUpDown_KeyDown);
            this.keyVal3_numericUpDown.KeyUp += new KeyEventHandler(numericUpDown_KeyDown);
            this.keyVal3_numericUpDown.ValueChanged += new System.EventHandler(this.keyVal_numericUpDown_ValueChanged);
            this.keyVal3_numericUpDown.Enter += new EventHandler(numericUpDown_Enter);
            // 
            // units_label
            // 
            this.units_label.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.units_label.Location = new System.Drawing.Point(419, 6);
            this.units_label.Name = "units_label";
            this.units_label.Size = new System.Drawing.Size(100, 13);
            this.units_label.TabIndex = 500;
            this.units_label.Text = "units";
            this.units_label.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // BehaviorKeyPanel
            //
            this.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.Controls.Add(this.enablePanel_ckbx);
            this.Controls.Add(this.overKeyName_ckbx);
            this.Controls.Add(this.keyColor_btn);
            this.Controls.Add(this.flag_ckbx);
            this.Controls.Add(this.bhvr_combobx);
            this.Controls.Add(this.keyVal1_numericUpDown);
            this.Controls.Add(this.keyVal2_numericUpDown);
            this.Controls.Add(this.keyVal3_numericUpDown);
            this.Controls.Add(this.units_label);
            this.MouseClick += new MouseEventHandler(BehaviorKeyPanel_MouseClick);
            this.MouseEnter += new EventHandler(BehaviorKeyPanel_MouseEnter);
            this.Location = new System.Drawing.Point(8, 6);
            this.Name = "BehaviorKeyPanel";
            this.Size = new System.Drawing.Size(width, 26);
            this.TabIndex = 0;
            ((System.ComponentModel.ISupportInitialize)(this.keyVal3_numericUpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.keyVal1_numericUpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.keyVal2_numericUpDown)).EndInit();
            this.ResumeLayout(false);
        }

        void BehaviorKeyPanel_MouseEnter(object sender, EventArgs e)
        {

            ParticalsWin pw = ownerPanel as ParticalsWin;
            BehaviorWin bw = ownerPanel as BehaviorWin;

            if (ModifierKeys == Keys.Control)
            {//((ParticalsWin)this.Parent.Parent.Parent)
                if(pw != null)
                    pw.fToolTip.Active = true;
                else if (bw != null)
                    bw.fToolTip.Active = true;
            }
            else
            {
                if (pw != null)
                    pw.fToolTip.Active = false;
                else if (bw != null)
                    bw.fToolTip.Active = false;
            }
        }

        void mnuItemNew_Click(object sender, EventArgs e)
        {
            Color c = keyColor_btn.BackColor;
            this.keyVal1_numericUpDown.Value = 255 - c.R;

            this.keyVal2_numericUpDown.Value = 255 - c.G;

            this.keyVal3_numericUpDown.Value = 255 - c.B;
        }
       
        #endregion
        //match maya behavior by selecting all digits when entering numericupdown
        private void numericUpDown_Enter(object sender, System.EventArgs e)
        {
            NumericUpDown nud = (NumericUpDown)sender;
            
            nud.Select(0,20);
        }

        public bool Used
        {
            get
            {
                //isUsed = bhvrOriginalValue != null;
                return isUsed;
            }
            set
            {
                isUsed = value;
            }
        }

        private string getPanelValue()
        {
            string val = "";

            val += this.enablePanel_ckbx.Text + " ";

            string[] vals = { null, null, null, null };

            switch (fieldsCount)
            {
                case 1:
                    vals[0] = keyVal1_numericUpDown.Value.ToString();
                    break;

                case 2:
                    vals[0] = keyVal1_numericUpDown.Value.ToString();
                    vals[1] = keyVal2_numericUpDown.Value.ToString();
                    break;

                case 3:
                    vals[0] = keyVal1_numericUpDown.Value.ToString();
                    vals[1] = keyVal2_numericUpDown.Value.ToString();
                    vals[2] = keyVal3_numericUpDown.Value.ToString();
                    break;

                case 5:
                    vals[0] = keyVal1_numericUpDown.Value.ToString();
                    vals[1] = keyVal2_numericUpDown.Value.ToString();
                    vals[2] = bhvr_combobx.Text;

                    if (this.rotateX_ckbx.Checked)
                        vals[3] = rotateX_ckbx.Text;
                    break;
            }

            if (isCombo &&
                fieldsCount == 0)
            {
                vals[0] = bhvr_combobx.Text;
            }

            foreach (string str in vals)
            {
                if (str != null)
                {
                    decimal d;

                    if (decimal.TryParse(str, out d))
                    {
                        if (isColor)
                        {
                            int c = (int)Math.Ceiling(d);

                            val += c.ToString() + " ";
                        }
                        else if (decimal_places == 0)
                        {
                            int c = (int)Math.Ceiling(d);

                            val += c.ToString() + " ";
                        }
                        else
                        {
                            if(str.Contains("."))
                            {
                                string []valSplit = str.Split('.');

                                int m = 0;

                                int.TryParse(valSplit[1], out m);
                                
                                if(m == 0)
                                    val += valSplit[0] + " ";
                                else
                                    val += d.ToString() + " ";
                            }
                            else
                                val += d.ToString() + " ";
                        }
                    }
                    else
                    {
                        val += str + " ";
                    }
                }
            }

            if (isCkbx)
            {
                if (rotateX_ckbx.Checked) val += "RotateX ";

                if (rotateY_ckbx.Checked) val += "RotateY ";

                if (rotateZ_ckbx.Checked) val += "RotateZ ";

                if (translateX_ckbx.Checked) val += "TranslateX ";

                if (translateY_ckbx.Checked) val += "TranslateY ";

                if (translateZ_ckbx.Checked) val += "TranslateZ ";
            }


            return val.Trim();
        }

        public string getOverrideVal()
        {
            string val = "";

            if (this.OverrideBhvr)
            {
                val = getPanelValue();

                if (this.overKeyName_ckbx.ForeColor == Color.Red)
                    val = "#" + val;
            }

            return val;
        }

        public virtual string getVal()
        {
            string val = "";

            bool isOver = this.overKeyName_ckbx.Checked;

            bool cEnable = this.enablePanel_ckbx.Checked;
            
            bool writeBhvrVal = true;

            if (this.overKeyName_ckbx.Checked && bhvrOriginalValue == null)
                writeBhvrVal = false;

            if (writeBhvrVal)
            {
                if (!cEnable)
                {
                    val = "#";
                }

                if (isOver)
                    setVal(this, bhvrOriginalValue, bhvrOverrideValue, false, isLocked);

                val += getPanelValue();

                if (isOver)
                    setVal(this, bhvrOriginalValue, bhvrOverrideValue, isOver, isLocked);
            }

            return val.Trim();
        }

        public void toggleCheckBX()
        {
            resetPanel = true;

            this.overKeyName_ckbx.Checked = !this.overKeyName_ckbx.Checked;

            this.enablePanel_ckbx.Checked = !this.enablePanel_ckbx.Checked;

            resetPanel = false;
        }

        public void setVal(object sender, string bhvrVal, string overrideVal, bool isOverrideValue, bool lockedFile)
        {
            //this.SuspendLayout();

            reset();
            
            isLocked = lockedFile;

            if (Tag.GetType() == typeof(Behavior))
                isDirty = ((Behavior)Tag).isDirty;

            //to avoid loop when overKeyName_ckbox.Checked and numValue changes
            resetPanel = true;

            bhvrOriginalValue = bhvrVal;

            bhvrOverrideValue = overrideVal;

            string val = isOverrideValue ? overrideVal : bhvrVal;

            bool cEnable = bhvrVal != null;

            bool overEnable = isOverridable && isOverrideValue;

            isUsed = overEnable || cEnable;

            if (val != null)
            {
                //remove comment char at the begining of the line and set disable ctl flag
                if (val.StartsWith("#"))
                {
                    cEnable = false;
                    val = val.Trim().Substring(1);
                }
                else
                {
                    this.overKeyName_ckbx.ForeColor = this.enablePanel_ckbx.ForeColor;
                    this.overKeyName_ckbx.Font = new Font(this.overKeyName_ckbx.Font, FontStyle.Regular);
                }

                //remove comment after value
                if (val.Contains("#"))
                {
                    val = val.Trim().Substring(0, val.IndexOf('#') - 1);
                }

                string[] vals = val.Trim().Split(' ');

                decimal v1, v2, v3;

                bool success = false;

                switch (fieldsCount)
                {
                    case 1:
                        success = decimal.TryParse(vals[0], out v1);

                        if (success)
                        {
                            fixMinMax(v1, keyVal1_numericUpDown);

                            keyVal1_numericUpDown.Value = v1;
                        }

                        break;

                    case 2://testing the vals length due to errors in file used in the game and everyone is afraid to fix the error!!!!!!!!!
                        if (vals.Length > 0)
                        {
                            success = decimal.TryParse(vals[0], out v1);

                            if (success)
                            {
                                fixMinMax(v1, keyVal1_numericUpDown);

                                keyVal1_numericUpDown.Value = v1;
                            }
                        }
                        if (vals.Length > 1)
                        {
                            //used to be vals[2] no one found this bug, so I chaged it on 6/8/2011
                            success = decimal.TryParse(vals[1], out v2);

                            if (success)
                            {
                                fixMinMax(v2, keyVal2_numericUpDown);

                                keyVal2_numericUpDown.Value = v2;
                            }
                        }
                        break;

                    case 3://testing the vals length due to errors in file used in the game and everyone is afraid to fix the error!!!!!!!!!
                        if (vals.Length > 0)
                        {
                            success = decimal.TryParse(vals[0], out v1);

                            if (success)
                            {
                                fixMinMax(v1, keyVal1_numericUpDown);

                                keyVal1_numericUpDown.Value = v1;
                            }
                        }

                        if (vals.Length > 1)
                        {
                            success = decimal.TryParse(vals[1], out v2);

                            if (success)
                            {
                                fixMinMax(v2, keyVal2_numericUpDown);

                                keyVal2_numericUpDown.Value = v2;
                            }
                        }

                        if (vals.Length > 2)
                        {
                            success = decimal.TryParse(vals[2], out v3);

                            if (success)
                            {
                                fixMinMax(v3, keyVal3_numericUpDown);

                                keyVal3_numericUpDown.Value = v3;
                            }
                        }
                        if (isColor)
                            setColorBox();

                        break;

                    //Special case 2 num fields, combobox and checkbox
                    case 5://testing the vals length due to errors in file used in the game and everyone is afraid to fix the error!!!!!!!!!
                        if (vals.Length > 0)
                        {
                            success = decimal.TryParse(vals[0], out v1);

                            if (success)
                            {
                                fixMinMax(v1, keyVal1_numericUpDown);

                                keyVal1_numericUpDown.Value = v1;
                            }
                        }

                        if (vals.Length > 1)
                        {
                            success = decimal.TryParse(vals[1], out v2);

                            if (success)
                            {
                                fixMinMax(v2, keyVal2_numericUpDown);

                                keyVal2_numericUpDown.Value = v2;
                            }
                        }
                        if (vals.Length > 2)
                        {

                            if (bhvr_combobx.Items.Contains(vals[2]))
                                bhvr_combobx.Text = vals[2];
                        }
                        if (vals.Length == 4 && rotateX_ckbx.Text.Equals(vals[3]))
                            this.rotateX_ckbx.Checked = true;

                        break;
                }

                if (isCombo &&
                    fieldsCount == 0 &&
                    vals.Length == 1 &&
                    bhvr_combobx.Items.Contains(vals[0]))
                {
                    bhvr_combobx.Text = vals[0];
                }

                if (isCkbx)
                {
                    foreach (string strVal in vals)
                    {
                        switch (strVal)
                        {
                            case "RotateX":

                                rotateX_ckbx.Checked = true;

                                break;

                            case "RotateY":

                                rotateY_ckbx.Checked = true;

                                break;

                            case "RotateZ":

                                rotateZ_ckbx.Checked = true;

                                break;

                            case "TranslateX":

                                translateX_ckbx.Checked = true;

                                break;

                            case "TranslateY":

                                translateY_ckbx.Checked = true;

                                break;

                            case "TranslateZ":

                                translateZ_ckbx.Checked = true;

                                break;
                        }
                    }
                }
            }

            if (overrideVal != null && overrideVal.StartsWith("#"))
            {
                this.bhvrOverrideValue = this.bhvrOverrideValue.Trim().Substring(1);
                overEnable = false;
                this.overKeyName_ckbx.ForeColor = Color.Red;
                this.overKeyName_ckbx.Font = new Font(this.overKeyName_ckbx.Font, FontStyle.Bold | FontStyle.Italic);
            }
            if (bhvrVal != null && bhvrVal.StartsWith("#"))
            {
                this.bhvrOriginalValue = this.bhvrOriginalValue.Trim().Substring(1);
            }
            this.overKeyName_ckbx.Checked = overEnable;

            this.enablePanel_ckbx.Checked = cEnable;

            //force color change
            enablePanel_ckbx_CheckedChanged(enablePanel_ckbx, new EventArgs());

            resetPanel = false;

            //this.ResumeLayout(false);
        }

        public virtual void enable(bool enableControls)
        {
            foreach (Control ctl in this.Controls)
            {
                //exclude enablePanel_ckbx and overKeyName_ckbx 
                if (ctl != enablePanel_ckbx &&
                    ctl != overKeyName_ckbx)
                    ctl.Enabled = enableControls;
            }

            if (!enablePanel_ckbx.Visible)
            {
                flag_ckbx.Enabled = true;
            }
        }
 
        protected void hideEnableCkbs(bool moveOverCkbxLeftTo3)
        {
            this.enablePanel_ckbx.Visible = false;

            if (moveOverCkbxLeftTo3)
            {
                this.overKeyName_ckbx.Location = new Point(3, this.overKeyName_ckbx.Location.Y);
            }
        }       
        
        //Change NumericUpDown Increment based on holding control key wit mouseWheel
        protected void numericUpDown_KeyDown(object sender, KeyEventArgs e)
        {
            NumericUpDown nUD = (NumericUpDown)sender;
           
            if (e.Control)
                nUD.Increment = 1;

            else
                nUD.Increment = 0.001m;
        }
        
        //invoke CheckChanged Event Handler so BhvrWin toggle enable/disable controls
        protected void flag_ckbx_CheckedChanged(object sender, EventArgs e)
        {
            if (CheckChanged != null)
                CheckChanged(sender, e);
        }

        protected virtual void overKeyName_ckbx_CheckedChanged(object sender, EventArgs e)
        {
            if (!resetPanel && !isDirty )
            {
                isDirty = true;

                if (IsDirtyChanged != null)
                    IsDirtyChanged(this, e);
            }

            if (!resetPanel)
            {
                setVal(this, this.bhvrOriginalValue, this.bhvrOverrideValue, overKeyName_ckbx.Checked, isLocked);
            }
            if (this.bhvrOverrideValue != null && !this.overKeyName_ckbx.Checked)
            {
                this.overKeyName_ckbx.ForeColor = Color.Red;
                this.overKeyName_ckbx.Font = new Font(this.overKeyName_ckbx.Font, FontStyle.Bold | FontStyle.Italic);
            }
            else
            {
                this.overKeyName_ckbx.ForeColor = this.enablePanel_ckbx.ForeColor;
                this.overKeyName_ckbx.Font = new Font(this.overKeyName_ckbx.Font, FontStyle.Regular);
            }

            //change backcolor indicating override flag
            if (this.overKeyName_ckbx.Checked)
            {
                this.BackColor = Color.FromArgb(255, Math.Max(0, (int)mainBackColor.R),
                                                     Math.Max(0, (int)mainBackColor.G - 20),
                                                     Math.Max(0, (int)mainBackColor.B - 50));

                enable(this.overKeyName_ckbx.Checked);
            }

            //change backcolor indicating commented value    
            else if (!enablePanel_ckbx.Checked)
            {
                this.BackColor = Color.FromArgb(Math.Max(0, (int)mainBackColor.R - 50),
                                                Math.Max(0, (int)mainBackColor.G),
                                                Math.Max(0, (int)mainBackColor.B - 50));
            }

            else
            {
                this.BackColor = mainBackColor;

                if (bhvrOriginalValue != null)
                {
                    enablePanel_ckbx.Checked = true;
                }
            }
        }
  
        protected void keyColor_btn_Click(object sender, EventArgs e)
        {
            COH_CostumeUpdater.assetEditor.aeCommon.COH_ColorPicker colorPicker = new COH_CostumeUpdater.assetEditor.aeCommon.COH_ColorPicker();
            colorPicker.setColor(this.keyColor_btn.BackColor);

            Color c = colorPicker.getColor();

            if (!c.Equals(keyColor_btn.BackColor))
            {
                keyColor_btn.BackColor = c;

                this.keyVal1_numericUpDown.Value = c.R;

                this.keyVal2_numericUpDown.Value = c.G;

                this.keyVal3_numericUpDown.Value = c.B;
            }
        }

        protected void invokeIsDirtyChanged()
        {
            if (IsDirtyChanged != null)
                IsDirtyChanged(this, new EventArgs());
        }

        private void BehaviorKeyPanel_MouseClick(object sender, MouseEventArgs e)
        {
            this.Focus();
        }

        private void fixMinMax(decimal dValue, NumericUpDown numUpDwn)
        {
            if (dValue > numUpDwn.Maximum)
                numUpDwn.Maximum = dValue;

            if (dValue < numUpDwn.Minimum)
                numUpDwn.Minimum = dValue;

            int dPalces;

            if (!isColor && dValue.ToString().IndexOf('.') > -1)
            {
                dPalces = dValue.ToString().Substring(dValue.ToString().IndexOf('.')).Length;
                numUpDwn.DecimalPlaces = dPalces;
            }
        }

        private void setDefaultVal(decimal [] dVals)
        {
            if (dVals != null)
            {
                this.keyVal1_numericUpDown.Value = dVals[0];

                if (dVals.Length > 1)
                    this.keyVal2_numericUpDown.Value = dVals[1];

                if (dVals.Length > 2)
                    this.keyVal3_numericUpDown.Value = dVals[2];
            }
        }

        private void setMinMax(int min, int max, int decimalPlaces)
        {
            this.keyVal1_numericUpDown.Maximum = new decimal(max);

            this.keyVal1_numericUpDown.Minimum = new decimal(min);

            this.keyVal1_numericUpDown.DecimalPlaces = decimalPlaces;


            this.keyVal2_numericUpDown.Maximum = new decimal(max);

            this.keyVal2_numericUpDown.Minimum = new decimal(min);

            this.keyVal2_numericUpDown.DecimalPlaces = decimalPlaces;


            this.keyVal3_numericUpDown.Maximum = new decimal(max);

            this.keyVal3_numericUpDown.Minimum = new decimal(min);

            this.keyVal3_numericUpDown.DecimalPlaces = decimalPlaces;
        }

        private void enableOverRide(bool isOverridable)
        {
            this.overKeyName_ckbx.Enabled = isOverridable;
        }

        private void enableColor()
        {
            this.keyColor_btn.Enabled = true;

            this.keyVal1_numericUpDown.Maximum = new decimal(new int[] { 255, 0, 0, 0 });

            this.keyVal1_numericUpDown.Value = new decimal(new int[] { 255, 0, 0, 0 });

            this.keyVal1_numericUpDown.DecimalPlaces = 0;

            //this.keyVal1_numericUpDown.ValueChanged += new System.EventHandler(this.keyVal_numericUpDown_ValueChanged);

            
            this.keyVal2_numericUpDown.Maximum = new decimal(new int[] { 255, 0, 0, 0 });

            this.keyVal2_numericUpDown.Value = new decimal(new int[] { 255, 0, 0, 0 });

            this.keyVal2_numericUpDown.DecimalPlaces = 0;

            //this.keyVal2_numericUpDown.ValueChanged += new System.EventHandler(this.keyVal_numericUpDown_ValueChanged);

            
            this.keyVal3_numericUpDown.Maximum = new decimal(new int[] { 255, 0, 0, 0 });

            this.keyVal3_numericUpDown.Value = new decimal(new int[] { 255, 0, 0, 0 });

            this.keyVal3_numericUpDown.DecimalPlaces = 0;

            //this.keyVal3_numericUpDown.ValueChanged += new System.EventHandler(this.keyVal_numericUpDown_ValueChanged);

            keyVal_numericUpDown_ValueChanged(keyVal3_numericUpDown, new EventArgs());
        }

        private void enablePanel_ckbx_CheckedChanged(object sender, EventArgs e)
        {
            if (!resetPanel && !isDirty)
            {
                isDirty = true;

                if (IsDirtyChanged != null)
                    IsDirtyChanged(this, e);
            }

            this.SuspendLayout();

            //disable controls if enablepanel and overkeyname are off 
            enable(this.enablePanel_ckbx.Checked|overKeyName_ckbx.Checked);

            if (!this.overKeyName_ckbx.Checked)
            {
                if (enablePanel_ckbx.Checked)
                {
                    this.BackColor = mainBackColor;
                }
                else
                {
                    //shift backcolor to green indicating commented value while maintaining alternate row backcolor
                    this.BackColor = Color.FromArgb(Math.Max(0, (int)mainBackColor.R - 50),
                                                    Math.Max(0, (int)mainBackColor.G),
                                                    Math.Max(0, (int)mainBackColor.B - 50));
                }
            }

            this.ResumeLayout(true);
        }

        protected void setColorBox()
        {
            if (keyColor_btn != null && isColor)
            {
                int r = (int)Math.Min(255, Math.Max(this.keyVal1_numericUpDown.Value, 0));

                int g = (int)Math.Min(255, Math.Max(this.keyVal2_numericUpDown.Value, 0));

                int b = (int)Math.Min(255, Math.Max(this.keyVal3_numericUpDown.Value, 0));

                Color c = Color.FromArgb(255, r, g, b);

                if (!c.Equals(keyColor_btn.BackColor))
                {
                    keyColor_btn.BackColor = c;
                }
            }
        }

        protected void keyVal_numericUpDown_ValueChanged(object sender, EventArgs e)
        {
            if (!resetPanel && !isDirty )
            {
                isDirty = true;
                
                resetPanel = true;

                if (this.bhvrOriginalValue == null && enablePanel_ckbx.Checked)
                {
                    this.bhvrOriginalValue = " ";

                    this.bhvrOriginalValue = getVal();
                }                

                resetPanel = false;

                if (IsDirtyChanged != null)
                    IsDirtyChanged(this, e);
            }

            //enable physics panels if physics value is one
            if (this.Name.Equals("physics_BehaviorKeyPanel"))
            {
                //decimal value check may not be allways one so we get the delta to be less than 0.01
                flag_ckbx.Checked = (1.0m - keyVal1_numericUpDown.Value) < 0.01m ? true : false;

                flag_ckbx_CheckedChanged(this.flag_ckbx, e);
            }

            else if (keyColor_btn != null && keyColor_btn.Visible)
            {
                setColorBox();
            }
        }
  }
}
