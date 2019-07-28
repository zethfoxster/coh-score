using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace FX_TreeParser
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            if (args.Length > 0)
            {
                string arg = (string)args[0];
                FX_TreeParser fxt = new FX_TreeParser(arg);
                fxt.ShowDialog();
                
            }
        }
    }
}
