using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater
{
    public partial class PicBxNTitle : Panel
        //Form
    {
        private string maxPath;
        private string maxFileName;
        private string[] sortedFilesPath;
        private string[] namesFromSortedTgaFilesPath;

        public PicBxNTitle(string path, string[] sortedPaths, string[] sortedNamesFromPath)
        {
            InitializeComponent();
            sortedFilesPath = sortedPaths;
            namesFromSortedTgaFilesPath = sortedNamesFromPath;
            setPic(path);
            
        }
        public void setPic(string path)
        {
            this.maxFileName = System.IO.Path.GetFileNameWithoutExtension(path) + ".max";
            this.iconLabel.Text = maxFileName;
            this.iconPicBx.Image = new Bitmap(path);
            
            maxPath = findMaxPath();
        }


        private string findMaxPath()
        {
            int pInd = 0;//binarySearch(namesFromSortedTgaFilesPath, 
               //maxFileName, 0, namesFromSortedTgaFilesPath.Length);

            if (pInd > 0)
            {
                return sortedFilesPath[pInd];
            }
            else
            {
                return null;
            }
        }

        private int binarySearch(string[] files, string item, int low, int high)
        {
            System.Windows.Forms.Application.DoEvents();
            if (high < low)
                return -1; // not found

            int mid = low + (high - low) / 2;

            string midStrlc = files[mid].ToLower();

            if (midStrlc.CompareTo(item.ToLower()) > 0)
            {
                return binarySearch(files, item, low, mid - 1);
            }
            else if (midStrlc.CompareTo(item.ToLower()) < 0)
            {
                return binarySearch(files, item, mid + 1, high);
            }
            else
            {
                return mid; // found
            }

        }
        
        private void iconPicBx_Click(object sender, EventArgs e)
        {
            if(maxPath != null)
                System.Diagnostics.Process.Start("explorer.exe", maxPath);
        }
    }
}
