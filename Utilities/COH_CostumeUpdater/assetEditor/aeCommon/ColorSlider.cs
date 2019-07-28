using System;
using System.Windows;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Windows.Forms;




namespace COH_CostumeUpdater.assetEditor.aeCommon
{
    class ColorSlider:Panel
    {
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.NumericUpDown numericUpDown1;
        private System.Windows.Forms.NumericUpDown numericUpDown2;
        private System.Windows.Forms.Label label1;
        private System.ComponentModel.IContainer components = null;
        private int lableXlocation;
        private int lableYlocation;
        private int lableHeight;
        private int lableWidth;
        private int numericUpDownXlocation;
        private int numericUpDownYlocation;
        private int numericUpDownHeight;
        private int numericUpDownWidth;
        private int panel2Xlocation;
        private int panel2Ylocation;
        private int panel2Height;
        private int panel2Width;
        private int button2Xlocation;
        private int button2Ylocation;
        private int button2Height;
        private int button2Width;
        private bool isVertical;

        //event handling delegate
        public event EventHandler ValueChanged;
        public decimal oldVal;
        public string csLabel;

        public ColorSlider(string label)
        {
            setLayout();
            InitializeComponent();
            label1.Text = label;
            csLabel = label;
            oldVal = Value;
        }

        public void setBKimage(Bitmap bkImage)
        {
            this.SuspendLayout();
            this.panel2.BackgroundImage = bkImage;
            this.ResumeLayout(false);
        }

        protected void OnValueChanged(EventArgs e)
        {
            if (ValueChanged != null)
                ValueChanged(this,e );
        }

        private void setLayout()
        {
                lableXlocation = 0;
                lableYlocation = 3;
                lableHeight = 13;
                lableWidth = 18;

                numericUpDownXlocation = 20;
                numericUpDownYlocation = 0;
                numericUpDownHeight = 20;
                numericUpDownWidth = 60;

                panel2Xlocation = 80;
                panel2Ylocation = 0;
                panel2Height = 20;
                panel2Width = 200;

                button2Xlocation = 82;
                button2Ylocation = 1;
                button2Height = 18;
                button2Width = 10;
        }

        private decimal value;

