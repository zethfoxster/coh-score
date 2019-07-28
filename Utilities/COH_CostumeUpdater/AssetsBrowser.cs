using System;
using System.IO;

using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater
{
    public partial class AssetsBrowser : Panel
        //Form
    {
        public AssetsBrowser()
        {
            InitializeComponent();
            build();
        }

        private string[] getImageList(string path)
        {
            return Directory.GetFiles(path, "*.jpg");
        }

        private void build()
        {
            string[] viml = getImageList(@"N:\Web\Internal Website\Catalog\villains\image2");
            //string[] imL = getImageList(@"N:\Web\Internal Website\Catalog\objects\images");
            int x = 10, y = 10;
            int maxX = (int)(Width / 148); 
            int pCount = 1;
            this.SuspendLayout();
            foreach (string str in viml)
            {
                pCount++;
                PicBxNTitle pbt = new PicBxNTitle(str, new string[] { "a", "b", "c" }, new string[] { "a", "b", "c" });
                pbt.Location = new Point(x, y);
                this.Controls.Add(pbt);
                x += pbt.Width + 10;
                if (pCount > maxX)
                {
                    x = 10;
                    y += pbt.Height + 10;
                    pCount = 1;
                }
            }
            /*
            foreach (string str in imL)
            {
                pCount++;
                PicBxNTitle pbt = new PicBxNTitle(str, new string[] {"a", "b", "c" }, new string[] {"a", "b", "c"});
                pbt.Location = new Point(x, y);
                this.Controls.Add(pbt);
                x += pbt.Width + 10;
                if (pCount > maxX)
                {
                    x = 10;
                    y += pbt.Height + 10;
                    pCount = 1;
                }                
            }*/
            this.ResumeLayout(false);
        }
        void AssetsBrowser_Resize(object sender, System.EventArgs e)
        {
        }
    }
}
