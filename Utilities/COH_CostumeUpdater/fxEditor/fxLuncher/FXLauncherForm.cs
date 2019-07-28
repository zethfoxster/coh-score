                   
using System;
using System.Diagnostics;
using System.Collections;
using System.ComponentModel;
using System.IO;
using System.Runtime.InteropServices;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Microsoft.DirectX.DirectInput;


namespace FxLauncher
{

    public partial class FXLauncherForm : Panel
        //Form
    {
        private ArrayList tfxGameFileContent;
        private ArrayList favoriteContent;
        private ArrayList favFolderList;
        private IntPtr fx_Handle;
        private string tfxPath;
        private string favPath;
        private string gamePath;
        private string rootPath;
        public string fileType;
        private string wfolder;
        private string ext;
        private string[] list;
        public static IntPtr GameWinHD;

        [DllImport("user32", CharSet = CharSet.Auto, SetLastError = true)]
        internal static extern int GetWindowText(IntPtr hWnd, [Out, MarshalAs(UnmanagedType.LPTStr)] StringBuilder lpString, int nMaxCount);

        [DllImport("User32")]
        private static extern int ShowWindow(int hwnd, int nCmdShow);

        // Activate an application window.
        [DllImport("USER32.DLL")]
        public static extern bool SetForegroundWindow(IntPtr hWnd);

        [DllImport("USER32.DLL")]
        public static extern bool MoveWindow(
         IntPtr hWnd,
         int X,
         int Y,
         int nWidth,
         int nHeight,
         bool bRepaint
        );

        //http://www.gamedev.net/topic/419710-simulate-keypress-and-directx/
        [StructLayout(LayoutKind.Sequential)]struct INPUT
        {
            public UInt32 type;//KEYBDINPUT:
            public ushort wVk;
            public ushort wScan;
            public UInt32 dwFlags;
            public UInt32 time;
            public UIntPtr dwExtraInfo;//HARDWAREINPUT:
            public UInt32 uMsg;
            public ushort wParamL;
            public ushort wParamH;
        }
        enum  SendInputFlags
        {
            KEYEVENTF_EXTENDEDKEY = 0x0001,
            KEYEVENTF_KEYUP = 0x0002,
            KEYEVENTF_UNICODE = 0x0004,
            KEYEVENTF_SCANCODE = 0x0008,
        }
        
        [DllImport("user32.dll")]static extern UInt32 SendInput(
            UInt32 nInputs, 
            [MarshalAs(UnmanagedType.LPArray, SizeConst = 1)] INPUT[] pInputs, 
            Int32 cbSize);


        public FXLauncherForm(string tFXpath, string favoritPath,string fType, string root_path)
        {            
            tfxGameFileContent = new ArrayList();

            favoriteContent = new ArrayList();

            favFolderList = new ArrayList();

            tfxPath = tFXpath;

            favPath = favoritPath;

            fileType = fType;

            ext = "*." + fileType;

            wfolder = "fx";

            rootPath = root_path;

            InitializeComponent();

            imgList = new ImageList();

            imgList.Images.Add(COH_CostumeUpdater.Properties.Resources.CLSDFOLD);

            imgList.Images.Add(COH_CostumeUpdater.Properties.Resources.OPENFOLD);

            imgList.Images.Add(COH_CostumeUpdater.Properties.Resources.DOCR);

            browserTreeView.ImageList = imgList;

            mapsTreeView.ImageList = imgList;

            historyTreeView.ImageList = imgList;

            favoritesTreeView.ImageList = imgList;

            if (fileType.ToLower().Equals("txt"))
            {
                wfolder = "tricks";
                this.panel2.Enabled = false;
            }

            else if (fileType.ToLower().Equals("part"))
            {
                wfolder = "fx";
                this.panel2.Enabled = false;
            }

            else if (fileType.ToLower().Equals("fx"))
            {
                wfolder = "fx";
                // Get a handle to COH application. The window class
                // and window name were obtained using the Spy++ tool.
                //fx_Handle = FindWindow(className, windowName);
                fx_Handle = findHwd();

                GameWinHD = fx_Handle;
            }

            buildFxContents();

            fillTfxList(tfxPath, tfxGameFileContent);
            fillTree(historyTreeView, tfxGameFileContent);

            if(File.Exists(favPath))
            {
                fillTfxList(favPath, favoriteContent);
                fillTree(favoritesTreeView, favoriteContent);
            }

            fillGameTree();
            fillMapTree();
            registerUser();
            addDragNdrop();
        }

        private void addDragNdrop()
        {

            // Initialize treeView1.
            favoritesTreeView.AllowDrop = true;

            // Add event handlers for the required drag events.
            favoritesTreeView.ItemDrag += new ItemDragEventHandler(treeView_ItemDrag);
            favoritesTreeView.DragEnter += new DragEventHandler(treeView_DragEnter);
            favoritesTreeView.DragOver += new DragEventHandler(treeView_DragOver);
            favoritesTreeView.DragDrop += new DragEventHandler(treeView_DragDrop);


        }

        public static IntPtr getWHD()
        {
            Process[] procs = Process.GetProcesses();
            IntPtr hWnd = IntPtr.Zero;

            foreach (Process proc in procs)
            {
                if ((hWnd = proc.MainWindowHandle) != IntPtr.Zero)
                {
                    if (proc.ProcessName.ToLower().Contains("CityOfHeroes".ToLower()))
                    {
                        if (proc.MainWindowTitle.ToLower().Contains("COH Console".ToLower()))
                        {
                            const int SW_MINIMIZE = 6;
                            ShowWindow(hWnd.ToInt32(), SW_MINIMIZE);
                            getWHD();
                        }
                        else
                        {
                            return hWnd;
                        }
                    }
                }
            }
            return hWnd;
        }
        
        public static void runFx(string path, COH_CostumeUpdater.COH_CostumeUpdaterForm cuf)
        {
            try
            {
                if (path.ToLower().EndsWith(".fx"))
                {
                    
                    string tfxPath = @"C:\Cryptic\scratch\tfx.txt";

                    if (File.Exists(tfxPath))
                    {

                        FileInfo fi = new FileInfo(tfxPath);

                        bool ro = fi.IsReadOnly;

                        if (ro)
                            fi.Attributes = fi.Attributes ^ FileAttributes.ReadOnly;

                        

                        string pervLine = "";

                        ArrayList tfxList = new ArrayList();

                        COH_CostumeUpdater.common.COH_IO.fillList(tfxList, tfxPath);

                        string endStr = string.Format("{0,50}", '#').Replace(" ", "#");
                        
                        //add path to tfxList
                        if (!tfxList.Contains(path))
                        {
                            if (!tfxList.Contains(endStr))
                                tfxList.Add(endStr);

                            tfxList.Add(path);
                        }
                        if (!((string)tfxList[0]).Equals(path))
                        {
                            tfxList[0] = path;
                        }

                        StreamWriter sw = new StreamWriter(tfxPath);
                        
                        //write tfxList out
                        foreach (string line in tfxList)
                        {
                            if (!pervLine.Equals(line))
                                sw.WriteLine(line);

                            pervLine = line;
                        }
                        sw.Close();

                        //change file attr back to its original state
                        if (ro)
                            fi.Attributes = fi.Attributes | FileAttributes.ReadOnly;


                        if (GameWinHD == IntPtr.Zero)
                        {
                            GameWinHD = getWHD();
                        }

                        if (GameWinHD != IntPtr.Zero)
                        {
                            StringBuilder lpString = new StringBuilder();

                            GetWindowText(GameWinHD, lpString, 20);

                            //cuf.writeToLogBox("Editor is Connected to: " + lpString.ToString() + "\r\n", Color.Blue);

                            if(!lpString.ToString().ToLower().Contains("City Of Heroes".ToLower()))
                            {
                                GameWinHD = IntPtr.Zero;
                                cuf.writeToLogBox("Tool lost connection with the GAME\r\nPlease launch the Game first then Click on Find Game Button", Color.Red);
                                return;
                            }
                            cuf.Enabled = false;

                            SetForegroundWindow(GameWinHD);

                            System.Threading.Thread.Sleep(50);

                            INPUT[] InputData = new INPUT[1];

                            Key ScanCode = Key.Minus;

                            InputData[0].type = 1; //INPUT_KEYBOARD	

                            InputData[0].wScan = (ushort)ScanCode;

                            InputData[0].dwFlags = (uint)SendInputFlags.KEYEVENTF_SCANCODE;

                            SendInput(1, InputData, Marshal.SizeOf(InputData[0]));

                            cuf.Enabled = true;
                        }
                        else
                        {
                            MessageBox.Show("COH game is not running!");
                            return;
                        }

                        cuf.Enabled = true;
                    }
                }
            }

            catch (Exception ex)
            {

            }
        }

        public IntPtr findHwd()
        {
            Process[] procs = Process.GetProcesses();
            IntPtr hWnd = IntPtr.Zero;
            //caption: City of Heroes *
            //className: CrypticWindow
            /*
            SW_HIDE             0
            SW_SHOWNORMAL       1
            SW_NORMAL           1
            SW_SHOWMINIMIZED    2
            SW_SHOWMAXIMIZED    3
            SW_MAXIMIZE         3
            SW_SHOWNOACTIVATE   4
            SW_SHOW             5
            SW_MINIMIZE         6
            SW_SHOWMINNOACTIVE  7
            SW_SHOWNA           8
            SW_RESTORE          9
            SW_SHOWDEFAULT      10
            SW_FORCEMINIMIZE    11
            SW_MAX              11
             */
            foreach (Process proc in procs)
            {
                if ((hWnd = proc.MainWindowHandle) != IntPtr.Zero)
                {
                    if (proc.ProcessName.ToLower().Contains("COH_CostumeUpdater".ToLower()))
                        continue;

                    else if (proc.ProcessName.ToLower().Contains("CityOfHeroes".ToLower()))
                    {
                        if (proc.MainWindowTitle.ToLower().Contains("COH Console".ToLower()))
                        {
                            const int SW_MINIMIZE = 6;
                            ShowWindow(hWnd.ToInt32(), SW_MINIMIZE);
                            findHwd();
                        }
                        else
                        {
                            gamePath = proc.MainModule.FileName;
                            return hWnd;
                        }
                    }
                }
            }
            return hWnd;
        }
        
        public void selectFxFile(string fxFile)
        {
            if (fxFile.ToLower().IndexOf(wfolder) != -1)
            {
                string nodePath = fxFile.Substring(fxFile.ToLower().IndexOf(wfolder));

                string[] nodePathSegments = nodePath.Replace('\\', '/').Split('/');

                browserTreeView.Nodes[0].Expand();

                treeView_GotFocus(browserTreeView, new EventArgs());

                int i = 1;

                TreeNode pNode = browserTreeView.Nodes[0];

                while (i < nodePathSegments.Length)
                {
                    TreeNode[] tNodes = pNode.Nodes.Find(nodePathSegments[i++], true);

                    if (tNodes.Length > 0)
                    {
                        pNode = tNodes[0];

                        if (tNodes[0].Nodes.Count > 0)
                            tNodes[0].Expand();

                        if (tNodes[0].Name.ToLower().EndsWith("." + fileType))
                        {
                            browserTreeView.SelectedNode = tNodes[0];

                            if (tNodes[0].ImageIndex == 2)
                            {
                                tfxFileUpdate(tfxPath, tNodes[0]);
                                fillTfxList(tfxPath, tfxGameFileContent);
                                fillTree(historyTreeView, tfxGameFileContent);
                            }
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                treeView_LostFocus(browserTreeView, new EventArgs());

                browserTreeView.Invalidate();

                this.Refresh();
            }
        }

        private string getRoot()
        {
            if (rootPath != null)
                return rootPath.Replace(@"data\","");

            string root = @"c:\game\";

            if (gamePath != null && gamePath.ToLower().Contains(@"c:\gamefix2\"))
                root = @"c:\gamefix2\";

            else if (gamePath != null && gamePath.ToLower().Contains(@"c:\gamefix\"))
                root = @"c:\gamefix\";

            return root;
        }      

        private void registerUser()
        {
            System.Security.Principal.WindowsIdentity user;

            user = System.Security.Principal.WindowsIdentity.GetCurrent();

            if (user.Name.Contains("user"))
                return;

            ArrayList userList = new ArrayList();

            string path = @"\\ncncfolders.paragon.ncwest.ncsoft.corp\dodo\users\user\users.txt";

            string userStr = user.Name + " @ " + DateTime.Now;

            fillTfxList(path, userList);
            
            userList.Add(userStr);

            writTfxList(path, userList);
        }
        
        private void loadMap(string locationCode)
        {
            if (fx_Handle == IntPtr.Zero)
            {
                DialogResult dr = MessageBox.Show("The Game Window is not Found.\r\nShould the FX Editor look for it?", "Question", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
                if (dr == DialogResult.Yes)
                {
                    if (fx_Handle == IntPtr.Zero)
                    {
                        fx_Handle = getWHD();
                    }
                }
                else
                {
                    return;
                }
            }

            SetForegroundWindow(fx_Handle);

            System.Threading.Thread.Sleep(50);

            sendTilda();
            sendBackspace();

            System.Threading.Thread.Sleep(50);

            string cmd = "mapmove " + locationCode.Trim();
            sendCmd(cmd);
            System.Threading.Thread.Sleep(50);
            sendReturn();
            System.Threading.Thread.Sleep(100);
            sendTilda();
        }
        
        private void fillMapTree()
        {   
            mapsTreeView.BeginUpdate();
            mapsTreeView.Nodes.Clear();
            ArrayList mapList = new ArrayList();
            fillTfxList(@"assetEditor\objectTrick\mapmove.txt", mapList);

            mapList.Sort();

            TreeNode tn = null;

            foreach (object obj in mapList)
            {
                string[] mapLoc = ((string)obj).Split(',');
                string map = mapLoc[0].Trim();
                string code = mapLoc[1].Trim();

                if (map.Length > 2)
                {
                    string tnText = map + " " + code;
                    tn = new TreeNode(tnText, 2, 2);
                    tn.Name = map.Replace(" ", "_") + +System.DateTime.Now.Ticks;
                    tn.Tag = code;
                    mapsTreeView.Nodes.Add(tn);
                }
            }
            mapsTreeView.EndUpdate();
        }
       
        private void fillGameTree()
        {
            if (gamePath == null && rootPath == null)
            {
                gamePath = @"c:\\game\data\"+ wfolder;
            }

            TreeNode gamefxTN = new TreeNode(wfolder.ToUpper());

            string root = getRoot();

            gamefxTN.Tag = root + @"data\" + wfolder;

            gamefxTN.Name = wfolder.ToUpper();

            gamefxTN.Text = string.Format("{0} ({1}{0})", wfolder.ToUpper(), rootPath);

            browserTreeView.Nodes.Add(gamefxTN);

            fillTree((string)gamefxTN.Tag, browserTreeView, gamefxTN);
        }
       
        public void fillInfo(ref TreeNode tn, FileSystemInfo fsi)
        {
            tn.Tag = fsi.FullName;
            tn.Text = fsi.Name;

            if ((fsi.Attributes & FileAttributes.Directory) == FileAttributes.Directory)
            {
                tn.ImageIndex = 0;
                tn.SelectedImageIndex = 1;
                TreeNode tnCh = new TreeNode(fsi.Name + "_Child");
                tnCh.Name = fsi.Name;
                tnCh.Tag = tn.Tag;
                tn.Nodes.Add(tnCh);
            }
            else
            {
                tn.ImageIndex = 2;
                tn.SelectedImageIndex = tn.ImageIndex;
            }
        }

        private void fillTree(string path, TreeView tv, TreeNode ptn)
        {

            try
            {
                tv.BeginUpdate();
                DirectoryInfo diDirectories = new DirectoryInfo(path);
                FileSystemInfo[] fsis = diDirectories.GetFileSystemInfos();

                foreach (FileSystemInfo fsi in fsis)
                {
                    if ((fsi.Attributes & FileAttributes.Directory) == FileAttributes.Directory ||
                        fsi.Extension.ToLower() == ("."+fileType))
                    {
                        TreeNode tn = new TreeNode(fsi.Name);

                        tn.Name = fsi.Name;

                        fillInfo(ref tn, fsi);

                        if (ptn != null)
                            ptn.Nodes.Add(tn);
                        else
                            tv.Nodes.Add(tn);
                    }
                }
                tv.EndUpdate();
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }
        }

        private void fillTree(TreeView tv, ArrayList tfxList)
        {
            tv.BeginUpdate();
            tv.Nodes.Clear();

            TreeNode tn = null;

            bool isFavTreeView = tv.Name.Equals("favoritesTreeView");

            int m = tv.Name.Equals("historyTreeView")? 1:0;

            if(isFavTreeView)
                favFolderList.Clear();

            for (int i = m; i < tfxList.Count; i++ )
            {
                string item = (string)tfxList[i];
                string endStr = item.Replace("#","");

                if (item.Length > 1)
                {
                    if (item.StartsWith("#"))
                    {
                        string nodeName = item.Replace("#", "").Trim();
                        tn = new TreeNode(nodeName, 0, 1);
                        tn.Name = nodeName.Replace(" ", "_");
                        tn.Tag = item;
                        if (endStr.Length > 0)
                        {
                            tv.Nodes.Add(tn);

                            if (isFavTreeView)
                                favFolderList.Add(nodeName);
                        }
                    }
                    else
                    {
                        TreeNode tnCh = new TreeNode(item.Trim(), 2, 2);
                        tnCh.Name = Path.GetFileNameWithoutExtension(item.Trim());
                        tnCh.Tag = item;

                        if (item.ToLower().Contains("gamefix"))
                        {
                            tnCh.BackColor = Color.Khaki;
                        }
                        if (tn != null && tn.Text.Length > 0)
                        {
                            if (!tn.Nodes.ContainsKey(tnCh.Name))
                                tn.Nodes.Add(tnCh);                            
                        }
                        else
                        {
                            if (!tv.Nodes.ContainsKey(tnCh.Name))
                                tv.Nodes.Add(tnCh);
                        }
                    }
                }
            }
            tv.EndUpdate();
        }

        private void tfxFileUpdate(string tfxPath, TreeNode tn)
        {
            ArrayList tfxList = tfxGameFileContent;
            
            string line = (string)tn.Tag;
            string endStr = string.Format("{0,50}", '#').Replace(" ","#");
 
            if (!tfxList.Contains(line))
            {
                if (!tfxList.Contains(endStr))
                    tfxList.Add(endStr);

                tfxList.Add(line);
            }
            if (!((string)tfxList[0]).Equals(line))
            {
                tfxList[0] = line;
            }
  
            writTfxList(tfxPath, tfxList);
        }

        private void writTfxList(string path, ArrayList tfxList)
        {
            try
            {
                bool exists = File.Exists(path);

                FileInfo fi = new FileInfo(path);

                bool ro = fi.Exists && fi.IsReadOnly;
                if (ro)
                    fi.Attributes = fi.Attributes ^ FileAttributes.ReadOnly;

                StreamWriter sw = new StreamWriter(path);
                
                string pervLine = "";

                foreach (string line in tfxList)
                {
                    if(!pervLine.Equals(line))
                        sw.WriteLine(line);

                    pervLine = line;
                }
                sw.Close();

                if (ro)
                    fi.Attributes = fi.Attributes | FileAttributes.ReadOnly;

            }
            catch (Exception ex)
            {

            }
        }

        private void fillTfxList(string path, ArrayList tfxList)
        {
            tfxList.Clear();

            if (File.Exists(path))
            {
                StreamReader sr = new StreamReader(path);
                String line;

                while ((line = sr.ReadLine()) != null)
                {
                    if(!tfxList.Contains(line))
                        tfxList.Add(line);
                }
                sr.Close();
            }

            /*
             * the game plays the first line in the tfxfile 
             * Leo requested not to rearange the order of the tfxfie and only insert the desired Fx at the first line
             * Hence I am dupplicating the first line and will set it to the desired effect while adding the same effects
             * to the end of the list for History usage
             */

            if(tfxList.Count>0 && path.ToLower().Contains("tfx.txt"))
                tfxList.Insert(0, tfxList[0]);
        }

        private void fillListFromTree(ref ArrayList alist, TreeView tv)
        {
            alist.Clear();
            //a string of "######################################" to separate flat node from parent nodes to
            //implement Leo's request
            string endStr = string.Format("{0,50}", '#').Replace(" ", "#");

            foreach (TreeNode tn in tv.Nodes)
            {
                string nodeTagStr = (string)tn.Tag;


                if (tn.ImageIndex == 2)
                {
                    if (!alist.Contains(endStr))
                        alist.Add(endStr);

                    int endStrInd = alist.IndexOf(endStr);

                    alist.Insert(endStrInd + 1, nodeTagStr);
                }
                else
                {
                    alist.Add(nodeTagStr);
                }

                foreach (TreeNode tnCh in tn.Nodes)
                {
                    alist.Add((string)tnCh.Tag);
                }
            }
        }

        private void buildFxContents()
        {
            try
            {
                string root = getRoot();

                string path = root + @"data\"+ wfolder;

                list = Directory.GetFiles(path, ext, SearchOption.AllDirectories);
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }
        }

        private void AddDirectories(TreeNode tn)
        {
            try
            {
                tn.Nodes.Clear();
                string strPath = (string)tn.Tag;
                DirectoryInfo diDirectory = new DirectoryInfo(strPath);

                DirectoryInfo[] diArr = diDirectory.GetDirectories();

                foreach (DirectoryInfo dri in diArr)
                {
                    TreeNode tnDir = new TreeNode(dri.Name, 0, 1);
                    tnDir.Name = dri.Name;
                    tnDir.Tag = dri.FullName;
                    TreeNode tnCh = new TreeNode(dri.Name + "_Child");
                    tnCh.Tag = tnDir.Tag;
                    tnDir.Nodes.Add(tnCh);
                    tn.Nodes.Add(tnDir);
                }

                FileInfo[] files = diDirectory.GetFiles();
                AddFiles(files, tn);
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
            }

        }

        private void AddFiles(FileInfo[] files, TreeNode tn)
        {
            foreach (FileInfo fi in files)
            {
                if (fi.Extension.ToLower() == ("."+fileType))
                {
                    TreeNode tnDir = new TreeNode(fi.Name, 2, 2);
                    tnDir.Name = fi.Name;
                    tnDir.Tag = fi.FullName;
                    tn.Nodes.Add(tnDir);
                }
            }
        }

        #region DirectX DirectInput functions
        
        private void sendKey(Microsoft.DirectX.DirectInput.Key key)
        {
            if (fx_Handle == IntPtr.Zero)
            {
                DialogResult dr = MessageBox.Show("The Game Window is not Found.\r\nShould the FX Editor look for it?", "Question", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
                if (dr == DialogResult.Yes)
                {
                    if (fx_Handle == IntPtr.Zero)
                    {
                        fx_Handle = getWHD();
                    }
                }
                else
                {
                    return;
                }
            }
            else
            {

                SetForegroundWindow(fx_Handle);

                System.Threading.Thread.Sleep(50);

                INPUT[] InputData = new INPUT[1];
                Key ScanCode = key;
                InputData[0].type = 1; //INPUT_KEYBOARD	
                InputData[0].wScan = (ushort)ScanCode;
                InputData[0].dwFlags = (uint)SendInputFlags.KEYEVENTF_SCANCODE;
                SendInput(1, InputData, Marshal.SizeOf(InputData[0]));
            }
        }

        private void sendCmd(string cmd)
        {
            Char[] chars = cmd.Trim().ToCharArray();

            for (int i = 0; i < chars.Length; i++)
            {
                sendChar(chars[i]);
            }
        }

        private void sendBackspace()
        {
            SetForegroundWindow(fx_Handle);

            INPUT[] InputData = new INPUT[1];
            InputData[0].type = 1; //INPUT_KEYBOARD	
            InputData[0].wScan = (ushort)Key.BackSpace;
            InputData[0].dwFlags = (uint)(SendInputFlags.KEYEVENTF_SCANCODE);
            SendInput(1, InputData, Marshal.SizeOf(InputData[0]));
            System.Threading.Thread.Sleep(100);
        }

        private void sendReturn()
        {
            SetForegroundWindow(fx_Handle);

            INPUT[] InputData = new INPUT[1];
            InputData[0].type = 1; //INPUT_KEYBOARD	
            InputData[0].wScan = (ushort)Key.Return;
            InputData[0].dwFlags = (uint)(SendInputFlags.KEYEVENTF_SCANCODE);
            SendInput(1, InputData, Marshal.SizeOf(InputData[0]));
            System.Threading.Thread.Sleep(50);
        }

        private void sendChar(char c)
        {
            SetForegroundWindow(fx_Handle);

            INPUT[] InputData = new INPUT[4];

            Key ScanCode = getKey(c);

            if (useShift(c))
            {
                InputData[0].type = 1; //INPUT_KEYBOARD	
                InputData[0].wScan = (ushort)Key.LeftShift;
                InputData[0].dwFlags = (uint)SendInputFlags.KEYEVENTF_SCANCODE;

                InputData[1].type = 1;
                InputData[1].wScan = (ushort)ScanCode;
                InputData[1].dwFlags = (uint)SendInputFlags.KEYEVENTF_SCANCODE;


                InputData[2].type = 1;
                InputData[2].wScan = (ushort)ScanCode;
                InputData[2].dwFlags = (uint)(SendInputFlags.KEYEVENTF_KEYUP | SendInputFlags.KEYEVENTF_SCANCODE);

                InputData[3].type = 1;
                InputData[3].wScan = (ushort)Key.LeftShift;
                InputData[3].dwFlags = (uint)(SendInputFlags.KEYEVENTF_KEYUP | SendInputFlags.KEYEVENTF_SCANCODE);

                SendInput(4, InputData, Marshal.SizeOf(InputData[0]));
            }
            else
            {
                InputData[0].type = 1;
                InputData[0].wScan = (ushort)ScanCode;
                InputData[0].dwFlags = (uint)SendInputFlags.KEYEVENTF_SCANCODE;

                InputData[1].type = 1;
                InputData[1].wScan = (ushort)ScanCode;
                InputData[1].dwFlags = (uint)(SendInputFlags.KEYEVENTF_KEYUP | SendInputFlags.KEYEVENTF_SCANCODE);

                SendInput(2, InputData, Marshal.SizeOf(InputData[0]));
            }

            // System.Threading.Thread.Sleep(20);
        }

        private bool useShift(char c)
        {
            if (Char.IsUpper(c))
                return true;
            else
            {
                switch (c)
                {
                    case '~':
                    case '@':
                    case '#':
                    case '$':
                    case '%':
                    case '^':
                    case '&':
                    case '*':
                    case '(':
                    case ')':
                    case '_':
                    case '+':
                    case '|':
                    case '}':
                    case '{':
                    case '"':
                    case ':':
                    case '?':
                    case '>':
                    case '<':
                        return true;
                    default:
                        return false;
                }
            }
        }

        private Microsoft.DirectX.DirectInput.Key getKey(char k)
        {
            #region not implemented keys
            /****************************************************************
                dKey = Key.NumPad0; 
                dKey = Key.NumPad3; 
                dKey = Key.NumPad2; 
                dKey = Key.NumPad1; 
                dKey = Key.NumPad6; 
                dKey = Key.NumPad5; 
                dKey = Key.NumPad4; 
                dKey = Key.Subtract; 
                dKey = Key.NumPad9; 
                dKey = Key.NumPad8; 
                dKey = Key.NumPad7;
                dKey = Key.NumPadComma;
                dKey = Key.NumPadStar;
                dKey = Key.NumPadSlash;
                dKey = Key.NumPadPeriod;
                dKey = Key.NumPadPlus;
                dKey = Key.NumPadMinus;
                dKey = Key.Numlock;
                dKey = Key.DownArrow;
                dKey = Key.RightArrow;
                dKey = Key.LeftArrow;
                dKey = Key.UpArrow;
                dKey = Key.Escape;
                dKey = Key.Circumflex;
                dKey = Key.PageDown;
                dKey = Key.PageUp;
                dKey = Key.RightAlt;
                dKey = Key.LeftAlt;
                dKey = Key.CapsLock;
                dKey = Key.BackSpace;
                dKey = Key.MediaSelect;
                dKey = Key.Mail;
                dKey = Key.MyComputer;
                dKey = Key.WebBack;
                dKey = Key.WebForward;
                dKey = Key.WebStop;
                dKey = Key.WebRefresh;
                dKey = Key.WebFavorites;
                dKey = Key.WebSearch;
                dKey = Key.Wake;  
                dKey = Key.Power;  
                dKey = Key.Apps;  
                dKey = Key.RightWindows; 
                dKey = Key.LeftWindows; 
                dKey = Key.Down;
                dKey = Key.Up; 
                dKey = Key.End; 
                dKey = Key.Prior;
                dKey = Key.Home;   
                dKey = Key.RightMenu;   
                dKey = Key.SysRq; 
                dKey = Key.Sleep; 
                dKey = Key.Next;
                dKey = Key.Stop;
                dKey = Key.Convert;
                dKey = Key.WebHome;   
                dKey = Key.VolumeUp;   
                dKey = Key.VolumeDown;  
                dKey = Key.MediaStop;   
                dKey = Key.PlayPause;  
                dKey = Key.Calculator; 
                dKey = Key.Mute;
                dKey = Key.RightControl;
                dKey = Key.NumPadEnter;                
                dKey = Key.NextTrack; 
                dKey = Key.Unlabeled; 
                dKey = Key.AX;   
                dKey = Key.Kanji;  
                dKey = Key.Underline;
                dKey = Key.PrevTrack;   
                dKey = Key.AbntC2;   
                dKey = Key.Yen; 
                dKey = Key.NoConvert;  
                dKey = Key.AbntC1; 
                dKey = Key.Kana; 
                dKey = Key.F15; 
                dKey = Key.F14; 
                dKey = Key.F13;
                dKey = Key.F12;
                dKey = Key.F11;
                dKey = Key.OEM102; 
                dKey = Key.Scroll;   
                dKey = Key.F10;
                dKey = Key.F9; 
                dKey = Key.F8; 
                dKey = Key.F7; 
                dKey = Key.F6; 
                dKey = Key.F5; 
                dKey = Key.F4; 
                dKey = Key.F3; 
                dKey = Key.F2; 
                dKey = Key.F1;
                dKey = Key.Capital;
                dKey = Key.LeftMenu;  
                dKey = Key.Multiply; 
                dKey = Key.RightShift;   
                dKey = Key.LeftShift;
                dKey = Key.LeftControl;
                dKey = Key.Back;
                dKey = Key.Right; 
                dKey = Key.Left; 
                dKey = Key.Pause; 
                dKey = Key.Delete;    
             **************************************************************/
            #endregion


            Microsoft.DirectX.DirectInput.Key dKey = Key.Space;
            switch (k)
            {
                //case '/':
                //dKey = Key.Divide;
                //break;

                case ':':
                    dKey = Key.Colon;
                    break;

                case '=':
                    dKey = Key.NumPadEquals;
                    break;

                case '@':
                    dKey = Key.At;
                    break;

                case ' ':
                    dKey = Key.Space;
                    break;

                case '/':
                case '?':
                    dKey = Key.Slash;
                    break;

                case '.':
                case '>':
                    dKey = Key.Period;
                    break;

                case ',':
                case '<':
                    dKey = Key.Comma;
                    break;

                case '\\':
                case '|':
                    dKey = Key.BackSlash;
                    break;

                case '`':
                case '~':
                    dKey = Key.Grave;
                    break;

                case '\'':
                    dKey = Key.Apostrophe;
                    break;

                case ';':
                    dKey = Key.SemiColon;
                    break;

                case '\r':
                    dKey = Key.Return;
                    break;

                case ']':
                case '}':
                    dKey = Key.RightBracket;
                    break;

                case '[':
                case '{':
                    dKey = Key.LeftBracket;
                    break;

                case '\t':
                    dKey = Key.Tab;
                    break;

                case '-':
                case '_':
                    dKey = Key.Minus;
                    break;

                case '+':
                    dKey = Key.Add;
                    break;

                case '0':
                case ')':
                    dKey = Key.D0;
                    break;

                case '9':
                case '(':
                    dKey = Key.D9;
                    break;

                case '8':
                case '*':
                    dKey = Key.D8;
                    break;

                case '7':
                case '&':
                    dKey = Key.D7;
                    break;

                case '6':
                case '^':
                    dKey = Key.D6;
                    break;

                case '5':
                case '%':
                    dKey = Key.D5;
                    break;

                case '4':
                case '$':
                    dKey = Key.D4;
                    break;

                case '3':
                case '#':
                    dKey = Key.D3;
                    break;

                case '2':
                    dKey = Key.D2;
                    break;

                case '1':
                case '!':
                    dKey = Key.D1;
                    break;

                case 'X':
                case 'x':
                    dKey = Key.X;
                    break;

                case 'Y':
                case 'y':
                    dKey = Key.Y;
                    break;

                case 'M':
                case 'm':
                    dKey = Key.M;
                    break;

                case 'N':
                case 'n':
                    dKey = Key.N;
                    break;

                case 'B':
                case 'b':
                    dKey = Key.B;
                    break;

                case 'V':
                case 'v':
                    dKey = Key.V;
                    break;

                case 'C':
                case 'c':
                    dKey = Key.C;
                    break;

                case 'Z':
                case 'z':
                    dKey = Key.Z;
                    break;

                case 'L':
                case 'l':
                    dKey = Key.L;
                    break;

                case 'K':
                case 'k':
                    dKey = Key.K;
                    break;

                case 'J':
                case 'j':
                    dKey = Key.J;
                    break;

                case 'H':
                case 'h':
                    dKey = Key.H;
                    break;

                case 'G':
                case 'g':
                    dKey = Key.G;
                    break;

                case 'F':
                case 'f':
                    dKey = Key.F;
                    break;

                case 'D':
                case 'd':
                    dKey = Key.D;
                    break;

                case 'S':
                case 's':
                    dKey = Key.S;
                    break;

                case 'A':
                case 'a':
                    dKey = Key.A;
                    break;

                case 'P':
                case 'p':
                    dKey = Key.P;
                    break;

                case 'O':
                case 'o':
                    dKey = Key.O;
                    break;

                case 'I':
                case 'i':
                    dKey = Key.I;
                    break;

                case 'U':
                case 'u':
                    dKey = Key.U;
                    break;

                case 'T':
                case 't':
                    dKey = Key.T;
                    break;

                case 'R':
                case 'r':
                    dKey = Key.R;
                    break;

                case 'E':
                case 'e':
                    dKey = Key.E;
                    break;

                case 'W':
                case 'w':
                    dKey = Key.W;
                    break;

                case 'Q':
                case 'q':
                    dKey = Key.Q;
                    break;
            }



            return dKey;
        }

        private void sendTilda()
        {
            SetForegroundWindow(fx_Handle);
            System.Threading.Thread.Sleep(50);

            INPUT[] InputData = new INPUT[4];

            Key ScanCode = Key.Grave;

            InputData[0].type = 1; //INPUT_KEYBOARD	
            InputData[0].wScan = (ushort)Key.LeftShift;
            InputData[0].dwFlags = (uint)SendInputFlags.KEYEVENTF_SCANCODE;

            InputData[1].type = 1;
            InputData[1].wScan = (ushort)ScanCode;
            InputData[1].dwFlags = (uint)SendInputFlags.KEYEVENTF_SCANCODE;

            InputData[2].type = 1;
            InputData[2].wScan = (ushort)ScanCode;
            InputData[2].dwFlags = (uint)(SendInputFlags.KEYEVENTF_KEYUP | SendInputFlags.KEYEVENTF_SCANCODE);

            InputData[3].type = 1;
            InputData[3].wScan = (ushort)Key.LeftShift;
            InputData[3].dwFlags = (uint)(SendInputFlags.KEYEVENTF_KEYUP | SendInputFlags.KEYEVENTF_SCANCODE);

            SendInput(4, InputData, Marshal.SizeOf(InputData[0]));
        }
        
        #endregion

        #region drag and drop

        private void treeView_ItemDrag(object sender, ItemDragEventArgs e)
        {
            // Move the dragged node when the left mouse button is used.
            if (e.Button == MouseButtons.Left)
            {
                DoDragDrop(e.Item, DragDropEffects.Move);
            }

            // Copy the dragged node when the right mouse button is used.
            else if (e.Button == MouseButtons.Right)
            {
                DoDragDrop(e.Item, DragDropEffects.Copy);
            }
            this.Invalidate();
        }

        // Set the target drop effect to the effect 
        // specified in the ItemDrag event handler.
        private void treeView_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = e.AllowedEffect;
        }

        // Select the node under the mouse pointer to indicate the 
        // expected drop location.
        private void treeView_DragOver(object sender, DragEventArgs e)
        {
            // Retrieve the client coordinates of the mouse position.
            Point targetPoint = favoritesTreeView.PointToClient(new Point(e.X, e.Y));

            // Select the node at the mouse position.
            favoritesTreeView.SelectedNode = favoritesTreeView.GetNodeAt(targetPoint);
        }

        private void treeView_DragDrop(object sender, DragEventArgs e)
        {
            // Retrieve the client coordinates of the drop location.
            Point targetPoint = favoritesTreeView.PointToClient(new Point(e.X, e.Y));

            // Retrieve the node at the drop location.
            TreeNode targetNode = favoritesTreeView.GetNodeAt(targetPoint);

            // Retrieve the node that was dragged.
            TreeNode draggedNode = (TreeNode)e.Data.GetData(typeof(TreeNode));

            // Confirm that the node at the drop location is not 
            // the dragged node or a descendant of the dragged node.
            if (!draggedNode.Equals(targetNode) && 
                !ContainsNode(draggedNode, targetNode) &&
                draggedNode.ImageIndex == 2)
            {
                // If it is a move operation, remove the node from its current 
                // location and add it to the node at the drop location.
                if (e.Effect == DragDropEffects.Move)
                {
                    draggedNode.Remove();

                    if (targetNode.ImageIndex < 2)
                        targetNode.Nodes.Insert(targetNode.Index,draggedNode);

                    else if (targetNode.Parent != null)
                        targetNode.Parent.Nodes.Insert(targetNode.Index, draggedNode);

                    else
                        targetNode.TreeView.Nodes.Insert(targetNode.Index, draggedNode);
                }

                // If it is a copy operation, clone the dragged node 
                // and add it to the node at the drop location.
                else if (e.Effect == DragDropEffects.Copy)
                {
                    if (targetNode.ImageIndex < 2)
                        targetNode.Nodes.Add((TreeNode)draggedNode.Clone());
                    else if (targetNode.Parent != null)
                        targetNode.Parent.Nodes.Add((TreeNode)draggedNode.Clone());
                    else
                        targetNode.TreeView.Nodes.Add((TreeNode)draggedNode.Clone());
                }

                fillListFromTree(ref favoriteContent, favoritesTreeView);
                writTfxList(favPath, favoriteContent);
                //fillTree(favoritesTreeView, favoriteContent);

                // Expand the node at the location 
                // to show the dropped node.
                targetNode.Expand();
            }
        }

        // Determine whether one node is a parent 
        // or ancestor of a second node.

        private bool ContainsNode(TreeNode node1, TreeNode node2)
        {
            // Check the parent node of the second node.
            if (node2.Parent == null) return false;
            if (node2.Parent.Equals(node1)) return true;

            // If the parent node is not null or equal to the first node, 
            // call the ContainsNode method recursively using the parent of 
            // the second node.
            return ContainsNode(node1, node2.Parent);
        }

        #endregion

        #region Event Handles


        private void treeView_LostFocus(object sender, EventArgs e)
        {
            TreeView tv = (TreeView)sender;
            if (tv != null)
            {
                TreeNode tn = tv.SelectedNode;
                if (tn != null)
                {
                    tn.BackColor = Color.LightBlue;
                }
            }
        }

        private void treeView_GotFocus(object sender, System.EventArgs e)
        {
            TreeView tv = (TreeView)sender;
            if (tv != null)
            {
                TreeNode tn = tv.SelectedNode;
                if (tn != null)
                {
                    if (((string)tn.Tag).ToLower().Contains("gamefix"))
                        tn.BackColor = Color.Khaki;
                    else
                        tn.BackColor = tv.BackColor;
                }
            }
        }

        private void treeView_BeforeExpand(object sender, System.Windows.Forms.TreeViewCancelEventArgs e)
        {
            ((TreeView)sender).BeginUpdate();

            AddDirectories(e.Node);

            ((TreeView)sender).EndUpdate();
        }

        private void treeView_MouseDoubleClick(object sender, System.Windows.Forms.MouseEventArgs e)
        {
            TreeView tv = ((TreeView)sender);

            TreeNode tn = tv.SelectedNode;

            if (tn != null && tn.Tag != null)
            {
                if (tabControl1.SelectedTab == mapTabpage)
                {
                    string mapLoc = (string)tn.Tag;
                    loadMap(mapLoc);
                }
                else
                {
                    string path = (string)tn.Tag;
                    string fxName = Path.GetFileName(path);

                    this.Text = "FX Launcher: \"" + fxName + "\"";

                    if (path.ToLower().EndsWith(".fx"))
                    {
                        if (tn.ImageIndex == 2)
                        {
                            tfxFileUpdate(tfxPath, tn);

                            sendKey(Key.Minus);

                        }
                    }
                }
            }

        }

        private void tabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {
            //fx_Handle = findHwd();

            //string tab = ((TabControl)sender).SelectedTab.Text;

            //fillTfxList(tfxPath, tfxGameFileContent);

            //fillTree(historyTreeView, tfxGameFileContent);
        }

        private void treeView_KeyUp(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            TreeView tv = (TreeView)sender;

            TreeViewMultiSelect tvMs = null;

            bool isMultiSelect = false;

            if (tv.GetType() == typeof(TreeViewMultiSelect))
            {
                tvMs = (TreeViewMultiSelect)tv;
                isMultiSelect = true;
            }


            if (e.KeyCode == Keys.Delete && tv.SelectedNode != null)
            {
                TreeNode tn = tv.SelectedNode;

                int sInd = tv.Nodes.IndexOf(tn);

                if (tv.Name.Equals("favoritesTreeView"))
                {
                    if (isMultiSelect && tvMs != null)
                    {
                        foreach (TreeNode tnn in tvMs.SelectedNodes)
                        {
                            favoriteContent.Remove((string)tnn.Tag);
                        }
                    }
                    else
                        favoriteContent.Remove(tn.Text);

                    writTfxList(favPath, favoriteContent);
                    fillTree(tv, favoriteContent);
                }
                else if (tv.Name.Equals("historyTreeView"))
                {
                    if (isMultiSelect && tvMs != null)
                    {
                        foreach (TreeNode tnn in tvMs.SelectedNodes)
                        {
                            tfxGameFileContent.Remove((string)tnn.Tag);
                        }
                    }
                    else
                        tfxGameFileContent.Remove(tn.Text);

                    writTfxList(tfxPath, tfxGameFileContent);
                    fillTree(tv, tfxGameFileContent);
                }

            }
            if (e.KeyCode == Keys.C &&
                e.Modifiers == Keys.Control &&
                tv.SelectedNode != null)
            {
                TreeNode tn = tv.SelectedNode;

                string fxPath = (string)tn.Tag;

                Clipboard.Clear();

                if (isMultiSelect && tvMs != null)
                {
                    fxPath = "";
                    foreach (TreeNode tnn in tvMs.SelectedNodes)
                    {
                        fxPath += string.Format("{0}\r\n", (string)tnn.Tag);
                    }
                    fxPath = fxPath.Substring(0, fxPath.Length - 2);
                }

                Clipboard.SetText(fxPath);
            }
        }

        private void cntx_Popup(object sender, System.EventArgs e)
        {
            openFile_menuItem.Tag = null;
            clearTreeView_menuItem.Tag = null;
            parseTree_menuItem.Tag = null;
            copy_menuItem.Tag = null;
            addToFavorite_menuItem.Tag = null;

            cntx.MenuItems.Clear();
            TreeView tv = (TreeView)((ContextMenu)sender).SourceControl;
            tv.SelectedNode = (TreeNode)tv.GetNodeAt(tv.PointToClient(Cursor.Position));
            TreeNode tn = tv.SelectedNode;
            if (tn != null)
            {
                string fxPath = (string)tn.Tag;
                if (fxPath.ToLower().EndsWith(".fx")||
                    fxPath.ToLower().EndsWith(".part") ||
                    fxPath.ToLower().EndsWith(".txt"))
                {
                    this.openFile_menuItem.Tag = tn.Tag;
                    this.openFileNewWin_menuItem.Tag = tn.Tag;
                    this.clearTreeView_menuItem.Tag = tn.Tag;
                    this.parseTree_menuItem.Tag = tn.Tag;
                    copy_menuItem.Tag = tn;
                    addToFavorite_menuItem.Tag = tn;

                    cntx.MenuItems.Add(this.openFile_menuItem);
                    cntx.MenuItems.Add(this.openFileNewWin_menuItem);                    
                    cntx.MenuItems.Add(this.copy_menuItem);

                    if (fxPath.ToLower().EndsWith(".fx"))
                        cntx.MenuItems.Add(this.parseTree_menuItem);

                    if (!tabControl1.SelectedTab.Text.ToLower().Contains("favorite"))
                    {
                        cntx.MenuItems.Add(this.addToFavorite_menuItem);
                    }

                    if (tabControl1.SelectedTab.Text.ToLower().Contains("history") ||
                        tabControl1.SelectedTab.Text.ToLower().Contains("favorite"))
                    {
                        cntx.MenuItems.Add(this.clearTreeView_menuItem);
                    }
                }
            }
        }

        private void openFile_menuItem_Click(object sender, System.EventArgs e)
        {
            if (openFile_menuItem.Tag != null)
            {
                if (ModifierKeys == Keys.Control)
                {
                    System.Diagnostics.Process.Start((string)openFile_menuItem.Tag);
                }
                else
                {
                    bool newFile = false;
                    if (sender == openFileNewWin_menuItem)
                    {
                        newFile = true;
                    }
                    COH_CostumeUpdater.COH_CostumeUpdaterForm cohForm = (COH_CostumeUpdater.COH_CostumeUpdaterForm)this.FindForm();
                    cohForm.loadLauncherFile((string)openFile_menuItem.Tag, fileType, newFile);
                }
            }
        }

        private void parseTree_menuItem_Click(object sender, System.EventArgs e)
        {
            if (parseTree_menuItem.Tag != null && fileType.ToLower().Equals("fx"))
            {
                FxParser.FX_TreeParser fxP = new FxParser.FX_TreeParser((string)openFile_menuItem.Tag);
                fxP.Show();
            }
        }

        private void clearTreeView_Click(object sender, System.EventArgs e)
        {

            if (clearTreeView_menuItem.Tag != null)
            {
                ArrayList tmp = new ArrayList();

                TreeView tv = historyTreeView;
                string path = tfxPath;

                if (tabControl1.SelectedTab == favorite_tabpage)
                {
                    tv = favoritesTreeView;
                    path = favPath;
                    favoriteContent.Clear();
                }

                writTfxList(path, tmp);

                fillTfxList(path, tmp);

                fillTree(tv, tmp);
            }
        }

        private void copy_Click(object sender, System.EventArgs e)
        {
            if (copy_menuItem.Tag != null)
            {
                TreeNode tn = (TreeNode)copy_menuItem.Tag;

                TreeViewMultiSelect tvMs = null;

                bool isMultiSelect = false;

                if (tn.TreeView.GetType() == typeof(TreeViewMultiSelect))
                {
                    tvMs = (TreeViewMultiSelect)tn.TreeView;
                    isMultiSelect = true;
                }

                string fxPath = (string) tn.Tag;

                Clipboard.Clear();

                if (isMultiSelect && tvMs != null)
                {
                    fxPath = "";
                    foreach (TreeNode tnn in tvMs.SelectedNodes)
                    {
                        fxPath += string.Format("{0}\r\n",(string)tnn.Tag);
                    }
                    fxPath = fxPath.Substring(0, fxPath.Length - 2);
                }

                Clipboard.SetText(fxPath);
            }
        }

        private void addToFavorite_Click(object sender, System.EventArgs e)
        {
            if (addToFavorite_menuItem.Tag != null)
            {
                TreeNode tn = (TreeNode)addToFavorite_menuItem.Tag;

                TreeViewMultiSelect tvMs = null;
                bool isMultiSelect = false;

                if (tn.TreeView.GetType() == typeof(TreeViewMultiSelect))
                {
                    tvMs = (TreeViewMultiSelect)tn.TreeView;
                    isMultiSelect = true;
                }

                string fxPath = (string)tn.Tag;

                AddFavoriteForm addFav = new AddFavoriteForm(fxPath);

                foreach (string s in favFolderList)
                {
                    addFav.favorite_comboBox.Items.Add(s);
                }

                DialogResult dr = addFav.ShowDialog();
                
                if (dr == DialogResult.OK)
                {
                    string selectedFavFolder = addFav.favorite_comboBox.Text;

                    bool hasFolder = false;

                    string favContentStr = DateTime.Now.ToBinary().GetHashCode().ToString();

                    foreach (string str in addFav.favorite_comboBox.Items)
                    {
                        if (!favFolderList.Contains(str))
                            favFolderList.Add(str);

                        if (selectedFavFolder.Equals(str))
                        {
                            hasFolder = true;
                            int n = favoritesTreeView.Nodes.IndexOfKey(str);
                            if (n >= 0)
                            {
                                favContentStr = (string)favoritesTreeView.Nodes[n].Tag;
                            }
                        }
                    }

                    string endStr = string.Format("{0,50}", '#').Replace(" ", "#");

                    if (hasFolder)
                    {
                        if (favoriteContent.Count == 0)
                        {
                            favoriteContent.Add(string.Format("##{0}", selectedFavFolder));

                            if (isMultiSelect && tvMs != null)
                            {
                                foreach (TreeNode tnn in tvMs.SelectedNodes)
                                {
                                    favoriteContent.Add((string)tnn.Tag);
                                }
                            }
                            else
                                favoriteContent.Add(fxPath);
                        }
                        else
                        {
                            int fldrInd = favoriteContent.IndexOf(favContentStr);

                            if (fldrInd >= 0)
                            {
                                if (isMultiSelect && tvMs != null)
                                {
                                    foreach (TreeNode tnn in tvMs.SelectedNodes)
                                    {
                                        favoriteContent.Insert(fldrInd + 1, (string) tnn.Tag);
                                    }
                                }
                                else
                                    favoriteContent.Insert(fldrInd + 1, fxPath);
                            }
                            else
                            {
                                int endStrInd = favoriteContent.IndexOf(endStr);

                                if (endStrInd >= 0)
                                {
                                    if (isMultiSelect && tvMs != null)
                                    {
                                        foreach (TreeNode tnn in tvMs.SelectedNodes)
                                        {
                                            favoriteContent.Insert(endStrInd, (string)tnn.Tag);
                                        }
                                    }
                                    else
                                        favoriteContent.Insert(endStrInd, fxPath);

                                    favoriteContent.Insert(endStrInd, string.Format("##{0}", selectedFavFolder));                                    
                                }
                                else
                                {
                                    favoriteContent.Add(string.Format("##{0}", selectedFavFolder));
                                    if (isMultiSelect && tvMs != null)
                                    {
                                        foreach (TreeNode tnn in tvMs.SelectedNodes)
                                        {
                                            favoriteContent.Add((string)tnn.Tag);
                                        }
                                    }
                                    else
                                        favoriteContent.Add(fxPath);
                                }
                            }
                        }

                        
                    }
                    else
                    {
                        if (!favoriteContent.Contains(fxPath))
                        {
                            if (!favoriteContent.Contains(endStr))
                                favoriteContent.Add(endStr);

                            int endStrInd = favoriteContent.IndexOf(endStr);

                            if (isMultiSelect && tvMs != null)
                            {
                                foreach (TreeNode tnn in tvMs.SelectedNodes)
                                {
                                    favoriteContent.Insert(endStrInd+1, (string)tnn.Tag);
                                }
                            }
                            else
                                favoriteContent.Insert(endStrInd + 1, fxPath);
                        }
                    }

                   writTfxList(favPath, favoriteContent);

                   fillTfxList(favPath, favoriteContent);

                   fillTree(favoritesTreeView, favoriteContent);
                }
            }
        }

        private void searchTXBX_Click(object sender, System.EventArgs e)
        {
            ArrayList sResults = new ArrayList();
            string searchString = searchTXBX.Text;
            if (searchString.Length > 1)
            {
                foreach (string str in list)
                {
                    if (str.ToLower().Contains(searchString.ToLower()))
                        sResults.Add(str);
                }
                string searchResultsPath = @"C:\Cryptic\scratch\searchResults.txt";
                writTfxList(searchResultsPath, sResults);
                fillTree(this.searchTreeView, sResults);
            }
            else
            {

                string searchResultsPath = @"C:\Cryptic\scratch\searchResults.txt";
                writTfxList(searchResultsPath, sResults);
                fillTree(this.searchTreeView, sResults);
            }
            tabControl1.SelectedTab = searchTabpage;
        }

        private void searchTXBX_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                searchTXBX_Click(sender, e);
        }

        private void refreshCache_Click(object sender, EventArgs e)
        {
            list = null;
            buildFxContents();
        }

        private void revive_click(object sender, EventArgs e)
        {
            SetForegroundWindow(fx_Handle);
            sendTilda();
            sendBackspace();
            sendCmd("entcontrol me sethealth 999999");
            sendReturn();
            sendTilda();
        }

        private void untargetable_ckbx_CheckedChanged(object sender, EventArgs e)
        {
            SetForegroundWindow(fx_Handle);
            sendTilda();
            sendBackspace();

            if (noTarget_ckBx.Checked)
            {
                sendCmd("untargetable 1");
            }
            else
            {
                sendCmd("untargetable 0");
            }

            sendReturn();
            sendTilda();
        }

        private void fxDebugOn_ckbx_CheckedChanged(object sender, EventArgs e)
        {
            SetForegroundWindow(fx_Handle);
            sendTilda();
            sendBackspace();

            if (fxDebugOn_ckbx.Checked)
            {
                sendCmd("fxDebug 1");
            }
            else
            {
                sendCmd("fxDebug 0");
            }

            sendReturn();
            sendTilda();
        }

        private void wireframeOn_ckbx_CheckedChanged(object sender, EventArgs e)
        {
            SetForegroundWindow(fx_Handle);

            if (wireframeOn_ckbx.Checked)
            {
                sendKey(Key.F4);
            }
            else
            {
                sendKey(Key.F4);
            }
        }

        private void killFx_btn_Click(object sender, EventArgs e)
        {
            SetForegroundWindow(fx_Handle);
            sendTilda();
            sendBackspace();
            sendCmd("fxkill 1");
            sendReturn();
            sendTilda();
        }

        #endregion 
    }
}
