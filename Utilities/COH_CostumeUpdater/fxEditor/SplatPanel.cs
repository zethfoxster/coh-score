using System;
using System.Collections.Generic;
using System.ComponentModel;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.fxEditor
{
    class SplatPanel:Panel
        //Form
    {
        private System.Windows.Forms.PictureBox alpha_picBx;
        
        private System.Windows.Forms.PictureBox rgb_picBx;
        
        private System.Windows.Forms.Button browserBtn;
        
        public System.Windows.Forms.TextBox textureNamePath_txBx;
        
        private System.Windows.Forms.ContextMenu iconCntxMn;
        
        private System.Windows.Forms.MenuItem sendToPhotoShop_menuItem;
        
        private System.Windows.Forms.OpenFileDialog openFileDialog1;
        
        private string rootPath;

        private bool selected;
        
        private Label eventName_label;
        
        private Color bColor;
        
        private System.ComponentModel.IContainer components = null;

        public event EventHandler IsDirtyChanged;
        
        public bool isDirty;
        
        private bool settingCells;

        private CheckBox enable_checkBox;

        public event EventHandler CurrentRowChanged;

        private Event currentRowEvent;

        public SplatPanel(string path)
        {
            this.rgb_picBx = null;
           
            this.alpha_picBx = null;

            selected = false;

            rootPath = common.COH_IO.getRootPath(path);
            
            rootPath = rootPath.EndsWith(@"\") ? rootPath : rootPath + @"\";
            
            InitializeComponent();
            
            this.bColor = this.BackColor;
           
            this.eventName_label.Text = this.Text;

            if (path != null &&
                path.Length > 0 &&
                path.ToLower().EndsWith("tga"))
            {
                setIconImage(path);
                
                settingCells = true;

                textureNamePath_txBx.Tag = path;
                
                textureNamePath_txBx.Text = System.IO.Path.GetFileName(path);

                settingCells = false;
            }
        }

        public void selectSplatPanel()
        {
            selected = true;

            enable_checkBox_CheckedChanged(this.enable_checkBox, new EventArgs());
        }

        public void clearSelection()
        {
            selected = false;

            this.currentRowEvent = null;

            enable_checkBox_CheckedChanged(this.enable_checkBox, new EventArgs());
        }

        public Event CurrentRowEvent
        {
            get
            {
                return currentRowEvent;
            }
            set
            {
                currentRowEvent = value;
            }
        }

        public string getData()
        {
            if (this.textureNamePath_txBx.Text != null &&
                this.textureNamePath_txBx.Text.Length > 0)
            {
                return System.IO.Path.GetFileName(this.textureNamePath_txBx.Text);
            }
            else
            {
                return null;
            }
        }

        public bool isCommented
        {
            get
            {
                return !enable_checkBox.Checked;
            }
        }

        protected void OnCurrentRowChanged(EventArgs e)
        {
            if (CurrentRowChanged != null)
                CurrentRowChanged(this, e);
        }

        void EventSplat_MouseClick(object sender, MouseEventArgs e)
        {
            Event ev = currentRowEvent;

            currentRowEvent = (Event)this.Tag;

            if (currentRowEvent != null && ev != currentRowEvent)
            {
                if (CurrentRowChanged != null)
                    CurrentRowChanged(this, new EventArgs());
            }
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
        
        private void InitializeComponent()//string name)
        {
            this.textureNamePath_txBx = new System.Windows.Forms.TextBox();
            this.browserBtn = new System.Windows.Forms.Button();
            this.rgb_picBx = new System.Windows.Forms.PictureBox();
            this.iconCntxMn = new System.Windows.Forms.ContextMenu();
            this.sendToPhotoShop_menuItem = new System.Windows.Forms.MenuItem();
            this.alpha_picBx = new System.Windows.Forms.PictureBox();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.eventName_label = new System.Windows.Forms.Label();
            this.enable_checkBox = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.rgb_picBx)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.alpha_picBx)).BeginInit();
            this.SuspendLayout();
            // 
            // textureNamePath_txBx
            // 
            this.textureNamePath_txBx.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textureNamePath_txBx.Location = new System.Drawing.Point(111, 33);
            this.textureNamePath_txBx.Name = "textureNamePath_txBx";
            this.textureNamePath_txBx.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.textureNamePath_txBx.Size = new System.Drawing.Size(368, 20);
            this.textureNamePath_txBx.TabIndex = 1;
            this.textureNamePath_txBx.Validated += new System.EventHandler(this.textureNamePath_txBx_Validated);
            this.textureNamePath_txBx.MouseClick += new System.Windows.Forms.MouseEventHandler(this.EventSplat_MouseClick);
            // 
            // browserBtn
            // 
            this.browserBtn.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.browserBtn.Location = new System.Drawing.Point(481, 31);
            this.browserBtn.Name = "browserBtn";
            this.browserBtn.Size = new System.Drawing.Size(45, 23);
            this.browserBtn.TabIndex = 2;
            this.browserBtn.Text = "...";
            this.browserBtn.UseVisualStyleBackColor = true;
            this.browserBtn.Click += new System.EventHandler(this.browserBtn_Click);
            // 
            // rgb_picBx
            // 
            this.rgb_picBx.BackgroundImage = global::COH_CostumeUpdater.Properties.Resources.picBoxBkGrndImg;
            this.rgb_picBx.ContextMenu = this.iconCntxMn;
            this.rgb_picBx.Location = new System.Drawing.Point(5, 5);
            this.rgb_picBx.Name = "rgb_picBx";
            this.rgb_picBx.Size = new System.Drawing.Size(48, 48);
            this.rgb_picBx.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.rgb_picBx.TabIndex = 3;
            this.rgb_picBx.TabStop = false;
            this.rgb_picBx.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.tgaIconPicBx_MouseDoubleClick);
            this.rgb_picBx.MouseClick += new System.Windows.Forms.MouseEventHandler(this.EventSplat_MouseClick);
            // 
            // iconCntxMn
            // 
            this.iconCntxMn.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.sendToPhotoShop_menuItem});
            // 
            // sendToPhotoShop_menuItem
            // 
            this.sendToPhotoShop_menuItem.Index = 0;
            this.sendToPhotoShop_menuItem.Text = "Send To Photoshop";
            this.sendToPhotoShop_menuItem.Click += new System.EventHandler(this.sendToPhotoShop_menuItem_Click);
            // 
            // alpha_picBx
            // 
            this.alpha_picBx.BackgroundImage = global::COH_CostumeUpdater.Properties.Resources.picBoxBkGrndImg;
            this.alpha_picBx.InitialImage = null;
            this.alpha_picBx.Location = new System.Drawing.Point(59, 5);
            this.alpha_picBx.Name = "alpha_picBx";
            this.alpha_picBx.Size = new System.Drawing.Size(48, 48);
            this.alpha_picBx.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this.alpha_picBx.TabIndex = 3;
            this.alpha_picBx.TabStop = false;
            this.alpha_picBx.MouseClick += new System.Windows.Forms.MouseEventHandler(this.EventSplat_MouseClick);
            // 
            // openFileDialog1
            // 
            this.openFileDialog1.Filter = "TGA file (*.tga)|*.tga";
            // 
            // eventName_label
            // 
            this.eventName_label.AutoSize = true;
            this.eventName_label.Location = new System.Drawing.Point(114, 14);
            this.eventName_label.Name = "eventName_label";
            this.eventName_label.Size = new System.Drawing.Size(0, 13);
            this.eventName_label.TabIndex = 4;
            this.eventName_label.MouseClick += new System.Windows.Forms.MouseEventHandler(this.EventSplat_MouseClick);
            // 
            // enable_checkBox
            // 
            this.enable_checkBox.AutoSize = true;
            this.enable_checkBox.Location = new System.Drawing.Point(111, 5);
            this.enable_checkBox.Name = "enable_checkBox";
            this.enable_checkBox.Size = new System.Drawing.Size(59, 17);
            this.enable_checkBox.TabIndex = 5;
            this.enable_checkBox.Text = "Enable";
            this.enable_checkBox.UseVisualStyleBackColor = true;
            this.enable_checkBox.CheckedChanged += new System.EventHandler(this.enable_checkBox_CheckedChanged);
            // 
            // SplatPanel
            // 
            this.ClientSize = new System.Drawing.Size(535, 59);
            this.Controls.Add(this.enable_checkBox);
            this.Controls.Add(this.eventName_label);
            this.Controls.Add(this.browserBtn);
            this.Controls.Add(this.textureNamePath_txBx);
            this.Controls.Add(this.alpha_picBx);
            this.Controls.Add(this.rgb_picBx);
            this.Name = "SplatPanel";
            this.MouseClick += new System.Windows.Forms.MouseEventHandler(this.EventSplat_MouseClick);
            ((System.ComponentModel.ISupportInitialize)(this.rgb_picBx)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.alpha_picBx)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        private void textureNamePath_txBx_Validated(object sender, EventArgs e)
        {
            if (!settingCells &&  !isDirty)
            {
                Event ev = (Event)this.Tag;

                isDirty = true;

                if (ev.isDirty != isDirty)
                {
                    ev.isDirty = true;
                    ev.parent.isDirty = true;
                    ev.parent.parent.isDirty = true;

                    if (IsDirtyChanged != null)
                        IsDirtyChanged(this, e);
                }
            }
        }

        private void browserBtn_Click(object sender, EventArgs e)
        {
            if (System.IO.File.Exists(textureNamePath_txBx.Text))
            {
                openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName(textureNamePath_txBx.Text);
            }
            else if (textureNamePath_txBx.Tag != null &&
                System.IO.File.Exists((string)textureNamePath_txBx.Tag))
            {
                openFileDialog1.InitialDirectory = System.IO.Path.GetDirectoryName((string)textureNamePath_txBx.Tag);
            }
            DialogResult dr = openFileDialog1.ShowDialog();

            if (dr == DialogResult.OK)
            {
                textureNamePath_txBx.Tag = openFileDialog1.FileName;
                textureNamePath_txBx.Text = System.IO.Path.GetFileName(openFileDialog1.FileName);
                addToTextureDic(openFileDialog1.FileName);

                if (rgb_picBx != null)
                    setIconImage(openFileDialog1.FileName);
            }

        }

        public void setCommentColor(bool isCommented)
        {
            this.enable_checkBox.Checked = !isCommented;
            enable_checkBox_CheckedChanged(this.enable_checkBox, new EventArgs());
        }

        public void addToTextureDic(string tgaFileName)
        {

            COH_CostumeUpdaterForm form = this.FindForm() as COH_CostumeUpdaterForm;

            if (form != null)
            {
                if (!form.tgaFilesDictionary.ContainsKey(tgaFileName))
                {
                    form.tgaFilesDictionary[tgaFileName] = System.IO.Path.GetFileName(tgaFileName);
                    form.tgaFilesDictionary = common.COH_IO.sortDictionaryKeys(form.tgaFilesDictionary);
                }
            }
        }

        private void setIconImage(string path)
        {
            try
            {
                PresentParameters pp = new PresentParameters();
                pp.Windowed = true;
                pp.SwapEffect = SwapEffect.Copy;
                Device device = new Device(0, DeviceType.Hardware, this.rgb_picBx, CreateFlags.HardwareVertexProcessing, pp);
                Microsoft.DirectX.Direct3D.Texture tx = TextureLoader.FromFile(device, path);
                Microsoft.DirectX.GraphicsStream gs = TextureLoader.SaveToStream(ImageFileFormat.Dib, tx);

                Bitmap RGB_bitmap = new Bitmap(gs);
                Bitmap alpha_bitmap = new Bitmap(gs);

                rgb_picBx.Image = RGB_bitmap;

                //alpha_bitmap.MakeTransparent();
                gs.Seek(0, 0);

                System.Drawing.Imaging.BitmapData bmpData = alpha_bitmap.LockBits(new Rectangle(0, 0, alpha_bitmap.Width, alpha_bitmap.Height),
                                                                            System.Drawing.Imaging.ImageLockMode.ReadWrite,
                                                                            alpha_bitmap.PixelFormat);

                // Get the address of the first line.
                IntPtr ptr = bmpData.Scan0;

                // Declare an array to hold the bytes of the bitmap.
                int bytes = Math.Abs(bmpData.Stride) * alpha_bitmap.Height;

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

                        Color pClr = Color.FromArgb(255, a, a, a);

                        System.Runtime.InteropServices.Marshal.WriteInt32(bmpData.Scan0, (bmpData.Stride * y) + (4 * x), pClr.ToArgb());
                    }

                }

                alpha_bitmap.UnlockBits(bmpData);

                alpha_bitmap.RotateFlip(RotateFlipType.Rotate180FlipY);

                alpha_picBx.Image = alpha_bitmap;
                gs.Dispose();
                tx.Dispose();
                device.Dispose();
            }
            catch (Exception ex)
            { }
        }

        private void sendToPhotoShop_menuItem_Click(object sender, System.EventArgs e)
        {
            string path = (string)textureNamePath_txBx.Tag;
            if (System.IO.File.Exists(path))
            {
                System.Diagnostics.Process.Start(path);
            }
        }
        
        private void tgaIconPicBx_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            try
            {
                viewTga((string)textureNamePath_txBx.Tag, true);
            }
            catch (Exception ex)
            {
            }
        }

        private string viewTga(string path, bool show)
        {
            string results = "";
            
            common.TgaBox tbx = new common.TgaBox(path, true);

            if (show)
            {
                tbx.ShowDialog();
            }
            else
            {
                results = tbx.getImageSize();
            }

            return results;
        }

        private void enable_checkBox_CheckedChanged(object sender, EventArgs e)
        {
            if (!enable_checkBox.Checked)
            {
                if (selected)
                    this.BackColor = Color.FromArgb(Math.Max(0, (int)Color.Khaki.R - 50),
                                            Math.Max(0, (int)Color.Khaki.G),
                                            Math.Max(0, (int)Color.Khaki.B - 50));
                else
                    this.BackColor = Color.FromArgb(Math.Max(0, (int)this.bColor.R - 50),
                                            Math.Max(0, (int)this.bColor.G),
                                            Math.Max(0, (int)this.bColor.B - 50));

            }
            else
            {
                if (selected)
                    this.BackColor = Color.Khaki;
                else
                    this.BackColor = bColor;
            }
        }
    }
}
