using System;
using P4API;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace COH_CostumeUpdater.assetsMangement
{
    static class CohAssetMang
    {      
        public static string getCurrentUser()
        {
            return executeGimmeCmd("-whoami \r\n");
        }

        public static string getCurrentVersion()
        {
            return "";
        }

        public static string addP4(string path)
        {
            if (!isP4file(path))
            {
                string cmd = "add";
                //string [] args = {"-m1", "-s", "submitted", "//depot/dev/..."};
                string[] args = { path };
                string results = executeP4Cmd(cmd, args);
                return results;
            }
            else
            {
                return "File is already reside in the P4 depot!";
            }
        }

        public static string p4GetWorkspace()
        {
            string cmd = "where";
            string[] args = { " " };
            string results = executeP4Cmd(cmd, args);
            string[] rSplit = results.Split("\r\n".ToCharArray());
         
            foreach (string s in rSplit)
            {
                if (s.Contains("/gamefix2/"))
                {
                    return @"C:\gamefix2\";
                }
                else if (s.Contains("/gamefix/"))
                {
                    return @"C:\gamefix\";
                }
                else if (s.Contains("/game/"))
                {
                    return @"C:\game\";
                }
            }

            return null;

        }

        public static bool isP4file(string path)
        {
            string cmd = "fstat";
            string[] args = { path }; 
            string results = executeP4Cmd(cmd, args);

            if (results.Length > 0)
                return true;
            else
                return false;
        }

        public static void p4vDiffDialog(string path)
        {
            string args = string.Format("-cmd \"diffdialog {0}\"", path);
            System.Diagnostics.Process.Start("p4v", args);
        }
        public static void p4vDepotDiff(string path)
        {
            string args = string.Format("-cmd \"prevdiff {0}\"", path);
            System.Diagnostics.Process.Start("p4v",args);
        }

        public static bool isP4Different(string path)
        {
            string cmd = "diff";
            //string [] args = {"-m1", "-s", "submitted", "//depot/dev/..."};
            string[] args = { "-sa", path };
            string results = executeP4Cmd(cmd, args);

            if (results.Length > 0)
                return true;
            else
                return false;
        }

        public static bool isDBfile(string path)
        {
            string dbRoot = @"N:\revisions\";
            string dbPath = path.Replace(@"C:\game\", dbRoot);
            bool isGamefix2 = path.ToLower().Contains(@"C:\gamefix2\");
            bool isGamefix = path.ToLower().Contains(@"C:\gamefix\");
            if (isGamefix2)
                dbPath = path.Replace(@"C:\gamefix2\", dbRoot);
            else if (isGamefix)
                dbPath = path.Replace(@"C:\gameFix\", dbRoot);

            try
            {
                dbPath = System.IO.Path.GetDirectoryName(dbPath);
                System.IO.DirectoryInfo tmpDI = new System.IO.DirectoryInfo(dbPath);
                System.IO.FileInfo[] tmpFi = tmpDI.GetFiles(System.IO.Path.GetFileName(path) + @"_vb*");
                if (tmpFi.Length > 0)
                    return true;

                return false;
            }
            catch (Exception ex)
            {
                return false;
            }
        }


        public static Int32 getDBtime(string path)
        {
            return -1;
        }

        public static bool isFileCheckedOutByAnyone(string path)
        {
            string cmd = "opened";

            string[] args = {"-a", path };

            string results = executeP4Cmd(cmd, args);

            if (results.Trim().Length == 0 ||
                results.ToLower().Contains("not opened anywhere"))
            {
                return false;
            }
            else
                return true;
        }

        public static string whoCheckedOutFile(string path)
        {
            string cmd = "opened";

            string[] args = { "-a", path };

            string results = executeP4Cmd(cmd, args);

            return results;
        }

        public static string delete(string path)
        {
            //return executeGimmeCmd("-checkout " + path);
            string cmd = "delete";
            string[] args = { path };
            return executeP4Cmd(cmd, args);
        }

        public static string undoCheckout(string path)
        {
            //return executeGimmeCmd("-undo " + path);
            string cmd = "revert";
            string[] args = { path };
            return executeP4Cmd(cmd, args);
        }

        public static string checkout(string path)
        {
            //return executeGimmeCmd("-checkout " + path);
            string cmd = "edit";
            string[] args = { path };
            return executeP4Cmd(cmd, args);
        }

        public static string move(string oldPath, string newPath)
        {
            string cmd = "move";
            string[] args = { oldPath,newPath};
            return executeP4Cmd(cmd, args);
        }

        public static string rename(string oldPath, string newPath)
        {
            string cmd = "integ";

            string[] args = { oldPath, newPath};

            string results = executeP4Cmd(cmd, args);

            results += delete(oldPath);

            return results;
        }

        public static string checkoutNoEdit(string path)
        {
            return executeGimmeCmd("-editor null " + path);
        }

        public static void preFileCheckout(string fileName)
        {
            string backUpfile = fileName + ".bak";
            FileInfo fi = new FileInfo(backUpfile);

            if (fi.Exists)
            {
                bool ro = fi.IsReadOnly;
                if (ro)
                    fi.Attributes = fi.Attributes ^ FileAttributes.ReadOnly;

                string fName = fi.Name;
                string fDir = fi.DirectoryName;
                string fNewName = fDir + @"\_" + fi.Name.Replace(".bak", "._bak");
                fi.MoveTo(fNewName);

                if (ro)
                    fi.Attributes = fi.Attributes | FileAttributes.ReadOnly;
            }
        }
       
        public static void postFileCheckout(string fileName)
        {
            FileInfo orgFi = new FileInfo(fileName);
            string orgFiName = orgFi.Name;
            string orgFiDir = orgFi.DirectoryName;
            string backUpfile = orgFiDir + @"\_" + orgFiName + "._bak";

            FileInfo fi = new FileInfo(backUpfile);

            string fNewName = orgFiDir + @"\" + orgFiName + ".bak";

            if (fi.Exists)
            {
                bool ro = fi.IsReadOnly;
                if (ro)
                    fi.Attributes = fi.Attributes ^ FileAttributes.ReadOnly;

                fi.MoveTo(fNewName);

                if (ro)
                    fi.Attributes = fi.Attributes | FileAttributes.ReadOnly;
            }
        }

        public static string p4CheckIn(string path)
        {
            P4SubmitWin p4Subwin = new P4SubmitWin();
            string description = "";
            bool keepCheckedOut = false;

            //check to see if the file is different before sumbit command to avoid the creation of extra chagelist 
            //and having the user fill in description for unchanged file
            if (isP4Different(path))
            {

                System.Windows.Forms.DialogResult dr = p4Subwin.showSubmitWin(path, out keepCheckedOut, out description);

                string cmd = "submit";

                string[] args = keepCheckedOut ? (new string[] { "-r", "-d", description, path }) :
                                                 (new string[] { "-d", description, path });

                if (dr == System.Windows.Forms.DialogResult.OK)
                {
                    return executeP4Cmd(cmd, args);
                }
                return "Check In was canceled";
            }
            else
            {
                undoCheckout(path);
                return string.Format("{0} - unchanged, reverted\r\n", path);
            }
        }

        public static string listCheckedout()
        {
            string cmd = "opened";
            System.Security.Principal.WindowsIdentity user = System.Security.Principal.WindowsIdentity.GetCurrent();
            string userName = user.Name.Replace("PRG\\", "").ToUpper();

            string[] args = {"-a", "-u" , userName};
            return executeP4Cmd(cmd, args);
        }

        public static string getLatest(string path)
        {
            return executeGimmeCmd("-glvfile  " + path);
            //string cmd = "changes";
            //string [] args = {"-m1", "-s", "submitted", "//depot/dev/..."};
            //return executeP4Cmd(cmd, args);
        }

        public static string put(string path)
        {
            return executeGimmeCmd("-put " + path);
        }
        private static string executeP4Cmd(string cmd, string [] args)
        {
            string results = "";
            try
            {
                
                P4Connection p4 = new P4Connection();
                p4.Connect();

                P4RecordSet recSet = p4.Run(cmd, args);

                foreach (object obj in recSet.Messages)
                {
                    results += String.Format("{0}\r\n", (string)obj);
                }

                foreach (P4Record rec in recSet)
                {
                    FieldDictionary fd = rec.Fields;
                    if (cmd.Equals("opened"))
                    {
                        results += fd["clientFile"];
                    }
                    else
                    {
                        foreach (string key in fd.Keys)
                        {
                            results += String.Format("{0}\t\t{1}\r\n", key, fd[key]);
                        }
                    }
                    results += "\r\n";
                }
                p4.Disconnect();

             }
            catch (Exception ex)
            {
                return ex.Message;
            }
            return results;
        }
        private static string executeGimmeCmd(string cmd)
        {

            System.Diagnostics.Process cmdProc = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo cmdProcInfo = new System.Diagnostics.ProcessStartInfo(@"gimme");
            cmdProcInfo.Arguments = cmd;
            //cmdProcInfo.UseShellExecute = false;
            //cmdProcInfo.CreateNoWindow = true;
            //cmdProcInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Minimized;
            //cmdProcInfo.RedirectStandardOutput = true;          
            ///cmdProcInfo.RedirectStandardInput = true;
            //cmdProcInfo.RedirectStandardError = true;
            cmdProc.StartInfo = cmdProcInfo;            
            
            try
            {
                cmdProc.Start();
                //cmdProc.BeginErrorReadLine();
                //string line = cmdProc.StandardOutput.ReadToEnd();
                //cmdProc.StandardInput.WriteLine("\r");
                //cmdProc.StandardInput.Flush();
                cmdProc.WaitForExit();
                return ("Completed \"gimme " + cmd + "\"\r\n");
            }
            catch(Exception ex)
            {
                if (cmdProc != null)
                {
                    cmdProc.Close();
                }
                return ("Failed: \"gimme " + cmd + "\"\r\n" + ex.Message);           
            }
        }
    }
}
