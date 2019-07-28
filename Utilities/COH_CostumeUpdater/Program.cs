using System;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Tcp;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;

namespace COH_CostumeUpdater
{

    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            TcpChannel myChannel = new TcpChannel(0);
            ChannelServices.RegisterChannel(myChannel,true);
            RemotingConfiguration.RegisterWellKnownServiceType
            (typeof(COH_CostumeUpdater.assetEditor.aeCommon.RemotingMatBlock), "COH_CostumeUpdater.assetEditor.aeCommon.RemotingMatBlock", WellKnownObjectMode.Singleton);


            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(true);

            Application.Run(new COH_CostumeUpdaterForm());
        }
    }
}
