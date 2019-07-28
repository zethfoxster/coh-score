using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes; 


namespace findwrl
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length > 0)
            {
                string arg = (string)args[0];

                string []results = findwrl(Path.GetFileName(arg));

                if (results.Length > 0)
                {
                    ShowSelectedInExplorer.FilesOrFolders(results);
                }
            }
        }

        public static string[] findwrl(string maxFileName)
        {
            string[] pr = { "-", "\\", "|", "/" };//\player_library
            string[] files = Directory.GetFiles(@"c:\game\src", "*.wrl", System.IO.SearchOption.AllDirectories);
            StreamReader sr = null;
            System.Collections.ArrayList alist = new System.Collections.ArrayList();
            int i = 0;
            foreach(string path in files)
            {
                Console.Clear();
                Console.Write(maxFileName +" "+pr[i++ % 4]);
                
                sr = new StreamReader(path);
                string line;
                while((line = sr.ReadLine())!= null)
                {
                    if(line.ToLower().Contains("# MAX File:".ToLower()))
                    {
                        string wrlMaxSrcFile = line.ToLower().Replace("# MAX File:".ToLower(), "").Trim();
                        wrlMaxSrcFile = wrlMaxSrcFile.Substring(0, wrlMaxSrcFile.IndexOf(".max") + 4);
                        if (maxFileName.ToLower().Equals(wrlMaxSrcFile))
                        {
                            sr.Close();
                            string filePath = Path.GetDirectoryName(path);
                            string[] nfiles = Directory.GetFiles(filePath, "*.wrl", System.IO.SearchOption.AllDirectories);
                            foreach (string pth in nfiles)
                            {
                                Console.Clear();
                                Console.Write(maxFileName + " " + pr[i++ % 4]);
                                sr = new StreamReader(pth);
                                while ((line = sr.ReadLine()) != null)
                                {
                                    if (line.ToLower().Contains("# MAX File:".ToLower()))
                                    {
                                        string srcFile = line.ToLower().Replace("# MAX File:".ToLower(), "").Trim();
                                        srcFile = srcFile.Substring(0, srcFile.IndexOf(".max") + 4);
                                        if (maxFileName.ToLower().Equals(srcFile))
                                        {
                                            alist.Add(pth);
                                            sr.Close();
                                            break;
                                        }
                                    }
                                }
                            }
                            if (sr != null)
                                sr.Close();

                            return ((string[])alist.ToArray(typeof(string)));

                        
                        }
                        if (sr != null)
                            sr.Close();
                        break;
                    }
                }
            }

            return ((string[])alist.ToArray(typeof(string)));
        }
    }
}

