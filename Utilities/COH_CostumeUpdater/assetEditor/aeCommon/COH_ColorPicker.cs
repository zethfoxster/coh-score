using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.assetEditor.aeCommon
{
    public partial class COH_ColorPicker : Form
    {
        public COH_ColorPicker()
        {
            this.mSliderR = new COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider("R:");
            this.mSliderG = new COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider("G:");
            this.mSliderB = new COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider("B:");
            this.mSliderV = new COH_CostumeUpdater.assetEditor.aeCommon.ColorSlider("V:");

            InitializeComponent();
            
            doSliderValueChange = true;
            this.colorWheelPicker.Controls.Add(cwMarker);
            this.ResumeLayout(false);
            this.cwMarker.BackColor = Color.FromArgb(0, 0, 0, 0);
            this.SetStyle(ControlStyles.UserPaint |
                          ControlStyles.SupportsTransparentBackColor |
                          ControlStyles.OptimizedDoubleBuffer, true);

            Bitmap bcwM = (Bitmap)COH_CostumeUpdater.Properties.Resources.cwMarkerIcon.Clone();
            bcwM.MakeTransparent(Color.White);
            this.cwMarker.BackgroundImage = bcwM;
            aeCommon.ColorWheel cw = new aeCommon.ColorWheel();
            this.colorWheelPicker.Image = cw.colorCircle;
            this.mSliderR.setBKimage(cw.colorStripR);
            this.mSliderG.setBKimage(cw.colorStripG);
            this.mSliderB.setBKimage(cw.colorStripB);
            this.mSliderV.setBKimage(cw.colorStripV);
            
            this.Refresh();
        }

        public void setColor(Color inColor)
        {
            doSliderValueChange = false;
            this.mSliderR.setVal(Math.Min(1, Math.Max(0,(decimal)(inColor.R / 255.0))));
            this.mSliderG.setVal(Math.Min(1, Math.Max(0, (decimal)(inColor.G / 255.0))));
            this.mSliderB.setVal(Math.Min(1, Math.Max(0, (decimal)(inColor.B / 255.0))));
            mSliderV.setVal(Math.Min(1, Math.Max(0,HsvColor.getValue(inColor))));
            this.panel2.BackColor = inColor;
            oldColor = inColor;
            setCWmarker(inColor);
            doSliderValueChange = true;
        }

        public Color getColor()
        {
            Color c = this.panel2.BackColor;
            readLastColor();
            DialogResult dr = this.ShowDialog();

            if (dr == DialogResult.OK)
            {
                saveLastColor();
                return this.panel2.BackColor;
            }
            else
                return c;
        }

        private void saveLastColor()
        {
            int dwrd = panel2.BackColor.ToArgb();
            string regKeyName = "COH_AET_ColorPicker_lc";
            Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(regKeyName, true);

            if(key == null)
                key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(regKeyName);            

            key.SetValue(regKeyName, dwrd, Microsoft.Win32.RegistryValueKind.DWord);
            key.Close();

        }

        private void readLastColor()
        {
            string regKeyName = "COH_AET_ColorPicker_lc";

            Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.OpenSubKey(regKeyName);
            if (key != null)
            {
                object kVal = key.GetValue(regKeyName);

                if (kVal != null)
                    lastColorBtn.BackColor = Color.FromArgb((int)kVal);

                key.Close();
            }
        }            
            
        void colorWheelPicker_MouseLeave(object sender, System.EventArgs e)
        {
            this.Cursor = Cursors.Arrow;
            doSliderValueChange = true;
        }

        void colorWheelPicker_MouseEnter(object sender, System.EventArgs e)
        {
            this.Cursor = new Cursor(@"Resources\eyeyDrop.cur");
            doSliderValueChange = false;
        }

        void colorWheelPicker_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
        {
                        
            if (e.Button == MouseButtons.Left)
            {
                try
                {
                    int offset = 8;
                    Point p = getCWMarkerLoc(e.Location);
                    cwMarker.Location = p;
                    Color c = ((Bitmap)colorWheelPicker.Image).GetPixel(p.X + offset, p.Y + offset);
                    this.panel2.BackColor = c;
                    oldColor = c;
                    decimal R = (decimal)(c.R / 255.0f);
                    decimal G = (decimal)(c.G / 255.0f);
                    decimal B = (decimal)(c.B / 255.0f);
                    mSliderR.setVal(R);
                    mSliderG.setVal(G);
                    mSliderB.setVal(B);
                    mSliderV.setVal(HsvColor.getValue(c));

                }
                catch (Exception ex)
                {
                }
            }

        }

        void colorWheelPicker_Click(object sender, System.EventArgs e)
        {
            Point p = this.colorWheelPicker.PointToClient(Cursor.Position);
            colorWheelPicker_MouseMove(sender, new System.Windows.Forms.MouseEventArgs(MouseButtons.Left,1, p.X,p.Y,0));
        }

        private Point getCWMarkerLoc(Point mousePos)
        {
            int xOffset = -8;// 4;
            int yOffset = -8;// 20;
            Point pp = new Point(0, 0);
            int maxWidth = colorWheelPicker.Width - 16;
            int maxHeight = colorWheelPicker.Height - 16;

            int X = Math.Max(0, Math.Min(maxWidth, mousePos.X + xOffset));
            int Y = Math.Max(0, Math.Min(maxHeight, mousePos.Y + yOffset));

            float radius = maxHeight / 2.0f;
            PointF center = new PointF(maxWidth / 2.0f, maxHeight / 2.0f);

            double radians = Math.Atan2((double)(Y - 120), (double)(X - 120));
            int x = (int)(center.X + Math.Floor(radius * Math.Cos(radians)));
            int y = (int)(center.Y + Math.Floor(radius * Math.Sin(radians)));


            if (X <= 120 & Y <= 120)
            {
                pp = new Point(Math.Max(X, x), Math.Max(Y, y));
            }
            else if (X > 120 & Y <= 120)
            {
                pp = new Point(Math.Min(X, x), Math.Max(Y, y));
            }
            else if (X > 120 & Y > 120)
            {
                pp = new Point(Math.Min(X, x), Math.Min(Y, y));
            }
            else
            {
                pp = new Point(Math.Max(X, x), Math.Min(Y, y));

            }
            return pp;
        }

        void mcSlider_ValueChanged(object sender, System.EventArgs e)
        {
            float hue = 0.0f;
            float saturation = 0.0f;
            string csLabel = ((ColorSlider)sender).csLabel;

            if (doSliderValueChange)
            {
                if (csLabel.Equals("V:"))
                {
                    hue = this.oldColor.GetHue();
                    saturation = HsvColor.getSaturation(this.oldColor);
                    Color mc = HsvColor.ColorFromHSV((double)hue, (double)saturation, (double)mSliderV.Value);
                    doSliderValueChange = false;
                    this.mSliderR.setVal((decimal)(mc.R / 255.0f));
                    this.mSliderG.setVal((decimal)(mc.G / 255.0f));
                    this.mSliderB.setVal((decimal)(mc.B / 255.0f));
                    doSliderValueChange = true;
                }
                Color c = Color.FromArgb(255,
                                         (int)(mSliderR.Value * 255),
                                         (int)(mSliderG.Value * 255),
                                         (int)(mSliderB.Value * 255));
                this.panel2.BackColor = c;
                
                setCWmarker(c);

                if (!csLabel.Equals("V:"))
                {
                    oldColor = this.panel2.BackColor;
                    setValSlider(c);
                }
            }

        }
        private void setValSlider(Color c)
        {
            doSliderValueChange = false;
            this.mSliderV.setVal((HsvColor.getValue(c)));
            doSliderValueChange = true;
        }
        private void setCWmarker(Color c)
        {
            float hue = c.GetHue();
            float saturation = getSat(c);
            double radius = saturation * 120;
            double radians = (360 - hue) * (Math.PI / 180.0);
            int x = (int)(120 + Math.Floor(radius * Math.Cos(radians)));
            int y = (int)(120 + Math.Floor(radius * Math.Sin(radians)));

            Point p = new Point(x, y);
            cwMarker.Location = p;
        }
        private void cStore_Click(object sender, EventArgs e)
        {
            Color c = ((Button)sender).BackColor;
            oldColor = c;
            this.panel2.BackColor = c;
            setValSlider(c);

            decimal r = (decimal)(c.R / 255.0f);
            decimal g = (decimal)(c.G / 255.0f);
            decimal b = (decimal)(c.B / 255.0f);

            doSliderValueChange = false;
            mSliderR.setVal(r);
            mSliderG.setVal(g);
            mSliderB.setVal(b);
            doSliderValueChange = true;
            setCWmarker(c);
        }

        private float getSat(Color c)
        {
            int max = Math.Max(c.R, Math.Max(c.G, c.B));
            int min = Math.Min(c.R, Math.Min(c.G, c.B));
            int medium = getMiddle(max, min, c);
            float sat = ((float)((max - medium) + (medium - min))) / 255.0f;
            return sat;
        }

        private int getMiddle(int max, int min, Color c)
        {
            int[] color = new int[] { c.R, c.G, c.B };
            ArrayList colorList = new ArrayList(color);
            colorList.RemoveAt(colorList.IndexOf(max));
            colorList.RemoveAt(colorList.IndexOf(min));
            return (int)colorList[0];
        }
    }
}