        public decimal Value
        {
            get
            {
                value = numericUpDown1.Value;
                return value;
            }
            set
            {
                numericUpDown1.Value = Value;
                oldVal = value;
                value = numericUpDown1.Value;
            }
           
        }
        public void setVal(decimal v)
        {
            oldVal = numericUpDown1.Value;
            numericUpDown1.Value = Math.Min(1, Math.Max(0, v));
            numericUpDown2.Value = Math.Floor(255*numericUpDown1.Value);

        }
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        private void InitializeComponent()
        {

            this.label1 = new System.Windows.Forms.Label();
            this.numericUpDown1 = new System.Windows.Forms.NumericUpDown();
            this.numericUpDown2 = new System.Windows.Forms.NumericUpDown();
            this.panel2 = new System.Windows.Forms.Panel();
            this.button2 = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).BeginInit();
            this.panel2.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(lableXlocation, lableYlocation);
            this.label1.Name = "label1";
            this.label1.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.label1.Size = new System.Drawing.Size(lableWidth, lableHeight);
            this.label1.TabIndex = 6;
            this.label1.Text = "H:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // numericUpDown1
            // 
            this.numericUpDown1.DecimalPlaces = 4;
            this.numericUpDown1.Increment = new decimal(new int[] {
            5,
            0,
            0,
            131072});
            this.numericUpDown1.Location = new System.Drawing.Point(numericUpDownXlocation, numericUpDownYlocation);
            this.numericUpDown1.Maximum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDown1.Name = "numericUpDown1";
            this.numericUpDown1.Size = new System.Drawing.Size(numericUpDownWidth, numericUpDownHeight);
            this.numericUpDown1.TabIndex = 5;
            this.numericUpDown1.Tag = true;
            this.numericUpDown1.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.numericUpDown1.ValueChanged += new System.EventHandler(this.numericUpDown1_ValueChanged);
            // 
            // panel2
            // 
            this.panel2.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel2.Controls.Add(this.button2);
            this.panel2.Location = new System.Drawing.Point(panel2Xlocation, panel2Ylocation);
            this.panel2.Name = "panel2";
            this.panel2.Text = "sSliderPanel2";
            this.panel2.Size = new System.Drawing.Size(panel2Width, panel2Height);
            this.panel2.TabIndex = 4;
            this.panel2.MouseMove += new System.Windows.Forms.MouseEventHandler(this.panel2_MouseMove);
            this.panel2.MouseDown += new System.Windows.Forms.MouseEventHandler(this.panel2_MouseDown);
            this.panel2.MouseUp += new System.Windows.Forms.MouseEventHandler(this.panel2_MouseUp);
            // 
            // button2
            // 
            this.button2.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.button2.Enabled = false;
            this.button2.Location = new System.Drawing.Point(button2Xlocation, button2Ylocation);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(button2Width, button2Height);
            this.button2.TabIndex = 0;
            this.button2.UseVisualStyleBackColor = true;
            this.button2.LocationChanged += new EventHandler(button2_LocationChanged);
            // 
            // numericUpDown2
            // 
            this.numericUpDown2.DecimalPlaces = 0;
            this.numericUpDown2.Increment = 1;
            this.numericUpDown2.Location = new System.Drawing.Point(numericUpDownXlocation + 258, numericUpDownYlocation);
            this.numericUpDown2.Maximum = 255;
            this.numericUpDown2.Name = "numericUpDown2";
            this.numericUpDown2.Size = new System.Drawing.Size(numericUpDownWidth, numericUpDownHeight);
            this.numericUpDown2.TabIndex = 5;
            this.numericUpDown2.Tag = true;
            this.numericUpDown2.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.numericUpDown2.ValueChanged += new System.EventHandler(this.numericUpDown2_ValueChanged);
            // 
            // ColorSlider
            // 
            this.Controls.Add(this.numericUpDown2);//////////////
            this.Controls.Add(this.label1);
            this.Controls.Add(this.numericUpDown1);
            this.Controls.Add(this.button2);
            this.Controls.Add(this.panel2);
            this.Name = "cSlider";
            this.Text = "cSlider";

            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown1)).EndInit();
            this.panel2.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        void numericUpDown1_ValueChanged(object sender, System.EventArgs e)
        {
            if ((bool)numericUpDown1.Tag)
            {
                int x = (int)(this.numericUpDown1.Value * (this.panel2.Width - button2.Width));
                x = Math.Max(panel2.Left, Math.Min(x + panel2.Left, (panel2.Width - button2.Width) + panel2.Left));
                this.button2.Location = new System.Drawing.Point(x, 1);
                oldVal = this.value;
                this.value = this.numericUpDown1.Value;

                numericUpDown2.Tag = false;
                numericUpDown2.Value = Math.Ceiling(255 * numericUpDown1.Value);
                numericUpDown2.Tag = true;
            }
        }

        void numericUpDown2_ValueChanged(object sender, System.EventArgs e)
        {
            if ((bool)numericUpDown2.Tag)
            {
                numericUpDown1.Tag = false;
                numericUpDown1.Value = (numericUpDown2.Value / 255.0m);
                numericUpDown1.Tag = true;
                int x = (int)(this.numericUpDown1.Value * (this.panel2.Width - button2.Width));
                x = Math.Max(panel2.Left, Math.Min(x + panel2.Left, (panel2.Width - button2.Width) + panel2.Left));
                this.button2.Location = new System.Drawing.Point(x, 1);
                oldVal = this.value;
                this.value = this.numericUpDown1.Value;
            }
        }

        void button2_LocationChanged(object sender, EventArgs e)
        {
            oldVal = this.value;
            this.value = this.numericUpDown1.Value;
            numericUpDown2.Tag = false;
            numericUpDown2.Value = Math.Ceiling(255 * numericUpDown1.Value);
            numericUpDown2.Tag = true;
            OnValueChanged(new EventArgs());
        }

        void panel2_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                numericUpDown1.Tag = false;
                
                this.SuspendLayout();
                int x = Math.Max(panel2.Left, Math.Min(e.X+panel2.Left, (panel2.Width - button2.Width)+panel2.Left));
                decimal val = (x - panel2.Left) / (decimal)((panel2.Width - button2.Width));
                this.numericUpDown1.Value = val;
                button2.Location = new System.Drawing.Point(x, 1);

                this.ResumeLayout(false);
            }
        }

        void panel2_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            panel2_MouseMove(sender, e);
        }
        void panel2_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            numericUpDown1.Tag = true;
        }
    }
}
