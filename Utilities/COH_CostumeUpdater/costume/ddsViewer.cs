using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.costume
{
    public partial class ddsViewer : Form
    {
        private int flags;
        private float[] fade;
        private bool hasAlpha;
        private bool turnAlphaOn;
        private Bitmap mImage;
        private PresentParameters pp;
        private Device device;
        private string saveImagePath;
        private int saveImagW;
        private int saveImagH;

        public ddsViewer(string fileName)
        {

            InitializeComponent();
            pp = new PresentParameters();
            pp.Windowed = true;
            pp.SwapEffect = SwapEffect.Copy;
            device = new Device(0, DeviceType.Hardware, this, CreateFlags.HardwareVertexProcessing, pp);
            viewDDs(fileName);
        }
        
        public ddsViewer(string fileName, bool alphaOn)
        {
            InitializeComponent();
            pp = new PresentParameters();
            pp.Windowed = true;
            pp.SwapEffect = SwapEffect.Copy;
            device = new Device(0, DeviceType.Hardware, this, CreateFlags.HardwareVertexProcessing, pp);
            this.turnAlphaOn = alphaOn;

            viewDDs(fileName);
        }

        public ddsViewer(string saveImgLocation, int w, int h, bool alphaOn)
        {
            InitializeComponent();
            pp = new PresentParameters();
            pp.Windowed = true;
            pp.SwapEffect = SwapEffect.Copy;
            device = new Device(0, DeviceType.Hardware, this, CreateFlags.HardwareVertexProcessing, pp);
            this.turnAlphaOn = alphaOn;
            saveImagePath = saveImgLocation;
            saveImagW = w;
            saveImagH = h;
        }

        private void fillBuff(System.IO.Stream s, ref byte[] buffer, int streamOffset)
        {
            s.Seek(streamOffset, 0);

            for (int i = 0; i < buffer.Length; i++)
            {
                buffer[i] = (byte)System.Convert.ToByte(s.ReadByte());
            }

            s.Seek(0, 0);
        }
        
        private int getTextureFileOffset(System.IO.Stream s)
        {
            //first 4bytes of texture file is the header length hence the offset for dds
            byte[] headerLength = new byte[4];

            fillBuff(s, ref headerLength, 0);

            return BitConverter.ToInt32(headerLength, 0);
        }

        private void setTextureFlags(System.IO.Stream s)
        {
            byte[] flag4bytes = new byte[4];

            fillBuff(s, ref flag4bytes, 16);

            flags = (int)System.BitConverter.ToInt32(flag4bytes, 0);

            byte[] fadeFlag = new byte[8];

            fillBuff(s,ref fadeFlag, 20);

            fade = new float[2];

            fade[0] = BitConverter.ToSingle(fadeFlag, 0);

            fade[1] = BitConverter.ToSingle(fadeFlag, 4);

            byte[] alphaFlag = new byte[1];
            
            fillBuff(s, ref alphaFlag, 28);

            hasAlpha = BitConverter.ToBoolean(alphaFlag, 0);

        }

        private System.IO.MemoryStream getMemStream(string fileName)
        {
            System.IO.StreamReader sr = new System.IO.StreamReader(fileName, true);
            
            sr.BaseStream.Seek(0, 0);

            setTextureFlags(sr.BaseStream);

            int offset = getTextureFileOffset(sr.BaseStream);
            
            long bufferSize = sr.BaseStream.Length - offset;
            
            byte[] tBuffer = new byte[bufferSize];  

            fillBuff(sr.BaseStream, ref tBuffer, offset);

            sr.Close();

            System.IO.MemoryStream mSr = new System.IO.MemoryStream(tBuffer);

            return mSr;
        }

        private string buildFlagsStr()
        {
            string flagsStr = "";
            int flagVal;
            int i = 0;

            flagVal = (int)Math.Pow(2, i++);
            if ( (flags & flagVal) == flagVal)
                flagsStr += "FADE, ";

            flagVal = (int)Math.Pow(2, i++);
            if ( (flags & flagVal) == flagVal)
                flagsStr += "TRUECOLOR, ";

            flagVal = (int)Math.Pow(2, i++);
            if ( (flags & flagVal) == flagVal)
                flagsStr += "TRILINEAR, ";

            flagVal = (int)Math.Pow(2, i++);
            if ( (flags & flagVal) == flagVal)
                flagsStr += "old3, ";

            flagVal = (int)Math.Pow(2, i++);
            if ( (flags & flagVal) == flagVal)
                flagsStr += "DUAL, ";

            flagVal = (int)Math.Pow(2, i++);
            if ( (flags & flagVal) == flagVal)
                flagsStr += "old5, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "CLAMPS, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "CLAMPT, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "old8, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "MIRRORS, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "MIRRORT, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "REPLACEABLE, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "BUMPMAP, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "REPEATS, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "REPEATT, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "CUBEMAP, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "NOMIP, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "JPEG, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "NODITHER, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "NOCOLL, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "SURFACESLICK, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "SURFACEICY, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "SURFACEBOUNCY, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "BORDER, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "OLDTINT, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "DOUBLEFUSION, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "POINTSAMPLE, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "NORMALMAP, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "SPECINALPHA, ";

            flagVal = (int)Math.Pow(2, i++);
            if ((flags & flagVal) == flagVal)
                flagsStr += "FALLBACKFORCEOPAQUE, ";

            return flagsStr;
        }

        private void fillTxtBx(ImageInformation imInfo)
        {
            string flagStr = buildFlagsStr();

            string fadeStr = " Close Fade: " + fade[0] + "\tFar Fade: " + fade[1];
            
            string alphaStr = hasAlpha ? "Has Alpha" : "No Alpha";

            flagStr = flagStr.Length > 1 ? flagStr : "None";

            textBox1.Text = "\r\n";

            textBox1.Text += " Flags: " + flagStr + "\r\n";

            textBox1.Text += fadeStr + "\r\n";

            textBox1.Text += " Format: " + imInfo.Format + "\t" + alphaStr + "\r\n";

            textBox1.Text += " MipLevels: " + imInfo.MipLevels +  "\tDepth: " + imInfo.Depth + "\r\n";

            textBox1.Text += " Height: " + imInfo.Height + "\tWidth:" + imInfo.Width + "\r\n";

            textBox1.SelectionStart = 0;

        }
        
        private Bitmap getImage(string fileName)
        {
            Microsoft.DirectX.GraphicsStream gs = null;
            Microsoft.DirectX.Direct3D.Texture tx = null;
            try
            {
                this.invertAlpha.Enabled = false;

                System.IO.MemoryStream mSr = getMemStream(fileName);

                mSr.Seek(0, 0);

                tx = TextureLoader.FromStream(device, mSr);

                mSr.Seek(0, 0);

                ImageInformation imInfo = TextureLoader.ImageInformationFromStream(mSr);

                mSr.Close();

                fillTxtBx(imInfo);

                gs = TextureLoader.SaveToStream(ImageFileFormat.Dib, tx);

                Bitmap btmap = new Bitmap(gs);

                if (hasAlpha)
                {
                    aChk.Enabled = true;
                    invertAlpha.Enabled = true;
                    fixBitmapAlpha(gs, btmap, imInfo);
                }
                
                return btmap;
            }
            catch (Exception e)
            {
                if (gs != null)
                {
                    gs.Close();

                    gs.Dispose();
                }
                if(tx != null)
                    tx.Dispose();

                if (pp != null)
                {
                    pp = null;

                    pp = new PresentParameters();

                    pp.Windowed = true;

                    pp.SwapEffect = SwapEffect.Copy;
                }

                if (device != null)
                {
                    device.Dispose();

                    device = null;

                    device = new Device(0, DeviceType.Hardware, this, CreateFlags.HardwareVertexProcessing, pp);
                }
                
                return null;
            }
        }

        private void fixBitmapAlpha(Microsoft.DirectX.GraphicsStream gs, Bitmap btmap, ImageInformation imInfo)
        {

            btmap.MakeTransparent();

            gs.Seek(0, 0);

            System.Drawing.Imaging.BitmapData bmpData = btmap.LockBits(new Rectangle(0, 0, imInfo.Width, imInfo.Height),
                                                                        System.Drawing.Imaging.ImageLockMode.ReadWrite,
                                                                        btmap.PixelFormat);

            // Get the address of the first line.
            IntPtr ptr = bmpData.Scan0;

            // Declare an array to hold the bytes of the bitmap.
            int bytes = Math.Abs(bmpData.Stride) * btmap.Height;

            int gsLen = (int)gs.Length;

            byte[] rgbaValues = new byte[bytes];

            byte[] gsBytes = new byte[gsLen];

            gs.Read(gsBytes, 0, gsLen);

            int gsLastI = gsLen - 1;

            // Copy the RGB values into the array.
            System.Runtime.InteropServices.Marshal.Copy(ptr, rgbaValues, 0, bytes);
            int i = 0;
            for (int y = 0; y < bmpData.Height; y++)
            {
                for (int x = 0; x < bmpData.Width; x++)
                {
                    byte g = gsBytes[gsLastI - i++],
                         b = gsBytes[gsLastI - i++],
                         a = gsBytes[gsLastI - i++],
                         r = gsBytes[gsLastI - i++];
                    if (invertAlpha.Checked)
                        a = (byte)(255 - a);

                    Color pClr = Color.FromArgb(a, r, g, b);

                    System.Runtime.InteropServices.Marshal.WriteInt32(bmpData.Scan0, (bmpData.Stride * y) + (4 * x), pClr.ToArgb());
                }

            }

            btmap.UnlockBits(bmpData);

            btmap.RotateFlip(RotateFlipType.Rotate180FlipY);
        }

        private void invert_Alpha()
        {

            System.Drawing.Imaging.BitmapData bmpData = mImage.LockBits(new Rectangle(0, 0, mImage.Width, mImage.Height),
                                                                        System.Drawing.Imaging.ImageLockMode.ReadWrite,
                                                                        mImage.PixelFormat);

            // Get the address of the first line.
            IntPtr ptr = bmpData.Scan0;

            // Declare an array to hold the bytes of the bitmap.
            int bytes = Math.Abs(bmpData.Stride) * mImage.Height;

            byte[] rgbaValues = new byte[bytes];

            // Copy the RGB values into the array.
            System.Runtime.InteropServices.Marshal.Copy(ptr, rgbaValues, 0, bytes);

            for (int i = 3; i < rgbaValues.Length; i+=4)
            {
                rgbaValues[i] = (byte)(255 - rgbaValues[i]);
            }

            System.Runtime.InteropServices.Marshal.Copy(rgbaValues,0, ptr, bytes);
            
            mImage.UnlockBits(bmpData);

            pictureBox1.Image = mImage;
        }

        private void viewDDs(string fileName)
        {
            mImage = getImage(fileName);

            int w = 300;

            int h = 300;

            this.Text = "TexureViewr: " + fileName;

            if (mImage.Width > w)
                w = mImage.Width + 100;
            if (mImage.Height > h)
                h = mImage.Height;

            this.Size = new Size( w , h + 225);
            
            pictureBox1.Image = mImage;

            if (hasAlpha && !turnAlphaOn)
            {
                aChk.Checked = false;
                fixDisplay();
            }

            this.Refresh();
        }

        public void saveImage(string fileName)
        {
            string saveFileName = saveImagePath + @"\" + System.IO.Path.GetFileNameWithoutExtension(fileName) + ".jpg";

            if (!System.IO.File.Exists(saveFileName))
            {
                mImage = getImage(fileName);

                if (mImage != null)
                {
                    mImage = new Bitmap(mImage, new Size(saveImagW, saveImagH));                                    

                    mImage.Save(saveFileName, System.Drawing.Imaging.ImageFormat.Jpeg);
                }
            }
        }

        private void fixDisplay()
        {
            Bitmap b = new Bitmap(mImage.Width, mImage.Height);
            Graphics g = Graphics.FromImage(b);
            System.Drawing.Imaging.ColorMatrix mCM = new System.Drawing.Imaging.ColorMatrix();
            
            // w
            mCM.Matrix44 = 1.00f;
            
            if (!rChk.Checked)
                mCM.Matrix00 = 0.00f;

            if (!gChk.Checked)
                mCM.Matrix11 = 0.00f;

            if (!bChk.Checked)
                mCM.Matrix22 = 0.00f;

            //turn alpha off by adding 1 to alpha value
            if (!aChk.Checked)
            {
                mCM.Matrix43 = 1.00f;
                invertAlpha.Enabled = false;
            }
            else
            {
                invertAlpha.Enabled = true;
            }

            System.Drawing.Imaging.ImageAttributes attr = new System.Drawing.Imaging.ImageAttributes();


            attr.SetColorMatrix(mCM);

            g.DrawImage(mImage, new Rectangle(0, 0, mImage.Width, mImage.Height), 
                                              0, 0, mImage.Width, mImage.Height, 
                                              System.Drawing.GraphicsUnit.Pixel,
                                              attr);
            attr.Dispose();

            g.Dispose();

            pictureBox1.Image = b;
        }

        private void chk_CheckedChanged(object sender, EventArgs e)
        {
            fixDisplay();   
        }

        private void invertAlpha_CheckedChanged(object sender, EventArgs e)
        {
            if(aChk.Checked)
                invert_Alpha();
        }
    }
}
