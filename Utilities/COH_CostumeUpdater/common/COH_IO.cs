using System;
using System.IO;
using System.Drawing;
using Microsoft.DirectX;
using Microsoft.DirectX.Direct3D;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace COH_CostumeUpdater.common
{   
    class COH_IO
    {
        public static void SetClipBoardDataObject(Object dataObj)
        {
            if (dataObj.GetType() == typeof(ArrayList))
            {
                string dataStr = "";

                ArrayList strData = (ArrayList)dataObj;

                foreach (string str in strData)
                {
                    dataStr += str + "\r\n";
                }
                if (dataStr.Length > 0)
                    Clipboard.SetDataObject(dataStr);
            }
            else
            {
                Clipboard.SetDataObject(dataObj, true);
            }            
        }

        public static string fixInnerCamelCase(string line, string secName)
        {
            string fixedLine = "";

            char[] lineChars = line.ToCharArray();

            char[] secChars = secName.ToCharArray();

            if (line.ToLower().Contains((secName).ToLower()))
            {
                int secInd = line.ToLower().IndexOf(secName.Trim().ToLower());

                for (int i = 0; i < secName.Length; i++)
                {
                    char c = lineChars[secInd + i];
                    char ch = secChars[i];
                    {
                        if (ch.ToString().ToLower().Equals(c.ToString().ToLower()))
                        {
                            lineChars[secInd+i] = ch;
                        }
                    }
                }
            }
            foreach (char m in lineChars)
            {
                fixedLine += m;
            }

            return fixedLine.Trim();
        }

        public static string fixCamelCase(string lineStartingWsecName, string secName)
        {
            string fixedLine = "";

            char[] lineChars = lineStartingWsecName.ToCharArray();

            char[] secChars = secName.ToCharArray();

            if (lineStartingWsecName.ToLower().StartsWith((secName).ToLower()))
            {
                for (int i = 0; i < secName.Length; i++)
                {
                    char c = lineChars[i];
                    char ch = secChars[i];
                    {
                        if (ch.ToString().ToLower().Equals(c.ToString().ToLower()))
                        {
                            lineChars[i] = ch;
                        }
                    }
                }
            }
            foreach (char m in lineChars)
            {
                fixedLine += m;
            }

            return fixedLine.Trim();
        }

        public static  ArrayList GetClipBoardStringDataObject()
        {
            ArrayList strData = null;

            // IDataObject to hold the data returned from the clipboard.
            IDataObject iData = Clipboard.GetDataObject();

            if (iData.GetDataPresent(DataFormats.Text))
            {
                strData = new ArrayList();

                string [] dataStr = ((String)iData.GetData(DataFormats.Text)).Replace("\r","").Split('\n');

                foreach (string str in dataStr)
                    strData.Add(str);
            }

            return strData;
        }

        public static bool contains(ArrayList alist, string testStr)
        {
            foreach (object obj in alist)
            {
                if (((string)obj).Contains(testStr))
                    return true;
            }
            return false;
        }

        public static void fillList(System.Collections.ArrayList strArrayList, string charTypeFileName)
        {
            StreamReader sr = new StreamReader(charTypeFileName);
            String line;
            
            while ((line = sr.ReadLine()) != null)
            {
                strArrayList.Add(line);
            }
            sr.Close();
        }

        public static void fillList(System.Collections.ArrayList strArrayList, string charTypeFileName, bool trimSpace)
        {
            StreamReader sr = new StreamReader(charTypeFileName);
            String line;

            while ((line = sr.ReadLine()) != null)
            {
                if(trimSpace)
                    line = line.Trim();

                strArrayList.Add(line);
            }
            sr.Close();
        }

        public static Bitmap getTgaImage(string path, bool alpha)
        {
            try
            {
                Form form = new Form();
                PresentParameters pp = new PresentParameters();
                pp.Windowed = true;
                pp.SwapEffect = SwapEffect.Copy;
                Device device = new Device(0, DeviceType.Hardware, form, CreateFlags.HardwareVertexProcessing, pp);
                Microsoft.DirectX.Direct3D.Texture tx = TextureLoader.FromFile(device, path);
                Microsoft.DirectX.GraphicsStream gs = TextureLoader.SaveToStream(ImageFileFormat.Dib, tx);

                Bitmap RGB_bitmap = new Bitmap(gs);
                Bitmap alpha_bitmap = new Bitmap(gs);

                if (!alpha)
                {
                    gs.Dispose();
                    tx.Dispose();
                    device.Dispose();

                    return RGB_bitmap;
                }
                else
                {

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


                    gs.Dispose();
                    tx.Dispose();
                    device.Dispose();

                    return alpha_bitmap;
                }
            }
            catch (Exception ex)
            { return null; }
        }

        public static string writeDistFile(System.Collections.ArrayList strArrayList, string distFilePath)
        {
            try
            {
                FileInfo fi = new FileInfo(distFilePath);

                bool ro = fi.Exists && fi.IsReadOnly;
                if (ro)
                    fi.Attributes = fi.Attributes ^ FileAttributes.ReadOnly;

                StreamWriter sw = new StreamWriter(distFilePath);
                foreach (object line in strArrayList)
                {
                    sw.WriteLine(line);
                }
                sw.Close();

                if (ro)
                    fi.Attributes = fi.Attributes | FileAttributes.ReadOnly;

                return "Updated for \"" + distFilePath + "\"\r\n";
            }
            catch (Exception ex)
            {
                return "Failed to Update for \"" + distFilePath + "\"\r\n" + ex.Message + "\r\n";
            }
        }
        
        public static string removeExtraSpaceBetweenWords(string str)
        {
            if (str.Length > 1)
            {
                string results = "";
                char[] strChars = str.ToCharArray();
                int consecutiveSpaceCount = 0;
                char oldChar = strChars[0];
                foreach (char c in str.ToCharArray())
                {
                    if (c != ' ')
                    {
                        results += c;
                        consecutiveSpaceCount = 0;
                    }
                    else
                    {
                        if (consecutiveSpaceCount == 0)
                        {
                            results += c;
                            consecutiveSpaceCount++;
                        }
                    }
                }
                return results;
            }
            else
                return str;
        }

        public static Dictionary<string, string> sortDictionaryKeys(Dictionary<string, string> dic)
        {
            System.Collections.Generic.Dictionary<string, string> sortedDic = new System.Collections.Generic.Dictionary<string,string>();

            foreach (KeyValuePair<string, string> kvp in dic.OrderBy(a => a.Key, new fxEditor.CompareFileNameStr()))
            {
                sortedDic[kvp.Key] = kvp.Value;   
            }

            return sortedDic;
        }
        public static System.Collections.Generic.Dictionary<string, string> getFilesDictionary(string searchLocation, string ext)
        {
            string[] files = Directory.GetFiles(searchLocation, ext, System.IO.SearchOption.AllDirectories);

            System.Collections.Generic.Dictionary<string, string> filesDictionary =
            new System.Collections.Generic.Dictionary<string, string>();

            int i = 0;

            foreach (string p in files)
            {
                string nP = System.IO.Path.GetFileName(p) + "," + p;
                files[i++] = nP;
                System.Windows.Forms.Application.DoEvents();
            }

            Array.Sort(files);

            foreach (string p in files)
            {
                string[] path = p.Split(','); 
                string key = path[1];
                string value = System.IO.Path.GetFileName(path[1]);
                try
                {
                    filesDictionary.Add(key, value);
                }
                catch (Exception ex) { }
                System.Windows.Forms.Application.DoEvents();
            }
            
            return filesDictionary;
        }

        //slow search O(n)
        public static string findTgaPath(string path, string root)
        {
            return findFxPath(path, root + @"src\Texture_Library", "*.tga");
        }

        public static string findFxPath(string path, string searchLocation, string ext)
        {
            string nPath = "";

            string[] files = Directory.GetFiles(searchLocation, ext, System.IO.SearchOption.AllDirectories);

            foreach (string p in files)
            {
                if (p.ToLower().Contains(path.ToLower()))
                {
                    nPath = p;
                    break;
                }
            }

            if (nPath == "")
            {
                if (searchLocation.EndsWith(@"\"))
                    nPath = searchLocation + path;
                else
                    nPath = searchLocation + @"\" + path;
            }
            return nPath;
        }
        
        //faster search given a sorted search list
        public static string findTGAPathFast(string tgaFileNameWext, string[] sortedTgaFilesPath, string [] tgaNamesFromSortedTgaFilesPath)
        {
            int pInd = binarySearch(tgaNamesFromSortedTgaFilesPath, tgaFileNameWext, 0, tgaNamesFromSortedTgaFilesPath.Length);

            if (pInd > 0)
            {
                return sortedTgaFilesPath[pInd];
            }
            else
            {
                return null;
            }
        }

        public static int binarySearch(string[] files, string item, int low, int high)
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
        
        public static string getRootPath(string filePath)
        {
            string pathRoot = @"C:\game\data\";

            if (filePath.ToLower().Contains("gamefix2"))
                pathRoot = @"C:\gamefix2\data\";

            else if (filePath.ToLower().Contains("gamefix"))
                pathRoot = @"C:\gamefix\data\";

            return pathRoot;
        }
        
        public static void setImageIndex(ref System.Windows.Forms.TreeNode tn, string str)
        {
            if (str.Replace("\t", "").ToLower().StartsWith("include"))
            {
                tn.ImageIndex = 9;
            }
            else if (str.ToLower().Contains("boneset"))
            {
                tn.ImageIndex = 11;
            }
            else if (str.ToLower().Contains("geoname"))
            {
                tn.ImageIndex = 2;
            }
            else if (str.ToLower().Contains("bodypart"))
            {
                tn.ImageIndex = 2;
            }
            else if (str.ToLower().Contains("displayname"))
            {
                tn.ImageIndex = 2;
            }
            else if (str.ToLower().Contains("name"))
            {
                tn.ImageIndex = 2;
            }
            else if (str.ToLower().Contains("tag"))
            {
                tn.ImageIndex = 7;
            }
            else if (str.ToLower().Contains("key"))
            {
                tn.ImageIndex = 6;
            }
            else if (str.ToLower().Contains("mask"))
            {
                tn.ImageIndex = 8;
            }
            else if (str.ToLower().Contains("geo"))
            {
                tn.ImageIndex = 3;
            }
            else if (str.ToLower().Contains("fx"))
            {
                tn.ImageIndex = 4;
            }
            else if (str.ToLower().Contains("texture"))
            {
                tn.ImageIndex = 5;
            }
            else if (tn.Text.ToLower().Contains("devonly 1"))
            {
                tn.ImageIndex = 10;
            }
            else
            {
                tn.ImageIndex = 100;
            }

              tn.SelectedImageIndex = tn.ImageIndex;
        }

        public static string stripWS_Tabs_newLine_Qts(string str, bool removeQuotes)
        {
            string nStr =  str.Replace("\t", "").Replace("\r", "").Replace("\n", "").Trim();
            if (removeQuotes)
                nStr = nStr.Replace("\"", "");

            return nStr;
        }

        public static string getFiledStr(string str)
        {
            int start, len;
            str = str.Replace("\r", "").Replace("\n","");

            if (str.Contains("\""))
            {
                start = str.IndexOf("\"") + 1;
                len = str.LastIndexOf("\"") - start;
                
            }
            else
            {
                start = str.IndexOf(" ") + 1;
                len = str.Length - start;
            }

            return str.Substring(start, len);
        }
    }
}
