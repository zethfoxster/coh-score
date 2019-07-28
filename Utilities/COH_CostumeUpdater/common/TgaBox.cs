using System;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using System.IO;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.common
{
    public partial class TgaBox : Form
    {
        private Bitmap mImage;

        public TgaBox(string path, bool turnAlphaOff)
        {
            InitializeComponent();
            this.SuspendLayout();
            this.mImage = getBitmap(path);
            this.pictureBox1.Image = mImage;
            this.Size = new Size(mImage.Width, mImage.Height + this.rgbaPanel.Height);
            this.ResumeLayout(false);
            this.Text = System.IO.Path.GetFileName(path) + " " + getImageSize();
            if (turnAlphaOff)
            {
                this.aChk.Checked = false;
                fixDisplay();
            }
        }

        public string getImageSize()
        {
            string imageSize = mImage.Size.ToString();
            return imageSize;
        }

        private Bitmap getBitmap(string path)
        {
            PresentParameters pp = new PresentParameters();
            pp.Windowed = true;
            pp.SwapEffect = SwapEffect.Copy;
            Device device = new Device(0, DeviceType.Hardware, this, CreateFlags.HardwareVertexProcessing, pp);
            Microsoft.DirectX.Direct3D.Texture tx = TextureLoader.FromFile(device, path);

            Microsoft.DirectX.GraphicsStream gs = TextureLoader.SaveToStream(ImageFileFormat.Dib, tx);
            ImageInformation imInfo = TextureLoader.ImageInformationFromStream(gs);

            Bitmap btmap = new Bitmap(gs);

            btmap.MakeTransparent();

            gs.Seek(0, 0);

            System.Drawing.Imaging.BitmapData bmpData = btmap.LockBits(new Rectangle(0, 0, btmap.Width, btmap.Height),
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

            return btmap;
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
