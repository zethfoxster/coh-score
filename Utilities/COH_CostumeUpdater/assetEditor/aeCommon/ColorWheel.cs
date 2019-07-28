using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.assetEditor.aeCommon
{
    class HsvColor
    {
        public double H;
        public double S;
        public double V;

        public HsvColor(double h, double s, double v)
        {
            this.H = h;
            this.S = s;
            this.V = v;
        }
        
        public HsvColor(Color rgbColor)
        {
            this.H = rgbColor.GetHue();
            this.S = HsvColor.getSaturation(rgbColor);
            this.V = (double)HsvColor.getValue(rgbColor);
        }
        private int getMedium(int max, int min, Color c)
        {
            string str = string.Format("{0} {1} {2}", c.R, c.G, c.B);
            str = str.Remove(str.IndexOf(max.ToString()), max.ToString().Length);
            str = str.Remove(str.IndexOf(min.ToString()), min.ToString().Length).Trim();
            int medium = System.Convert.ToInt32(str);
            return medium;
        }

        public static float getSaturation(Color c)
        {
            int max = Math.Max(c.R, Math.Max(c.G, c.B));
            int min = Math.Min(c.R, Math.Min(c.G, c.B));
            int chroma = max - min;
            decimal value = HsvColor.getValue(c);
            float sat = (float)( chroma == 0 ? 0 : (chroma / value));
            return (sat/255.0f);
        }

        public static decimal getValue(Color c)
        {
            //int bw = (int)(c.R * 0.299 + c.G * 0.587 + c.B * 0.114);
            int brightness = (Math.Max(c.R, Math.Max(c.B, c.G)));
            return (decimal)(brightness / 255.0);
        }

        public static decimal getBrightness(Color c)
        {
            int brightness = (int)Math.Sqrt(c.R * c.R * 0.241 + c.G * c.G * 0.691 + c.B * c.B * 0.068);
            return (decimal)(brightness / 255.0);
        }

        //http://stackoverflow.com/questions/1335426/is-there-a-built-in-c-net-system-api-for-hsv-to-rgb
        public static Color ColorFromHSV(double hue, double saturation, double value) 
        { 
            int hi = Convert.ToInt32(Math.Floor(hue / 60)) % 6; 
            double f = hue / 60 - Math.Floor(hue / 60); 
         
            value = value * 255; 
            int v = Convert.ToInt32(value); 
            int p = Convert.ToInt32(value * (1 - saturation)); 
            int q = Convert.ToInt32(value * (1 - f * saturation)); 
            int t = Convert.ToInt32(value * (1 - (1 - f) * saturation)); 
         
            if (hi == 0) 
                return Color.FromArgb(255, v, t, p); 
            else if (hi == 1) 
                return Color.FromArgb(255, q, v, p); 
            else if (hi == 2) 
                return Color.FromArgb(255, p, v, t); 
            else if (hi == 3) 
                return Color.FromArgb(255, p, q, v); 
            else if (hi == 4) 
                return Color.FromArgb(255, t, p, v); 
            else 
                return Color.FromArgb(255, v, p, q); 
        } 
    }

    class ColorWheel
    {
        private int ColorCount = 360;
        public Bitmap colorCircle;
        public Bitmap colorStripR;
        public Bitmap colorStripG;
        public Bitmap colorStripB;
        public Bitmap colorStripV;
        public Color selectedColor;

        public ColorWheel()
        {
            this.colorCircle = new Bitmap(256, 256, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            this.colorStripR = new Bitmap(256, 50, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            this.colorStripG = new Bitmap(256, 50, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            this.colorStripB = new Bitmap(256, 50, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            this.colorStripV = new Bitmap(256, 50, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
            this.selectedColor = Color.Red;
            this.CreateGradient();
        }

        private Point[] getPoints(double radius, Point center)
        {
            Point [] points = new Point[ColorCount];

            for(int i=0; i < points.Length; i++)
            {
                points[i] = getPoint(i,radius,center);
            }
            return points;
        }

        private Point getPoint(double degree, double radius, Point center)
        {
            double radians = degree * (Math.PI/180.0);
            int x = (int)(center.X + Math.Floor(radius * Math.Cos(radians)));
            int y = (int)(center.Y + Math.Floor(radius * Math.Sin(radians)));

            Point point = new Point(x,y);
            return point;
        }

        private Color[] getColors()
        {
            Color [] colors = new Color[ColorCount];

            for(int i=0; i < colors.Length; i++)
            {
                int offset = 360 - i;
                colors[i] = HsvColor.ColorFromHSV(offset,1,1);
            }
            return colors;
        }
        
        private void CreateGradient()
        {
            Graphics newGraphics =  null;
            PathGradientBrush pgb = null;
            LinearGradientBrush lgb = null; 

            try
            {
                int radius = 256;
                Point center = new Point(radius / 2, radius / 2);
                float[] mFactors = { 0.0f, 0.5f,1.0f, 1.0f};
                float[] mPositions = { 0.0f, 0.25f, 0.5f,1.0f};
                Blend blend = new Blend();
                blend.Factors = mFactors;
                blend.Positions = mPositions;
               


                // Create a new PathGradientBrush, supplying an array of points created 
                // by calling the GetPoints method.
                pgb = new PathGradientBrush( getPoints((double)radius, center));

                // Set the various properties. Note the SurroundColors property, which 
                // contains an array of points, in a one-to-one relationship with the 
                // points that created the gradient.
                pgb.CenterColor = Color.White;
                pgb.CenterPoint = center;
                pgb.SurroundColors = getColors();
                pgb.Blend = blend;

                newGraphics = Graphics.FromImage(colorCircle);
                newGraphics.FillEllipse(pgb, 0, 0, colorCircle.Width, colorCircle.Height);


                Color[] recColors = new Color[] {Color.Black, Color.White, selectedColor, Color.Black};
                lgb = new LinearGradientBrush(new Rectangle(0,0,colorStripR.Width,colorStripR.Height),
                                                Color.Black,
                                                Color.FromArgb(255, 255, 0, 0), 
                                                0.0f);


                newGraphics = Graphics.FromImage(colorStripR);
                newGraphics.FillRectangle(lgb, 0, 0, colorStripR.Width, colorStripR.Height);

                lgb = new LinearGradientBrush(new Rectangle(0, 0, colorStripR.Width, colorStripR.Height),
                                                Color.Black,
                                                Color.FromArgb(255,0,255,0),
                                                0.0f);
                newGraphics = Graphics.FromImage(colorStripG);
                newGraphics.FillRectangle(lgb, 0, 0, colorStripG.Width, colorStripG.Height);

                lgb = new LinearGradientBrush(new Rectangle(0, 0, colorStripR.Width, colorStripR.Height),
                                                Color.Black,
                                                Color.FromArgb(255, 0, 0, 255),
                                                0.0f);
                newGraphics = Graphics.FromImage(colorStripB);
                newGraphics.FillRectangle(lgb, 0, 0, colorStripB.Width, colorStripB.Height);

                lgb = new LinearGradientBrush(new Rectangle(0, 0, colorStripR.Width, colorStripR.Height),
                                                                Color.Black,
                                                                Color.FromArgb(255, 255, 255, 255),
                                                                0.0f);
                newGraphics = Graphics.FromImage(colorStripV);
                newGraphics.FillRectangle(lgb, 0, 0, colorStripV.Width, colorStripV.Height); 

            }
            catch(Exception ex)
            {
                System.Windows.Forms.MessageBox.Show(ex.Message);
            }

            finally
            {
                if(pgb != null)
                  pgb.Dispose();
                
                if(newGraphics != null)
                  newGraphics.Dispose();
             
            }

        }
    }

}
 
