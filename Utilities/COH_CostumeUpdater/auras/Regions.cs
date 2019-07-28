using System;
using System.Collections;
using System.IO;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater
{
    public class Regions
    {
        string header = "// Order matters\r\n//-----------------------------\r\n//\t1. Head\r\n//\t2. Upper\r\n//\t3. Lower\r\n\r\n";
        string sectionStart = "Region";
        string sectionEnd = "End";
        string regionHeader = "Region";
        string endHeader = "End";
        string nameHeader = "\tName";
        string displayNameHeader = "\tDisplayName";
        string includeHeader = "\tInclude";
        string keyHeader = "\tKey";
        public string fileName;
        public string filePath;
        public string fileType;
        private string root;
        
        public ArrayList regionSections;
        public ArrayList nonRegionedStrings;

        public Regions(string fileFullName)
        {
            fileName = Path.GetFileName(fileFullName);
            filePath = Path.GetFullPath(fileFullName);
            setFileType();
            regionSections = new ArrayList();
            nonRegionedStrings = new ArrayList();
            root = @"C:\game\data\";
            
            if (filePath.ToLower().Contains(@"c:\gamefix2"))
                root = @"C:\gamefix2\data\";

            else if (filePath.ToLower().Contains(@"c:\gamefix"))
                root = @"C:\gamefix\data\";

            initialize();
        }
        private void initialize()
        {
            try
            {
                StreamReader sr = new StreamReader(filePath);
                String line;
                Region r = null;
                while ((line = sr.ReadLine()) != null)
                {
                    if (line.StartsWith(sectionStart))
                    {
                        r = new Region(fileType, root);
                    }
                    else if (line.StartsWith(nameHeader))
                    {
                        int startIndex = line.IndexOf('\"');
                        int endIndex = line.IndexOf('\"', startIndex + 1);
                        int nameLen = endIndex - startIndex;
                        r.name = line.Substring(startIndex + 1, nameLen - 1);
                    }
                    else if (line.StartsWith(displayNameHeader))
                    {
                        int startIndex = line.IndexOf('\"');
                        int endIndex = line.IndexOf('\"', startIndex + 1);
                        int nameLen = endIndex - startIndex;
                        r.displayName = line.Substring(startIndex + 1, nameLen - 1);
                    }
                    else if (line.StartsWith(keyHeader))
                    {
                        int startIndex = line.IndexOf('\"');
                        int endIndex = line.IndexOf('\"', startIndex + 1);
                        int nameLen = endIndex - startIndex;
                        r.key = line.Substring(startIndex + 1, nameLen - 1);
                        r.hasKey = true;
                    }
                    else if (line.StartsWith(includeHeader))
                    {
                        r.addInclude(line);
                    }
                    else if (line.StartsWith(sectionEnd))
                    {
                        regionSections.Add(r);
                    }
                    else if (line.StartsWith("Include"))
                    {
                        nonRegionedStrings.Add(line);
                    }
                }
                sr.Close();
            }
            catch (Exception e)
            {
                System.Windows.Forms.MessageBox.Show(e.Message);                
            }

        }
        private void setFileType()
        {
            if(filePath.ToLower().Contains("arachnos"))
            {
                if(filePath.ToLower().Contains("male")&& 
                   filePath.ToLower().Contains("soldier"))
                {
                    fileType = "Arachnos_Male_Soldier"; 
                }
                else if (filePath.ToLower().Contains("male") &&
                         filePath.ToLower().Contains("widow"))
                {
                    fileType = "Arachnos_Male_Widow";
                }
                else if (filePath.ToLower().Contains("female") &&
                         filePath.ToLower().Contains("soldier"))
                {
                    fileType = "Arachnos_Female_Soldier";
                }
                else if (filePath.ToLower().Contains("female") &&
                         filePath.ToLower().Contains("widow"))
                {
                    fileType = "Arachnos_Female_Widow";
                }
                else if (filePath.ToLower().Contains("huge") &&
                         filePath.ToLower().Contains("soldier"))
                {
                    fileType = "Arachnos_Huge_Soldier";
                }
                else
                {
                    fileType = "Arachnos_Huge_Widow";
                }
            }
            else if (filePath.ToLower().Contains("huge"))
            {
                fileType = "Huge";
            }
            else if (filePath.ToLower().Contains("female"))
            {
                fileType = "Female";
            }
            else
            {
                fileType = "Male";
            }             
        }
    }

}
